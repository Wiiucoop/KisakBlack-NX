// nx_kbz.cpp -- on-device loader for KBZ1 "prelinked zones".
//
// KBZ1 files are produced offline by tools/ffconv/convert.exe from the shipped
// 32-bit .ff fastfiles. Each asset has already been transcoded to its native
// LP64 layout and every internal pointer flattened to a (block,offset) pair in
// a relocation table. Loading is therefore a generic three-step relocate --
// NO per-asset parsing on device:
//   1. allocate each block and copy its image,
//   2. walk the reloc table, writing block[tgt]+off into each pointer slot,
//   3. register each asset into the XAssetPool via DB_AddXAsset.
//
// This sidesteps the fundamental blocker that the .ff stream format hard-wires
// 4-byte pointers and x86 struct sizes, which cannot be consumed directly by a
// 64-bit build. See tools/ffconv/ for the offline half and the format spec.
#ifdef KISAK_NX
#include <cstdio>
#include <cstdint>
#include <cstring>
#include <cstdlib>

#include <database/database.h>
#include <database/db_registry.h>
#include <qcommon/common.h>

extern "C" void nx_normalize_path(const char *in, char *out, size_t outSize);
extern "C" const char *nx_get_install_dir(void);

namespace {

struct KbzHeader {
    char     magic[4];   // "KBZ1"
    uint32_t version;    // 1
    uint32_t blockCount; // 8
};

// Read an entire file into a malloc'd buffer (caller frees). Returns size in *n.
uint8_t *slurp(const char *path, long *n)
{
    FILE *f = fopen(path, "rb");
    if (!f) return nullptr;
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (sz <= 0) { fclose(f); return nullptr; }
    uint8_t *buf = (uint8_t *)malloc(sz);
    if (!buf) { fclose(f); return nullptr; }
    long got = (long)fread(buf, 1, sz, f);
    fclose(f);
    if (got != sz) { free(buf); return nullptr; }
    *n = sz;
    return buf;
}

} // namespace

// Parse + relocate + register a KBZ1 image already read into `file`.
// Returns 1 on success, -1 if malformed. `path` is for logging only.
static int loadKbzImage(const char *path, uint8_t *file, long fileSize)
{
    if (fileSize < (long)sizeof(KbzHeader) || memcmp(file, "KBZ1", 4)) {
        Com_PrintError(10, "NX_TryLoadKbz: '%s' bad magic\n", path);
        return -1;
    }
    const uint8_t *p   = file;
    const uint8_t *end = file + fileSize;
    KbzHeader hdr;
    memcpy(&hdr, p, sizeof(hdr));
    p += sizeof(hdr);
    uint32_t nblk = hdr.blockCount;
    if (nblk == 0 || nblk > 16 || p + 4 * nblk + 8 > end) {
        Com_PrintError(10, "NX_TryLoadKbz: '%s' bad header (nblk=%u)\n", path, nblk);
        return -1;
    }

    uint32_t blockSize[16] = {0};
    memcpy(blockSize, p, 4 * nblk);
    p += 4 * nblk;
    uint32_t relocCount, assetCount;
    memcpy(&relocCount, p, 4); p += 4;
    memcpy(&assetCount, p, 4); p += 4;
    Com_Printf(16, "NX_KBZ: '%s' nblk=%u relocs=%u assets=%u\n", path, nblk, relocCount, assetCount);

    // 1. allocate blocks and copy their images. These are PERMANENT (code_pre /
    // en_code_pre / code_post never unload), so a plain persistent malloc is
    // fine and the assets keep pointing into them for the process lifetime.
    uint8_t *block[16] = {0};
    for (uint32_t i = 0; i < nblk; ++i) {
        if (!blockSize[i]) continue;
        if (p + blockSize[i] > end) {
            Com_PrintError(10, "NX_TryLoadKbz: '%s' truncated block %u\n", path, i);
            for (uint32_t k = 0; k < i; ++k) free(block[k]);
            return -1;
        }
        block[i] = (uint8_t *)malloc(blockSize[i]);
        memcpy(block[i], p, blockSize[i]);
        p += blockSize[i];
    }

    // 2. apply relocations: each stored pointer slot becomes a real address.
    for (uint32_t i = 0; i < relocCount; ++i) {
        if (p + 10 > end) break;
        uint8_t  sb = *p++; uint32_t so; memcpy(&so, p, 4); p += 4;
        uint8_t  tb = *p++; uint32_t to; memcpy(&to, p, 4); p += 4;
        if (sb >= nblk || tb >= nblk || !block[sb] || !block[tb]) continue;
        if (so + sizeof(void *) > blockSize[sb] || to > blockSize[tb]) continue;
        void *tgt = block[tb] + to;
        memcpy(block[sb] + so, &tgt, sizeof(void *));
    }
    Com_Printf(16, "NX_KBZ: relocs applied, registering %u assets\n", assetCount);

    // 3. register each asset. The header struct is already native LP64 layout,
    // so we hand DB_AddXAsset a direct pointer into the relocated block.
    for (uint32_t i = 0; i < assetCount; ++i) {
        if (p + 9 > end) break;
        uint32_t type; memcpy(&type, p, 4); p += 4;
        uint8_t  ab = *p++; uint32_t ao; memcpy(&ao, p, 4); p += 4;
        if (ab >= nblk || !block[ab] || ao >= blockSize[ab]) continue;
        XAssetHeader h;
        h.data = block[ab] + ao;
        DB_AddXAsset((XAssetType)type, h);
    }

    Com_Printf(16, "NX_TryLoadKbz: loaded '%s' (%u assets, %u relocs)\n",
               path, assetCount, relocCount);
    return 1;
}

// Attempt to load a prelinked zone for `zoneName` (the .ff is at `ffFilename`).
// Candidate locations, in order:
//   1. "<installDir>/kbz/<zoneName>.kbz"   (single flat folder -- easiest)
//   2. "<ffFilename with .ff -> .kbz>"     (right next to the .ff)
// Returns: 1 loaded, 0 no .kbz found (fall back to .ff), -1 malformed.
extern "C" int NX_TryLoadKbz(const char *zoneName, const char *ffFilename)
{
    char cand[2][512];
    int nCand = 0;

    // candidate 1: flat kbz/ folder next to the game dir
    snprintf(cand[nCand++], 512, "%s/kbz/%s.kbz", nx_get_install_dir(), zoneName);

    // candidate 2: same path as the .ff, extension swapped
    {
        char *dst = cand[nCand];
        nx_normalize_path(ffFilename, dst, 512);
        size_t len = strlen(dst);
        if (len >= 3 && !strcmp(dst + len - 3, ".ff")) { strcpy(dst + len - 3, ".kbz"); nCand++; }
        else if (len + 5 < 512)                        { strcat(dst, ".kbz"); nCand++; }
    }

    for (int i = 0; i < nCand; ++i) {
        long fileSize = 0;
        uint8_t *file = slurp(cand[i], &fileSize);
        if (!file) { Com_Printf(16, "NX_TryLoadKbz: (not found) %s\n", cand[i]); continue; }
        int r = loadKbzImage(cand[i], file, fileSize);
        free(file); // block images are kept; only the raw file buffer is released
        return r;   // found a .kbz here: 1 loaded or -1 malformed
    }
    return 0; // no prelinked zone anywhere -> caller falls back to the .ff path
}

#endif // KISAK_NX

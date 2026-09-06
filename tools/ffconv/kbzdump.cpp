// kbzdump.cpp -- KBZ1 prelinked-zone validator (PC host tool).
//
// This is the reference implementation of the on-device relocator: it reads a
// KBZ1 file, allocates each block, applies the relocation table (turning stored
// (block,offset) pairs into real native pointers), then walks the asset table
// and reads each asset back through those pointers to prove the zone is
// self-consistent and the strings survived the 32->64-bit transcode.
//
// The device version does the same steps, but registers each asset into the
// game's XAssetPool instead of printing it.
#include <cstdio>
#include <cstdint>
#include <cstring>
#include <cstdlib>
#include <vector>

enum { AT_IMAGE = 8, AT_LOCALIZE = 23, AT_RAWFILE = 36, AT_STRINGTABLE = 37 };

// LP64 asset views (must match the transcoder / the game's native structs).
struct RawFile      { const char *name; int len; const char *buffer; };
struct StringCell   { const char *string; int hash; };
struct StringTable  { const char *name; int columnCount, rowCount; StringCell *values; int16_t *cellIndex; };
struct LocalizeEntry{ const char *value; const char *name; };

int main(int argc, char **argv) {
    if (argc < 2) { fprintf(stderr, "usage: kbzdump <zone.kbz> [--all]\n"); return 1; }
    bool all = argc > 2 && !strcmp(argv[2], "--all");

    FILE *f = fopen(argv[1], "rb");
    if (!f) { fprintf(stderr, "cannot open %s\n", argv[1]); return 1; }
    fseek(f, 0, SEEK_END); long fsz = ftell(f); fseek(f, 0, SEEK_SET);
    std::vector<uint8_t> buf(fsz);
    if (fread(buf.data(), 1, fsz, f) != (size_t)fsz) { fprintf(stderr, "short read\n"); return 1; }
    fclose(f);

    const uint8_t *p = buf.data();
    if (memcmp(p, "KBZ1", 4)) { fprintf(stderr, "bad magic\n"); return 1; }
    uint32_t ver, nblk; memcpy(&ver, p + 4, 4); memcpy(&nblk, p + 8, 4);
    const uint32_t *blkSize = (const uint32_t *)(p + 12);
    const uint8_t *cur = p + 12 + 4 * nblk;
    uint32_t relocCount, assetCount;
    memcpy(&relocCount, cur, 4); cur += 4;
    memcpy(&assetCount, cur, 4); cur += 4;

    printf("KBZ1 v%u  blocks=%u  relocs=%u  assets=%u\n", ver, nblk, relocCount, assetCount);

    // 1. allocate blocks and copy their images (this is what the device does)
    std::vector<uint8_t *> block(nblk);
    for (uint32_t i = 0; i < nblk; ++i) {
        block[i] = blkSize[i] ? (uint8_t *)malloc(blkSize[i]) : nullptr;
        if (blkSize[i]) { memcpy(block[i], cur, blkSize[i]); cur += blkSize[i]; }
        if (blkSize[i]) printf("  block[%u] = %u bytes\n", i, blkSize[i]);
    }

    // 2. apply relocations: *(void**)(block[sb]+so) = block[tb]+to
    int relocBad = 0;
    for (uint32_t i = 0; i < relocCount; ++i) {
        uint8_t sb = *cur++; uint32_t so; memcpy(&so, cur, 4); cur += 4;
        uint8_t tb = *cur++; uint32_t to; memcpy(&to, cur, 4); cur += 4;
        if (sb >= nblk || tb >= nblk || so + 8 > blkSize[sb] || to > blkSize[tb]) {
            if (relocBad < 8) printf("  BAD reloc: [b%u+%u] -> [b%u+%u]\n", sb, so, tb, to);
            ++relocBad; continue;
        }
        void *tgt = block[tb] + to;
        memcpy(block[sb] + so, &tgt, sizeof(void *));
    }

    // 3. walk assets through the now-native pointers
    int shown = 0, nameBad = 0, nullName = 0;
    auto okStr = [&](const char *s) -> bool {
        if (!s) return true;                       // null is legal
        for (uint32_t b = 0; b < nblk; ++b)
            if (block[b] && (const uint8_t *)s >= block[b] && (const uint8_t *)s < block[b] + blkSize[b]) return true;
        return false;                              // dangling pointer
    };
    int histType[64] = {0};
    for (uint32_t i = 0; i < assetCount; ++i) {
        uint32_t type; memcpy(&type, cur, 4); cur += 4;
        uint8_t ab = *cur++; uint32_t ao; memcpy(&ao, cur, 4); cur += 4;
        if (type < 64) histType[type]++;
        void *hdr = block[ab] + ao;
        const char *nm = nullptr; char extra[128] = {0};
        if (type == AT_RAWFILE) {
            RawFile *rf = (RawFile *)hdr; nm = rf->name;
            snprintf(extra, sizeof(extra), "len=%d buf=%s", rf->len, rf->buffer ? "yes" : "null");
        } else if (type == AT_STRINGTABLE) {
            StringTable *st = (StringTable *)hdr; nm = st->name;
            const char *c00 = (st->values && st->rowCount && st->columnCount) ? st->values[0].string : "";
            snprintf(extra, sizeof(extra), "%dx%d cell[0]='%s'", st->rowCount, st->columnCount, c00 ? c00 : "(null)");
        } else if (type == AT_IMAGE) {
            // GfxImage is read at explicit byte offsets rather than through a
            // struct, to stay honest about the LP64 layout the game actually
            // has: name@56, hash@64, sizeof 72 -- checked with offsetof
            // against gfx_d3d/r_material.h under the devkitA64 compiler.
            const uint8_t *g = (const uint8_t *)hdr;
            memcpy(&nm, g + 56, sizeof nm);
            uint32_t hash; memcpy(&hash, g + 64, 4);
            uint16_t w, h; memcpy(&w, g + 24, 2); memcpy(&h, g + 26, 2);
            snprintf(extra, sizeof(extra), "%ux%u hash=%08x", w, h, hash);
        } else if (type == AT_LOCALIZE) {
            LocalizeEntry *le = (LocalizeEntry *)hdr; nm = le->name;
            snprintf(extra, sizeof(extra), "value='%.60s'", le->value ? le->value : "(null)");
        }
        // Every asset the engine can look up must yield a name, because
        // DB_GetXAssetHeaderName (db_assetnames.cpp:466) asserts on a null one
        // while walking a hash bucket. The getter table (db_assetnames.cpp:23)
        // is uniform: DB_ImageGetName reads GfxImage::name at 56,
        // DB_LocalizeEntryGetName reads LocalizeEntry::name at 8,
        // DB_GetEmblemSetName returns a literal, and every other type goes
        // through DB_DDLGetName -- the pointer at offset 0.
        const char *engineName = nullptr;
        bool hasGetter = true;
        {
            const uint8_t *g = (const uint8_t *)hdr;
            if (type == AT_IMAGE)            memcpy(&engineName, g + 56, sizeof engineName);
            else if (type == AT_LOCALIZE)    memcpy(&engineName, g + 8,  sizeof engineName);
            else if (type == 42)             { engineName = "emblemset"; hasGetter = false; } // literal, never in a block
            else if (type == 19 || type == 25 || type == 26 ||
                     (type >= 30 && type <= 35))  hasGetter = false;   // null handler
            else                             memcpy(&engineName, g + 0,  sizeof engineName);
        }
        if (hasGetter && !engineName) {
            ++nullName;
            if (nullName <= 12) printf("  NULL name: asset %u type %u\n", i, type);
        } else if (hasGetter && !okStr(engineName)) {
            ++nameBad;
            if (nameBad <= 12) printf("  DANGLING name ptr: asset %u type %u\n", i, type);
        }
        if (!nm) nm = (type == 42) ? nullptr : engineName;
        if (!okStr(nm)) { ++nameBad; if (nameBad <= 8) printf("  BAD name ptr asset %u type %u\n", i, type); }
        if (all || shown < 12) { printf("  [%3u] t%-2u %-28s %s\n", i, type, nm ? nm : "(null)", extra); ++shown; }
    }

    printf("type histogram:");
    for (int t = 0; t < 64; ++t) if (histType[t]) printf(" t%d=%d", t, histType[t]);
    printf("\nvalidation: relocBad=%d nameBad=%d nullName=%d  => %s\n",
           relocBad, nameBad, nullName,
           (relocBad == 0 && nameBad == 0 && nullName == 0) ? "OK" : "FAIL");
    return (relocBad == 0 && nameBad == 0 && nullName == 0) ? 0 : 1;
}

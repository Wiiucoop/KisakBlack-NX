// ffconv -- offline Black Ops (T5) fastfile inspector / converter (PC host tool).
//
// Phase 1 (this file): container reader. Verifies the .ff header, inflates the
// zlib payload, parses the XFile block-size header, and dumps the start of the
// zone stream so the XAssetList/TOC parser can be nailed against ground truth.
//
// The full 32->64-bit asset repack is layered on top of this once the stream
// layout is confirmed against a real zone. Build: see build.sh (needs zlib).
//
// .ff container (confirmed via db_file_load.cpp + a working zlib round-trip):
//   [0]  char     magic[8]     "IWffu100" (unsigned) or "IWff0100" (signed)
//   [8]  uint32   version      473 (0x1D9) for T5
//   [12] zlib stream  -> inflates to the zone:
//        XFile { uint32 size; uint32 externalSize; uint32 blockSize[7]; }  (36 bytes)
//        then the asset stream (XAssetList, script strings, XAsset[], asset data)
#include <cstdio>
#include <cstdint>
#include <cstring>
#include <cstdlib>
#include <vector>
#include <string>
#include <zlib.h>

static const char *kAssetNames[43] = {
    "xmodelpieces", "physpreset", "physconstraints", "destructibledef", "xanim",
    "xmodel", "material", "techset", "image", "sound", "sound_patch", "col_map_sp",
    "col_map_mp", "com_map", "game_map_sp", "game_map_mp", "map_ents", "gfx_map",
    "lightdef", "ui_map", "font", "menufile", "menu", "localize", "weapon",
    "weapondef", "weaponvariant", "snddriverglobals", "fx", "impactfx", "aitype",
    "mptype", "mpbody", "mphead", "character", "xmodelalias", "rawfile",
    "stringtable", "packindex", "xGlobals", "ddl", "glasses", "emblemset",
};

struct XFileHeader {
    uint32_t size;
    uint32_t externalSize;
    uint32_t blockSize[7];
};

static std::vector<uint8_t> readFile(const char *path)
{
    FILE *f = fopen(path, "rb");
    if (!f) { fprintf(stderr, "cannot open %s\n", path); exit(1); }
    fseek(f, 0, SEEK_END);
    long n = ftell(f);
    fseek(f, 0, SEEK_SET);
    std::vector<uint8_t> buf(n);
    if (fread(buf.data(), 1, n, f) != (size_t)n) { fprintf(stderr, "short read\n"); exit(1); }
    fclose(f);
    return buf;
}

// Inflate the whole zlib payload (zone can be large; grow as needed).
static std::vector<uint8_t> inflateAll(const uint8_t *src, size_t srcLen)
{
    std::vector<uint8_t> out;
    out.resize(1 << 20);
    z_stream zs;
    memset(&zs, 0, sizeof(zs));
    if (inflateInit(&zs) != Z_OK) { fprintf(stderr, "inflateInit failed\n"); exit(1); }
    zs.next_in = (Bytef *)src;
    zs.avail_in = (uInt)srcLen;
    for (;;) {
        if (zs.total_out >= out.size())
            out.resize(out.size() * 2);
        zs.next_out = out.data() + zs.total_out;
        zs.avail_out = (uInt)(out.size() - zs.total_out);
        int r = inflate(&zs, Z_NO_FLUSH);
        if (r == Z_STREAM_END) break;
        if (r != Z_OK) { fprintf(stderr, "inflate error %d\n", r); break; }
        if (zs.avail_in == 0 && zs.avail_out != 0) break;
    }
    out.resize(zs.total_out);
    inflateEnd(&zs);
    return out;
}

static void hexdump(const uint8_t *p, size_t len, size_t base)
{
    for (size_t i = 0; i < len; i += 16) {
        printf("  %06zx  ", base + i);
        for (size_t j = 0; j < 16; ++j) {
            if (i + j < len) printf("%02x ", p[i + j]);
            else printf("   ");
        }
        printf(" |");
        for (size_t j = 0; j < 16 && i + j < len; ++j) {
            uint8_t c = p[i + j];
            putchar(c >= 32 && c < 127 ? c : '.');
        }
        printf("|\n");
    }
}

int main(int argc, char **argv)
{
    if (argc < 2) {
        fprintf(stderr, "usage: ffconv <zone.ff> [--dump N]\n");
        return 1;
    }
    size_t dumpBytes = 512;
    for (int i = 2; i < argc; ++i)
        if (!strcmp(argv[i], "--dump") && i + 1 < argc) dumpBytes = strtoul(argv[++i], 0, 0);

    std::vector<uint8_t> file = readFile(argv[1]);
    printf("file: %s (%zu bytes)\n", argv[1], file.size());
    if (file.size() < 12) { fprintf(stderr, "too small\n"); return 1; }

    char magic[9] = {0};
    memcpy(magic, file.data(), 8);
    uint32_t version;
    memcpy(&version, file.data() + 8, 4);
    printf("magic: '%s'  version: %u (0x%x)\n", magic, version, version);
    bool signedZone = (memcmp(magic, "IWff0100", 8) == 0);
    bool unsignedZone = (memcmp(magic, "IWffu100", 8) == 0);
    if (!signedZone && !unsignedZone) {
        fprintf(stderr, "not an IWff zone (magic mismatch)\n");
        return 1;
    }

    std::vector<uint8_t> zone = inflateAll(file.data() + 12, file.size() - 12);
    printf("inflated zone: %zu bytes\n", zone.size());
    if (zone.size() < sizeof(XFileHeader)) { fprintf(stderr, "zone too small\n"); return 1; }

    XFileHeader hdr;
    memcpy(&hdr, zone.data(), sizeof(hdr));
    printf("\nXFile header:\n");
    printf("  size         = %u\n", hdr.size);
    printf("  externalSize = %u\n", hdr.externalSize);
    for (int i = 0; i < 7; ++i)
        printf("  blockSize[%d] = %u\n", i, hdr.blockSize[i]);

    // The asset stream begins right after the 36-byte XFile header. The first
    // object is the XAssetList (16 bytes on disk):
    //   ScriptStringList { int count; ptr strings; }  (8)
    //   int assetCount;                               (4)
    //   ptr assets;                                   (4)
    const uint8_t *s = zone.data() + sizeof(hdr);
    size_t rem = zone.size() - sizeof(hdr);
    if (rem >= 16) {
        uint32_t stringCount, stringsTag, assetCount, assetsTag;
        memcpy(&stringCount, s + 0, 4);
        memcpy(&stringsTag,  s + 4, 4);
        memcpy(&assetCount,  s + 8, 4);
        memcpy(&assetsTag,   s + 12, 4);
        printf("\nXAssetList:\n");
        printf("  scriptString count = %u (tag=0x%08x)\n", stringCount, stringsTag);
        printf("  asset count        = %u (tag=0x%08x)\n", assetCount, assetsTag);
    }

    // --- walk to the XAsset[] array and histogram the types ---------------
    // The stream is gapless (DB_AllocStreamPos aligns the memory cursor, not the
    // stream). Order: XAssetList(16) -> [script strings] -> XAsset[]  (each 8:
    // type u32 + header tag u32) -> per-asset data.
    {
        uint32_t stringCount, stringsTag, assetCount, assetsTag;
        memcpy(&stringCount, s + 0, 4);
        memcpy(&stringsTag,  s + 4, 4);
        memcpy(&assetCount,  s + 8, 4);
        memcpy(&assetsTag,   s + 12, 4);
        const uint8_t *p = s + 16;
        const uint8_t *end = zone.data() + zone.size();

        // Script-string list: count 4-byte tags, then an inline NUL-terminated
        // string for each 0xFFFFFFFF tag.
        if (stringsTag != 0 && stringCount) {
            std::vector<uint32_t> tags(stringCount);
            memcpy(tags.data(), p, 4 * stringCount);
            p += 4 * (size_t)stringCount;
            for (uint32_t i = 0; i < stringCount; ++i) {
                if (tags[i] == 0xFFFFFFFFu) {
                    const uint8_t *z = p;
                    while (z < end && *z) ++z;
                    p = z + 1; // past the NUL
                }
            }
        }

        printf("\nasset TOC (%u assets):\n", assetCount);
        int hist[64] = {0};
        bool sane = true;
        const uint8_t *arr = p;
        for (uint32_t i = 0; i < assetCount; ++i) {
            uint32_t type, tag;
            memcpy(&type, arr + 8 * i, 4);
            memcpy(&tag,  arr + 8 * i + 4, 4);
            if (type < 43) hist[type]++;
            else { sane = false; printf("  [%u] OUT-OF-RANGE type=%u tag=0x%08x\n", i, type, tag); if (i > 3) break; }
        }
        if (sane) {
            for (int t = 0; t < 43; ++t)
                if (hist[t]) printf("  %-16s (type %2d): %d\n", kAssetNames[t], t, hist[t]);
        } else {
            printf("  (type parse looks misaligned -- string-list walk needs fixing for this zone)\n");
        }
    }
    return 0;
}

// convert.cpp -- offline 32->64-bit fastfile converter (Stage 1: rawfile,
// stringtable, localize). Reads a shipped .ff, transcodes each asset from its
// x86 (4-byte-pointer) layout into a native-LP64 "prelinked zone" (KBZ1) that
// the on-device generic relocator consumes.
//
// Each transcode fn mirrors the game's Load_<T> (src/database/db_load.cpp) but
// writes LP64 fields + records relocations instead of fixing up in place.
//
// The hard part is de-duplicated pointers: a shipped string can be emitted once
// and referenced again by a 32-bit "stream offset" that encodes (block, byte).
// To resolve those we emulate the game's APPEND-ONLY block-4 memory cursor in
// lockstep (every AllocLoad_* / Load_Stream that targets block 4), recording
// x86-offset -> output-Loc for each datum. A second pass then rewrites each
// deferred offset ref. Correctness is self-checking: the final emulated block-4
// cursor must equal XFileHeader.blockSize[4] exactly.
#include "prelink.h"
#include <zlib.h>
#include <unordered_map>

// asset type ids (src/database/db_assetnames.cpp g_assetNames)
enum { AT_LOCALIZE = 23, AT_RAWFILE = 36, AT_STRINGTABLE = 37 };

static std::vector<uint8_t> readFile(const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) { fprintf(stderr, "cannot open %s\n", path); exit(1); }
    fseek(f, 0, SEEK_END); long n = ftell(f); fseek(f, 0, SEEK_SET);
    std::vector<uint8_t> b(n);
    if (fread(b.data(), 1, n, f) != (size_t)n) { fprintf(stderr, "short read\n"); exit(1); }
    fclose(f); return b;
}
static std::vector<uint8_t> inflateAll(const uint8_t *src, size_t srcLen) {
    std::vector<uint8_t> out(1 << 20);
    z_stream zs; memset(&zs, 0, sizeof(zs));
    inflateInit(&zs);
    zs.next_in = (Bytef *)src; zs.avail_in = (uInt)srcLen;
    for (;;) {
        if (zs.total_out >= out.size()) out.resize(out.size() * 2);
        zs.next_out = out.data() + zs.total_out;
        zs.avail_out = (uInt)(out.size() - zs.total_out);
        int r = inflate(&zs, Z_NO_FLUSH);
        if (r == Z_STREAM_END) break;
        if (r != Z_OK) break;
        if (zs.avail_in == 0 && zs.avail_out != 0) break;
    }
    out.resize(zs.total_out); inflateEnd(&zs); return out;
}

// ---- block-4 x86 memory emulation (for de-dup pointer resolution) -----------
static const int OUT = 4;                // output block we place everything in
static uint32_t g_x86b4 = 0;             // emulated game block-4 byte cursor
static std::unordered_map<uint32_t, Prelink::Loc> g_b4map; // x86 off -> out Loc
struct Deferred { Prelink::Loc obj; uint32_t field; uint32_t x86off; };
static std::vector<Deferred> g_deferred; // block-4 offset refs, resolved in pass 2
static int g_cntAlias = 0, g_cntOffOther = 0;

// DB_AllocStreamPos(param): align the cursor up. param 0->none, 1->2, 3->4.
static inline uint32_t alignUp(uint32_t v, uint32_t param) { return (v + param) & ~param; }

// Reserve `size` bytes in the emulated block-4 image at alignment `param`,
// mapping that x86 offset to output location `out` (if valid). Returns x86 off.
static uint32_t b4Reserve(uint32_t param, uint32_t size, Prelink::Loc out) {
    g_x86b4 = alignUp(g_x86b4, param);
    uint32_t off = g_x86b4;
    if (out.valid()) g_b4map[off] = out;
    g_x86b4 += size;
    return off;
}

// Emit an inline string to the output block and mirror its block-4 x86 alloc
// (AllocLoad_raw_byte = DB_AllocStreamPos(0), then strlen+1 bytes).
static Prelink::Loc emitInlineStr(Prelink &z, const std::string &s) {
    Prelink::Loc L = z.putBytes(OUT, (const uint8_t *)s.c_str(), s.size() + 1, 1);
    b4Reserve(0, (uint32_t)s.size() + 1, L);
    return L;
}

// Write an XString slot from a tag already read from the stream:
//   0          -> null
//   -1         -> inline NUL-terminated string follows now
//   -2 (alias) -> pointer-insert back-ref (unused in code_* zones)
//   other      -> block/offset back-ref into already-emitted data (resolved p2)
static void putXStringFromTag(Reader &r, Prelink &z, Prelink::Loc obj, uint32_t field, uint32_t tag) {
    if (tag == TAG_NULL) { z.putPtr(obj, field, Prelink::none()); return; }
    if (tag == TAG_INLINE) { std::string s = r.cstr(); z.putPtr(obj, field, emitInlineStr(z, s)); return; }
    if (tag == TAG_ALIAS) { ++g_cntAlias; z.putPtr(obj, field, Prelink::none()); return; }
    uint32_t blk = (tag - 1) >> 29, off = (tag - 1) & 0x1FFFFFFF;
    if (blk == 4) g_deferred.push_back({obj, field, off}); // slot stays zero until pass 2
    else { ++g_cntOffOther; z.putPtr(obj, field, Prelink::none()); }
}

// ---- per-asset transcoders --------------------------------------------------
// Load_RawFile: x86 {char* name; int len; char* buffer} (12) -> LP64 (24).
static void tRawFile(Reader &r, Prelink &z, Prelink::Loc obj) {
    uint32_t nameTag = r.u32();
    int32_t  len     = r.i32();
    uint32_t bufTag  = r.u32();
    z.putI32(obj, 8, len);                       // name@0, len@8, buffer@16
    putXStringFromTag(r, z, obj, 0, nameTag);
    if (bufTag == TAG_NULL) { z.putPtr(obj, 16, Prelink::none()); }
    else {                                       // AllocLoad_raw_byte, len+1 bytes
        uint32_t n = (uint32_t)len + 1;
        Prelink::Loc buf = z.alloc(OUT, n, 1);
        r.bytes(z.at(buf), n);
        b4Reserve(0, n, buf);
        z.putPtr(obj, 16, buf);
    }
}

// Load_StringTable: x86 {char* name; int col; int row; Cell* values; short* cellIndex} (20).
// Array loaders read ALL fixed structs as one block first, then per-element data.
static void tStringTable(Reader &r, Prelink &z, Prelink::Loc obj) {
    uint32_t nameTag = r.u32();
    int32_t  col     = r.i32();
    int32_t  row     = r.i32();
    uint32_t valsTag = r.u32();
    uint32_t idxTag  = r.u32();
    z.putI32(obj, 8, col);                       // name@0,col@8,row@12,values@16,cellIndex@24
    z.putI32(obj, 12, row);
    putXStringFromTag(r, z, obj, 0, nameTag);
    int cells = row * col;
    if (valsTag == TAG_NULL) z.putPtr(obj, 16, Prelink::none());
    else {                                       // StringTableCell x86 (8) -> LP64 (16)
        Prelink::Loc arr = z.alloc(OUT, (size_t)cells * 16, 8);
        z.putPtr(obj, 16, arr);
        b4Reserve(3, (uint32_t)cells * 8, arr);  // 8*cells struct block (align 4)
        std::vector<uint32_t> strTag(cells);
        for (int i = 0; i < cells; ++i) {
            strTag[i]    = r.u32();
            int32_t hash = r.i32();
            z.putI32(Prelink::Loc{OUT, arr.off + (uint32_t)i * 16}, 8, hash);
        }
        for (int i = 0; i < cells; ++i)          // then inline cell strings, in order
            putXStringFromTag(r, z, Prelink::Loc{OUT, arr.off + (uint32_t)i * 16}, 0, strTag[i]);
    }
    if (idxTag == TAG_NULL) z.putPtr(obj, 24, Prelink::none());
    else {                                       // AllocLoad_XBlendInfo (align 2), 2*cells
        Prelink::Loc arr = z.alloc(OUT, (size_t)cells * 2, 2);
        z.putPtr(obj, 24, arr);
        b4Reserve(1, (uint32_t)cells * 2, arr);
        r.bytes(z.at(arr), (size_t)cells * 2);
    }
}

// Load_LocalizeEntry: x86 {char* value; char* name} (8) -> LP64 (16).
static void tLocalizeEntry(Reader &r, Prelink &z, Prelink::Loc obj) {
    uint32_t valueTag = r.u32();
    uint32_t nameTag  = r.u32();
    putXStringFromTag(r, z, obj, 0, valueTag);
    putXStringFromTag(r, z, obj, 8, nameTag);
}

int main(int argc, char **argv) {
    if (argc < 3) { fprintf(stderr, "usage: convert <in.ff> <out.kbz>\n"); return 1; }
    std::vector<uint8_t> file = readFile(argv[1]);
    if (file.size() < 12 || (memcmp(file.data(), "IWffu100", 8) && memcmp(file.data(), "IWff0100", 8))) {
        fprintf(stderr, "not an IWff zone\n"); return 1;
    }
    std::vector<uint8_t> zone = inflateAll(file.data() + 12, file.size() - 12);
    uint32_t blockSize4; memcpy(&blockSize4, zone.data() + 8 + 4 * 4, 4); // XFile.blockSize[4]
    Reader r(zone.data() + 36, zone.size() - 36);                          // skip XFile header

    uint32_t stringCount = r.u32(), stringsTag = r.u32();
    uint32_t assetCount = r.u32(), assetsTag = r.u32();
    (void)assetsTag;

    bool dbg = getenv("FFDBG") != nullptr;
    Prelink z;

    // Script string list -- Load_ScriptStringList (src/database/db_load.cpp:613).
    // These come BEFORE the asset type array in the stream. The 8-byte
    // {count, strings} header was read above; when the pointer tag is non-null
    // a count*4 pointer array is reserved in block 4, then Load_TempStringArray
    // reads every tag followed by the inline bodies.
    //
    // We parse them so the stream stays in sync and their block-4 offsets land
    // in the dedup map -- assets reference these strings by offset. They are
    // NOT yet carried into the KBZ nor registered on device: prelink.h has a
    // scriptStrings field but nothing writes or reads it yet.
    if (stringCount != 0 && stringsTag != 0) {
        Prelink::Loc arr = z.alloc(OUT, (size_t)stringCount * 8, 8);
        b4Reserve(3, stringCount * 4, arr);   // DB_AllocStreamPos(3) -> align 4

        std::vector<uint32_t> strTags(stringCount);
        for (uint32_t i = 0; i < stringCount; ++i) strTags[i] = r.u32();

        for (uint32_t i = 0; i < stringCount; ++i)
            putXStringFromTag(r, z, arr, i * 8, strTags[i]);
    }

    std::vector<uint32_t> types(assetCount), hdrTag(assetCount);
    for (uint32_t i = 0; i < assetCount; ++i) { types[i] = r.u32(); hdrTag[i] = r.u32(); }

    // The XAsset array follows the script strings in block 4, so reserve it
    // rather than resetting the cursor.
    b4Reserve(0, 8 * assetCount, Prelink::none());
    
    for (uint32_t i = 0; i < assetCount; ++i) {
        const uint8_t *pre = r.p;
        switch (types[i]) {
        case AT_RAWFILE:     { Prelink::Loc o = z.alloc(OUT, 24, 8); tRawFile(r, z, o);      z.addAsset(AT_RAWFILE, o); break; }
        case AT_STRINGTABLE: { Prelink::Loc o = z.alloc(OUT, 32, 8); tStringTable(r, z, o);  z.addAsset(AT_STRINGTABLE, o); break; }
        case AT_LOCALIZE:    { Prelink::Loc o = z.alloc(OUT, 16, 8); tLocalizeEntry(r, z, o); z.addAsset(AT_LOCALIZE, o); break; }
        default:
            fprintf(stderr, "unsupported asset type %u at index %u (Stage 1 = rawfile/stringtable/localize)\n", types[i], i);
            return 3;
        }
        if (dbg)
            printf("[%3u] type=%2u off=%6zu consumed=%3zd hdrTag=%08x\n",
                   i, types[i], (size_t)(pre - zone.data()), (ptrdiff_t)(r.p - pre), hdrTag[i]);
        if (r.overran) { fprintf(stderr, "stream overran at asset %u -- transcoder desync\n", i); return 4; }
    }

    // pass 2: resolve deferred block-4 offset refs against the emulated map
    int resolved = 0, missing = 0;
    for (const Deferred &d : g_deferred) {
        auto it = g_b4map.find(d.x86off);
        if (it != g_b4map.end()) { z.putPtr(d.obj, d.field, it->second); ++resolved; }
        else { ++missing; if (dbg) printf("  UNRESOLVED block4 off=%u\n", d.x86off); }
    }

    z.write(argv[2]);
    bool b4ok = (g_x86b4 == blockSize4);
    printf("converted %s -> %s\n", argv[1], argv[2]);
    printf("  assets=%u  relocs=%zu  outBlock=%zu bytes  streamPos=%zu/%zu\n",
           assetCount, z.relocs.size(), z.block[OUT].size(), (size_t)(r.p - zone.data()), zone.size());
    printf("  block4 x86 cursor=%u  header blockSize[4]=%u  %s\n",
           g_x86b4, blockSize4, b4ok ? "MATCH" : "*** MISMATCH ***");
    printf("  dedup refs: resolved=%d missing=%d  alias=%d  offOtherBlock=%d\n",
           resolved, missing, g_cntAlias, g_cntOffOther);
    return (b4ok && missing == 0) ? 0 : 5;
}

// KBZ1 writer.
void Prelink::write(const char *path) {
    FILE *f = fopen(path, "wb");
    if (!f) { fprintf(stderr, "cannot write %s\n", path); exit(1); }
    uint32_t blkSize[NBLOCK];
    for (int i = 0; i < NBLOCK; ++i) blkSize[i] = (uint32_t)block[i].size();
    char magic[4] = {'K','B','Z','1'};
    uint32_t ver = 1, nblk = NBLOCK;
    uint32_t rc = (uint32_t)relocs.size(), ac = (uint32_t)assets.size();
    fwrite(magic, 1, 4, f); fwrite(&ver, 4, 1, f); fwrite(&nblk, 4, 1, f);
    fwrite(blkSize, 4, NBLOCK, f);
    fwrite(&rc, 4, 1, f); fwrite(&ac, 4, 1, f);
    for (int i = 0; i < NBLOCK; ++i) if (!block[i].empty()) fwrite(block[i].data(), 1, block[i].size(), f);
    for (auto &rl : relocs) { fwrite(&rl.slotBlk,1,1,f); fwrite(&rl.slotOff,4,1,f); fwrite(&rl.tgtBlk,1,1,f); fwrite(&rl.tgtOff,4,1,f); }
    for (auto &a : assets)  { fwrite(&a.type,4,1,f); fwrite(&a.blk,1,1,f); fwrite(&a.off,4,1,f); }
    fclose(f);
}

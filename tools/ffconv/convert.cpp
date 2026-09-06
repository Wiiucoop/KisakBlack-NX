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
#include <map>

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
struct Deferred { Prelink::Loc obj; uint32_t field; uint32_t x86off; int asset; int line; };
static std::vector<Deferred> g_deferred; // block-4 offset refs, resolved in pass 2
static int g_cntAlias = 0, g_cntOffOther = 0;
static int g_curAsset = -1;              // asset being transcoded, for diagnostics

// DB_AllocStreamPos(param): align the cursor up. param 0->none, 1->2, 3->4.
static inline uint32_t alignUp(uint32_t v, uint32_t param) { return (v + param) & ~param; }

// Reserve `size` bytes in the emulated block-4 image at alignment `param`,
// mapping that x86 offset to output location `out` (if valid). Returns x86 off.
static bool g_traceB4 = false;           // FFB4=1: log every block-4 reservation
struct B4Log { uint32_t off, size; int line; };
static std::vector<B4Log> g_b4log;       // every reservation, in order
static uint32_t b4Reserve(uint32_t param, uint32_t size, Prelink::Loc out,
                          int line = __builtin_LINE()) {
    g_x86b4 = alignUp(g_x86b4, param);
    uint32_t off = g_x86b4;
    if (out.valid()) g_b4map[off] = out;
    g_b4log.push_back({off, size, line});
    if (g_traceB4) fprintf(stderr, "    b4 asset=%d off=%u +%u (line %d)\n",
                           g_curAsset, off, size, line);
    g_x86b4 += size;
    return off;
}

// ...but an asset's OWN struct never lands in block 4. Every Load_<T>Ptr
// wraps its allocation in DB_PushStreamPos(0) / DB_PopStreamPos, and popping
// out of block 0 rewinds that block's cursor (db_stream.cpp:69): block 0 is
// scratch, reused by the next asset. Only what Load_<T> goes on to reference,
// under its own DB_PushStreamPos(4), advances the block-4 cursor.
// Load_GfxTextureLoad (db_load.cpp:1904) does the same for the whole
// GfxImageLoadDef, pixel payload included -- the loader hands it to the GPU
// and drops it. Counted separately so the accounting stays visible.
static uint32_t g_x86temp = 0;
static void tempReserve(uint32_t size) { g_x86temp += size; }

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
static void putXStringFromTag(Reader &r, Prelink &z, Prelink::Loc obj, uint32_t field, uint32_t tag,
                              int line = __builtin_LINE()) {
    if (tag == TAG_NULL) { z.putPtr(obj, field, Prelink::none()); return; }
    if (tag == TAG_INLINE) { std::string s = r.cstr(); z.putPtr(obj, field, emitInlineStr(z, s)); return; }
    if (tag == TAG_ALIAS) { ++g_cntAlias; z.putPtr(obj, field, Prelink::none()); return; }
    uint32_t blk = (tag - 1) >> 29, off = (tag - 1) & 0x1FFFFFFF;
    if (blk == 4) g_deferred.push_back({obj, field, off, g_curAsset, line}); // zero until pass 2
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

// ---- techset tree -----------------------------------------------------------
// MaterialTechniqueSet -> MaterialTechnique[] -> MaterialPass[] -> leaves.
// Sizes in the comments are x86 (the sizeof= annotations in r_material.h) and
// the LP64 equivalents; every struct here widens because of embedded pointers.

enum { AT_TECHSET = 7 };

enum {
    SZ_SHADER_ARG  = 16,    // x86 8
    SZ_VERTEX_DECL = 184,   // x86 108
    SZ_SHADER      = 32,    // x86 16, vertex and pixel share the layout
    SZ_PASS        = 40,    // x86 20
    SZ_TECH_HDR    = 16,    // x86 8, passArray follows
    SZ_TECHSET     = 1056   // x86 528
};

static void putU16(Prelink &z, Prelink::Loc l, uint32_t field, uint16_t v) {
    memcpy(z.at(l) + field, &v, 2);
}

// Resolve a non-string pointer slot from its stream tag. This is the
// DB_ConvertOffsetToPointer form (db_stream_load.cpp:82): the offset names the
// datum itself.
static void putStructOffsetRef(Prelink &z, Prelink::Loc obj, uint32_t field, uint32_t tag,
                               int line = __builtin_LINE()) {
    uint32_t blk = (tag - 1) >> 29, off = (tag - 1) & 0x1FFFFFFF;
    if (blk == 4) g_deferred.push_back({obj, field, off, g_curAsset, line});
    else { ++g_cntOffOther; z.putPtr(obj, field, Prelink::none()); }
}

// ---- asset handles ----------------------------------------------------------
// Asset pointers do NOT resolve like the above. Load_<T>Ptr's fallback is
// DB_ConvertOffsetToAlias (db_stream_load.cpp:63), which *dereferences*:
// the offset names a 4-byte slot in block 4 whose contents are the asset
// pointer. Two things fill such slots:
//   - XAsset[i].header, at assetArray + i*8 + 4, once asset i is loaded;
//   - a slot DB_InsertPointer (db_stream.cpp:113) reserves when the asset was
//     written with tag -2, which is why -2 costs 4 block-4 bytes that -1 does
//     not.
// A slot is filled before anything can reference it -- the stream is written
// in load order -- so alias references resolve immediately, which also makes
// a chain of aliases resolve for free.
static std::unordered_map<uint32_t, Prelink::Loc> g_aliasMap;   // slot off -> asset
static int g_cntAliasOther = 0, g_aliasOk = 0, g_aliasBad = 0;

// DB_InsertPointer: reserve the 4-byte alias slot, in block 4, align 4.
static uint32_t insertPointerSlot() {
    return b4Reserve(3, 4, Prelink::none());
}

// Record that the 4-byte block-4 slot at x86 offset `slot` holds a pointer to
// `target`. Any pointer field of a block-4 struct is a candidate source for a
// later alias reference, so callers pass the slot's x86 offset where they know
// it; 0 means "this pointer does not live in block 4" (an asset's own header,
// for instance, which sits in the rewound temp block).
static void noteAliasSlot(uint32_t slot, Prelink::Loc target) {
    if (slot && target.valid()) g_aliasMap[slot] = target;
}

static Prelink::Loc putAssetHandleRef(Prelink &z, Prelink::Loc obj, uint32_t field,
                                      uint32_t tag, int line = __builtin_LINE()) {
    uint32_t blk = (tag - 1) >> 29, off = (tag - 1) & 0x1FFFFFFF;
    if (blk != 4) {
        ++g_cntAliasOther; z.putPtr(obj, field, Prelink::none());
        return Prelink::none();
    }
    auto it = g_aliasMap.find(off);
    if (it == g_aliasMap.end()) {
        ++g_aliasBad;
        if (getenv("FFDBG") && g_aliasBad <= 20) {
            uint32_t st = 0, sz = 0; int ln = 0;
            for (const auto &e : g_b4log)
                if (e.off <= off && e.off >= st) { st = e.off; sz = e.size; ln = e.line; }
            fprintf(stderr, "  UNRESOLVED alias slot=%u (asset %d, ref from line %d)"
                    " inside [%u,+%u) at +%u  reserved at line %d\n",
                    off, g_curAsset, line, st, sz, off - st, ln);
        }
        z.putPtr(obj, field, Prelink::none());
        return Prelink::none();
    }
    ++g_aliasOk;
    z.putPtr(obj, field, it->second);
    return it->second;
}

// MaterialVertexDeclaration: 108 flat -> 184. The trailing decl[18] are D3D
// objects built at runtime, so those bytes are consumed and dropped.
static void tMaterialVertexDeclaration(Reader &r, Prelink &z, Prelink::Loc obj) {
    r.bytes(z.at(obj), 4);          // streamCount, hasOptionalSource, isLoaded, pad
    r.bytes(z.at(obj) + 8, 32);     // routing.data[16], two bytes each
    r.skip(72);                     // routing.decl[18]
}

// MaterialVertexShader / MaterialPixelShader: {char* name; prog} 16 -> 32.
// prog.vs (or .ps) is a runtime D3D object; only the load def carries over.
// Load order: the whole 16-byte block, then the name string, then the bytecode.
static void tMaterialShader(Reader &r, Prelink &z, Prelink::Loc obj) {
    uint32_t nameTag  = r.u32();
    r.u32();                        // vs/ps -- runtime object, dropped
    uint32_t progTag  = r.u32();
    uint16_t progSize = r.u16();
    r.u16();                        // padding

    putXStringFromTag(r, z, obj, 0, nameTag);
    putU16(z, obj, 24, progSize);   // prog.loadDef.programSize

    if (progTag != TAG_NULL) {
        uint32_t n = (uint32_t)progSize * 4;   // Load_DWORDArray counts DWORDs
        Prelink::Loc buf = z.alloc(OUT, n, 4);
        r.bytes(z.at(buf), n);
        b4Reserve(3, n, buf);
        z.putPtr(obj, 16, buf);     // prog.loadDef.program
    }
}

// The union tail of MaterialShaderArgument. Load_MaterialArgumentDef reads
// nothing except for types 1 and 7, and then only when the slot held -1.
static void finishShaderArgument(Reader &r, Prelink &z, Prelink::Loc e,
                                 uint16_t type, uint32_t uTag) {
    if ((type == 1 || type == 7) && uTag != TAG_NULL) {
        if (uTag == TAG_INLINE) {
            Prelink::Loc lit = z.alloc(OUT, 16, 4);   // float[4]
            r.bytes(z.at(lit), 16);
            b4Reserve(3, 16, lit);
            z.putPtr(e, 8, lit);
        } else {
            putStructOffsetRef(z, e, 8, uTag);
        }
    } else {
        // codeConst / codeSampler / nameHash: a plain 32-bit value, not a
        // pointer. It lives in the low half of the widened union.
        memcpy(z.at(e) + 8, &uTag, 4);
    }
}

// MaterialPass: 20 -> 40. vertexDecl@0, vertexShader@8, pixelShader@16,
// the four counters@24, args@32.
static void tMaterialPass(Reader &r, Prelink &z, Prelink::Loc obj) {
    uint32_t declTag = r.u32();
    uint32_t vsTag   = r.u32();
    uint32_t psTag   = r.u32();
    uint8_t  counts[4] = {0, 0, 0, 0};
    r.bytes(counts, 4);
    uint32_t argsTag = r.u32();

    memcpy(z.at(obj) + 24, counts, 4);

    if (declTag == TAG_INLINE) {
        Prelink::Loc d = z.alloc(OUT, SZ_VERTEX_DECL, 8);
        b4Reserve(3, 108, d);
        tMaterialVertexDeclaration(r, z, d);
        z.putPtr(obj, 0, d);
    } else if (declTag != TAG_NULL) {
        putStructOffsetRef(z, obj, 0, declTag);
    }

    if (vsTag == TAG_INLINE) {
        Prelink::Loc s = z.alloc(OUT, SZ_SHADER, 8);
        b4Reserve(3, 16, s);
        tMaterialShader(r, z, s);
        z.putPtr(obj, 8, s);
    } else if (vsTag != TAG_NULL) {
        putStructOffsetRef(z, obj, 8, vsTag);
    }

    if (psTag == TAG_INLINE) {
        Prelink::Loc s = z.alloc(OUT, SZ_SHADER, 8);
        b4Reserve(3, 16, s);
        tMaterialShader(r, z, s);
        z.putPtr(obj, 16, s);
    } else if (psTag != TAG_NULL) {
        putStructOffsetRef(z, obj, 16, psTag);
    }

    if (argsTag != TAG_NULL) {
        const int n = counts[0] + counts[1] + counts[2];
        Prelink::Loc a = z.alloc(OUT, (size_t)n * SZ_SHADER_ARG, 8);
        b4Reserve(3, (uint32_t)n * 8, a);

        // Array loaders read every fixed record first, then the per-element
        // data -- Load_MaterialShaderArgumentArray (db_load.cpp:2226).
        std::vector<uint16_t> types(n), dests(n);
        std::vector<uint32_t> uTags(n);
        for (int i = 0; i < n; ++i) {
            types[i] = r.u16(); dests[i] = r.u16(); uTags[i] = r.u32();
        }
        for (int i = 0; i < n; ++i) {
            Prelink::Loc e{a.blk, a.off + (uint32_t)i * SZ_SHADER_ARG};
            putU16(z, e, 0, types[i]);
            putU16(z, e, 2, dests[i]);
            finishShaderArgument(r, z, e, types[i], uTags[i]);
        }
        z.putPtr(obj, 32, a);
    }
}

// MaterialTechnique: {char* name; u16 flags; u16 passCount; MaterialPass
// passArray[]}. The engine asserts the passes sit immediately after the
// 8-byte header in the stream, so header and passes are one block.
// Note the order: the passes come BEFORE the name string.
static Prelink::Loc tMaterialTechnique(Reader &r, Prelink &z) {
    uint32_t nameTag   = r.u32();
    uint16_t flags     = r.u16();
    uint16_t passCount = r.u16();

    Prelink::Loc t = z.alloc(OUT, SZ_TECH_HDR + (size_t)passCount * SZ_PASS, 8);
    b4Reserve(3, 8 + (uint32_t)passCount * 20, t);
    putU16(z, t, 8, flags);
    putU16(z, t, 10, passCount);

    // Load_MaterialPassArray reads all the fixed 20-byte records as one
    // block, then walks them; tMaterialPass mirrors that per element.
    for (uint16_t i = 0; i < passCount; ++i) {
        Prelink::Loc p{t.blk, t.off + SZ_TECH_HDR + (uint32_t)i * SZ_PASS};
        tMaterialPass(r, z, p);
    }

    putXStringFromTag(r, z, t, 0, nameTag);
    return t;
}

// MaterialTechniqueSet: 528 -> 1056. Fixed block is the name tag, four bytes
// of flags, and 130 technique tags; then the name string, then each technique.
static void tMaterialTechniqueSet(Reader &r, Prelink &z, Prelink::Loc obj) {
    uint32_t nameTag = r.u32();
    uint8_t  hdr[4] = {0, 0, 0, 0};
    r.bytes(hdr, 4);                // worldVertFormat, unused, techsetFlags
    memcpy(z.at(obj) + 8, hdr, 4);

    uint32_t techTags[130];
    for (int i = 0; i < 130; ++i) techTags[i] = r.u32();

    putXStringFromTag(r, z, obj, 0, nameTag);

    for (int i = 0; i < 130; ++i) {
        const uint32_t field = 16 + (uint32_t)i * 8;
        if (techTags[i] == TAG_INLINE) {
            Prelink::Loc t = tMaterialTechnique(r, z);
            z.putPtr(obj, field, t);
        } else if (techTags[i] != TAG_NULL) {
            putStructOffsetRef(z, obj, field, techTags[i]);
        }
    }
}

// ---- images -----------------------------------------------------------------
// GfxImage: 52 -> 72. The texture union at offset 0 doubles as the load-def
// pointer while the asset is on disk; Load_GfxImage (db_load.cpp:1966) reads
// the fixed block, then the name, then walks that pointer.

enum { AT_IMAGE = 8, SZ_IMAGE = 72 };

// GfxImageLoadDef: {u8 levelCount; u8 flags; pad2; int format; int
// resourceSize; u8 data[]}. No pointers, so the header keeps its x86 shape --
// only the trailing pixel blob varies in length.
static Prelink::Loc tGfxImageLoadDef(Reader &r, Prelink &z, Prelink::Loc img) {
    uint8_t  levelCount = 0, flags = 0;
    r.bytes(&levelCount, 1);
    r.bytes(&flags, 1);
    r.u16();                                  // padding
    int32_t format       = r.i32();
    int32_t resourceSize = r.i32();

    const uint32_t total = 12 + (uint32_t)(resourceSize > 0 ? resourceSize : 0);
    Prelink::Loc def = z.alloc(OUT, total, 4);
    tempReserve(total);            // Load_GfxTextureLoad -> block 0

    uint8_t *d = z.at(def);
    d[0] = levelCount;
    d[1] = flags;
    memcpy(d + 4, &format, 4);
    memcpy(d + 8, &resourceSize, 4);
    if (resourceSize > 0) r.bytes(d + 12, (size_t)resourceSize);

    z.putPtr(img, 0, def);                    // GfxImage.texture.loadDef
    return def;
}

static void tGfxImage(Reader &r, Prelink &z, Prelink::Loc obj) {
    uint32_t texTag = r.u32();                // texture union @0
    uint8_t  mid[8];                          // mapType..track @4
    r.bytes(mid, 8);
    uint8_t  card[8] = {0};                   // cardMemory @12
    r.bytes(card, 8);
    uint8_t  dims[8] = {0};                   // width..streaming @20
    r.bytes(dims, 8);
    uint32_t baseSize   = r.u32();            // @28
    r.u32();                                  // pixels -- runtime, dropped
    uint32_t loadedSize = r.u32();            // @36
    uint8_t  skipped    = 0;
    r.bytes(&skipped, 1);
    r.skip(3);                                // padding
    uint32_t nameTag = r.u32();               // @44
    uint32_t hash    = r.u32();               // @48

    uint8_t *o = z.at(obj);
    memcpy(o + 8,  mid,  4);                  // mapType, semantic, category, delayLoad
    memcpy(o + 12, mid + 4, 4);               // picmip, noPicmip, track
    memcpy(o + 16, card, 8);
    memcpy(o + 24, dims, 8);
    memcpy(o + 32, &baseSize, 4);
    memcpy(o + 48, &loadedSize, 4);
    o[52] = skipped;
    memcpy(o + 64, &hash, 4);

    putXStringFromTag(r, z, obj, 56, nameTag);

    // Load_GfxTextureLoad: -1 and -2 both mean the load def follows inline.
    if (texTag == TAG_INLINE || texTag == TAG_ALIAS) {
        uint32_t slot = (texTag == TAG_ALIAS) ? insertPointerSlot() : 0;
        Prelink::Loc def = tGfxImageLoadDef(r, z, obj);
        if (texTag == TAG_ALIAS) g_aliasMap[slot] = def;
    } else if (texTag != TAG_NULL) {
        putAssetHandleRef(z, obj, 0, texTag);
    }
}

// ---- materials --------------------------------------------------------------
// Material: 192 -> 208. Load_Material (db_load.cpp:2454) reads the whole fixed
// block, then walks info.name, the technique set, and three tables.

enum { AT_MATERIAL = 6, SZ_MATERIAL = 208, SZ_MAT_INFO = 40, SZ_TEXDEF = 24 };

static void tMaterial(Reader &r, Prelink &z, Prelink::Loc obj) {
    // --- fixed block, 192 bytes ---
    Prelink::Loc info{obj.blk, obj.off};        // MaterialInfo sits at offset 0
    uint32_t infoNameTag = r.u32();
    uint8_t  infoHead[8] = {0};
    r.bytes(infoHead, 8);
    r.skip(4);
    uint8_t  infoTail[24];
    r.bytes(infoTail, 24);

    uint8_t stateBitsEntry[130];
    r.bytes(stateBitsEntry, 130);

    uint8_t counts[6];                          // texture, constant, stateBits,
    r.bytes(counts, 6);                         // stateFlags, cameraRegion, mips

    uint32_t techTag  = r.u32();
    uint32_t texTag   = r.u32();
    uint32_t constTag = r.u32();
    uint32_t sbTag    = r.u32();

    uint8_t *o = z.at(obj);
    memcpy(o + 8,   infoHead, 8);
    memcpy(o + 16,  infoTail, 24);
    memcpy(o + 40,  stateBitsEntry, 130);
    memcpy(o + 170, counts, 6);

    // --- then the referenced data, in Load_Material's order ---
    putXStringFromTag(r, z, info, 0, infoNameTag);

    if (techTag == TAG_INLINE || techTag == TAG_ALIAS) {
        Prelink::Loc t = z.alloc(OUT, SZ_TECHSET, 8);
        tempReserve(528);      // Load_MaterialTechniqueSetPtr -> block 0
        if (techTag == TAG_ALIAS) g_aliasMap[insertPointerSlot()] = t;
        tMaterialTechniqueSet(r, z, t);
        z.putPtr(obj, 176, t);
    } else if (techTag != TAG_NULL) {
        putAssetHandleRef(z, obj, 176, techTag);
    }

    const int nTex   = counts[0];
    const int nConst = counts[1];
    const int nSb    = counts[2];

    if (texTag == TAG_INLINE) {
        Prelink::Loc tbl = z.alloc(OUT, (size_t)nTex * SZ_TEXDEF, 8);
        const uint32_t tblX86 = b4Reserve(3, (uint32_t)nTex * 16, tbl);

        // Array loader: every fixed 16-byte record first, then per-element data.
        std::vector<uint32_t> hashes(nTex), uTags(nTex);
        std::vector<uint8_t>  flags(nTex * 8);
        for (int i = 0; i < nTex; ++i) {
            hashes[i] = r.u32();
            r.bytes(&flags[i * 8], 8);          // nameStart..pad[3]
            uTags[i]  = r.u32();
        }
        for (int i = 0; i < nTex; ++i) {
            Prelink::Loc e{tbl.blk, tbl.off + (uint32_t)i * SZ_TEXDEF};
            uint8_t *d = z.at(e);
            memcpy(d, &hashes[i], 4);
            memcpy(d + 4, &flags[i * 8], 8);

            const uint8_t semantic = flags[i * 8 + 3];
            if (semantic == 11) {
                fprintf(stderr, "material texture %d is water (semantic 11); "
                                "water_t is not transcoded yet\n", i);
                exit(5);
            }
            const uint32_t uSlot = tblX86 + (uint32_t)i * 16 + 12;  // .u
            if (uTags[i] == TAG_INLINE || uTags[i] == TAG_ALIAS) {
                Prelink::Loc im = z.alloc(OUT, SZ_IMAGE, 8);
                tempReserve(52);   // Load_GfxImagePtr -> block 0
                if (uTags[i] == TAG_ALIAS) g_aliasMap[insertPointerSlot()] = im;
                noteAliasSlot(uSlot, im);
                tGfxImage(r, z, im);
                z.putPtr(e, 16, im);
            } else if (uTags[i] != TAG_NULL) {
                noteAliasSlot(uSlot, putAssetHandleRef(z, e, 16, uTags[i]));
            }
        }
        z.putPtr(obj, 184, tbl);
    } else if (texTag != TAG_NULL) {
        putStructOffsetRef(z, obj, 184, texTag);
    }

    if (constTag == TAG_INLINE) {
        // AllocLoad_GfxPackedVertex0 -> DB_AllocStreamPos(15), align 16.
        // MaterialConstantDef has no pointers, so it stays 32 bytes.
        const uint32_t n = (uint32_t)nConst * 32;
        Prelink::Loc tbl = z.alloc(OUT, n, 16);
        b4Reserve(15, n, tbl);
        r.bytes(z.at(tbl), n);
        z.putPtr(obj, 192, tbl);
    } else if (constTag != TAG_NULL) {
        putStructOffsetRef(z, obj, 192, constTag);
    }

    if (sbTag == TAG_INLINE) {
        const uint32_t n = (uint32_t)nSb * 8;    // GfxStateBits, no pointers
        Prelink::Loc tbl = z.alloc(OUT, n, 4);
        b4Reserve(3, n, tbl);
        r.bytes(z.at(tbl), n);
        z.putPtr(obj, 200, tbl);
    } else if (sbTag != TAG_NULL) {
        putStructOffsetRef(z, obj, 200, sbTag);
    }
}

// ---- small default assets ---------------------------------------------------

enum { AT_PHYSPRESET = 1, AT_LIGHTDEF = 18, AT_XGLOBALS = 39 };
enum { SZ_PHYSPRESET = 96, SZ_XGLOBALS = 48, SZ_LIGHTDEF = 32 };
enum { AT_PHYSCONSTRAINTS = 2, SZ_PHYSCONSTRAINT = 192, SZ_PHYSCONSTRAINTS = 3088 };

// PhysPreset: 84 -> 96. Two strings, everything else scalar.
static void tPhysPreset(Reader &r, Prelink &z, Prelink::Loc obj) {
    uint32_t nameTag = r.u32();
    uint8_t  mid[24];                  // flags, mass, bounce, friction,
    r.bytes(mid, 24);                  // bulletForceScale, explosiveForceScale
    uint32_t sndTag  = r.u32();
    uint8_t  tail[52];                 // piecesSpread..buoyancyBoxMax
    r.bytes(tail, 52);

    uint8_t *o = z.at(obj);
    memcpy(o + 8,  mid, 24);
    memcpy(o + 40, tail, 52);

    putXStringFromTag(r, z, obj, 0, nameTag);
    putXStringFromTag(r, z, obj, 32, sndTag);
}

// Load_MaterialHandle (db_load.cpp:2504): -1 and -2 both mean an inline
// Material follows; any other non-zero value is an offset alias.
static void tMaterialHandle(Reader &r, Prelink &z, Prelink::Loc obj,
                            uint32_t field, uint32_t tag, uint32_t x86slot = 0) {
    if (tag == TAG_INLINE || tag == TAG_ALIAS) {
        Prelink::Loc m = z.alloc(OUT, SZ_MATERIAL, 8);
        tempReserve(192);      // Load_MaterialHandle -> block 0
        if (tag == TAG_ALIAS) g_aliasMap[insertPointerSlot()] = m;
        noteAliasSlot(x86slot, m);
        tMaterial(r, z, m);
        z.putPtr(obj, field, m);
    } else if (tag != TAG_NULL) {
        noteAliasSlot(x86slot, putAssetHandleRef(z, obj, field, tag));
    } else {
        z.putPtr(obj, field, Prelink::none());
    }
}

// XGlobals: 40 -> 48. Name plus scalars, nothing else.
static void tXGlobals(Reader &r, Prelink &z, Prelink::Loc obj) {
    uint32_t nameTag = r.u32();
    uint8_t  rest[36];
    r.bytes(rest, 36);
    memcpy(z.at(obj) + 8, rest, 36);
    putXStringFromTag(r, z, obj, 0, nameTag);
}

// GfxLightDef: 16 -> 32. name@0, attenuation@8 (a GfxLightImage, itself
// 8 -> 16 because of the image pointer), lmapLookupStart@24.
static void tGfxLightDef(Reader &r, Prelink &z, Prelink::Loc obj) {
    uint32_t nameTag  = r.u32();
    uint32_t imageTag = r.u32();
    uint8_t  sampler  = 0;
    r.bytes(&sampler, 1);
    r.skip(3);                          // padding
    int32_t  lmapStart = r.i32();

    uint8_t *o = z.at(obj);
    o[16] = sampler;                    // attenuation.samplerState
    memcpy(o + 24, &lmapStart, 4);

    putXStringFromTag(r, z, obj, 0, nameTag);

    if (imageTag == TAG_INLINE || imageTag == TAG_ALIAS) {
        Prelink::Loc im = z.alloc(OUT, SZ_IMAGE, 8);
        tempReserve(52);       // Load_GfxImagePtr -> block 0
        if (imageTag == TAG_ALIAS) g_aliasMap[insertPointerSlot()] = im;
        tGfxImage(r, z, im);
        z.putPtr(obj, 8, im);           // attenuation.image
    } else if (imageTag != TAG_NULL) {
        putAssetHandleRef(z, obj, 8, imageTag);
    }
}

// ---- menu tracing (FFMTRACE=1) ----------------------------------------------
// A menu tree desync only shows up as garbage several structs later, so the
// cheapest check is the window name each menuDef and itemDef reads: while the
// cursor is aligned every one of them is a legible identifier.
static const uint8_t *g_zoneBase = nullptr;
static bool g_mtrace = false;
static int  g_menuIdx = -1, g_itemIdx = -1;

static void mtrace(Reader &r, const char *what, int val, uint32_t nameTag) {
    if (!g_mtrace) return;
    char name[64] = "<noname>";
    if (nameTag == TAG_INLINE) {                 // the name follows at r.p
        size_t n = 0;
        while (n < sizeof(name) - 1 && r.p + n < r.end && r.p[n]) {
            name[n] = (char)r.p[n]; ++n;
        }
        name[n] = 0;
    }
    fprintf(stderr, "  T menu=%d item=%d off=%zu %s=%d %s\n",
            g_menuIdx, g_itemIdx, (size_t)(r.p - g_zoneBase), what, val, name);
}

// MenuList: 12 -> 24. Name, count, and an array of menuDef_t pointers.
// menuDef_t is the whole UI menu tree; a default menu file should not have
// any, so stop loudly rather than guess.
enum { AT_MENUFILE = 21, SZ_MENULIST = 24 };

static Prelink::Loc tMenuDef(Reader &r, Prelink &z, Prelink::Loc obj, uint32_t field);

static void tMenuList(Reader &r, Prelink &z, Prelink::Loc obj) {
    uint32_t nameTag   = r.u32();
    int32_t  menuCount = r.i32();
    uint32_t menusTag  = r.u32();

    memcpy(z.at(obj) + 8, &menuCount, 4);
    putXStringFromTag(r, z, obj, 0, nameTag);

    if (menusTag != TAG_NULL) {
        Prelink::Loc tbl = z.alloc(OUT, (size_t)menuCount * 8, 8);
        b4Reserve(3, (uint32_t)menuCount * 4, tbl);
        std::vector<uint32_t> tags(menuCount);
        for (int i = 0; i < menuCount; ++i) tags[i] = r.u32();
        for (int i = 0; i < menuCount; ++i) {
            g_menuIdx = i; g_itemIdx = -1;
            // Load_menuDef_ptr tests for -1 and -2; every other non-zero value
            // is an alias that consumes no stream.
            Prelink::Loc e{tbl.blk, tbl.off + (uint32_t)i * 8};
            if (tags[i] == TAG_INLINE || tags[i] == TAG_ALIAS) {
                uint32_t slot = (tags[i] == TAG_ALIAS) ? insertPointerSlot() : 0;
                Prelink::Loc md = tMenuDef(r, z, tbl, (uint32_t)i * 8);
                if (tags[i] == TAG_ALIAS) g_aliasMap[slot] = md;
            } else if (tags[i] != TAG_NULL)
                putAssetHandleRef(z, e, 0, tags[i]);
            else
                z.putPtr(e, 0, Prelink::none());
        }
        z.putPtr(obj, 16, tbl);
    }
}

// PhysConstraint: 168 -> 192. Three pointers (target_bone1, target_bone2,
// material) push everything after them along. The uint16 targetname and
// target_ent fields are script string indices; Load_ScriptString reads
// nothing from the stream, so they carry over as-is.
// Reads one 168-byte fixed record and returns its three pointer tags; the
// referenced strings come later, after the whole block (Load_PhysConstraints
// reads all 2696 bytes, then the name, then each constraint's data).
struct PhysConstraintTags { uint32_t bone1, bone2, mat; };

static PhysConstraintTags tPhysConstraintFixed(Reader &r, Prelink &z,
                                               Prelink::Loc obj) {
    PhysConstraintTags t;
    uint8_t  head[20];
    r.bytes(head, 20);
    t.bone1 = r.u32();
    uint8_t  mid[12];
    r.bytes(mid, 12);
    t.bone2 = r.u32();
    uint8_t  body[100];
    r.bytes(body, 100);
    t.mat = r.u32();
    uint8_t  tail[24];
    r.bytes(tail, 24);

    uint8_t *o = z.at(obj);
    memcpy(o,       head, 20);
    memcpy(o + 32,  mid,  12);
    memcpy(o + 56,  body, 100);
    memcpy(o + 168, tail, 24);
    return t;
}

static void tPhysConstraints(Reader &r, Prelink &z, Prelink::Loc obj) {
    uint32_t nameTag = r.u32();
    uint32_t count   = r.u32();
    memcpy(z.at(obj) + 8, &count, 4);

    PhysConstraintTags tags[16];
    for (int i = 0; i < 16; ++i) {
        Prelink::Loc c{obj.blk, obj.off + 16 + (uint32_t)i * SZ_PHYSCONSTRAINT};
        tags[i] = tPhysConstraintFixed(r, z, c);
    }

    putXStringFromTag(r, z, obj, 0, nameTag);

    for (int i = 0; i < 16; ++i) {
        Prelink::Loc c{obj.blk, obj.off + 16 + (uint32_t)i * SZ_PHYSCONSTRAINT};
        putXStringFromTag(r, z, c, 24, tags[i].bone1);
        putXStringFromTag(r, z, c, 48, tags[i].bone2);

        tMaterialHandle(r, z, c, 160, tags[i].mat);
    }
}

// ---- xanim ------------------------------------------------------------------
// XAnimParts: 104 -> 152. A long chain of optional arrays; the load order in
// Load_XAnimParts (db_load.cpp:1007) is name, names, notify, deltaPart, then
// the six data arrays, then indices.
//
// Allocation alignments differ per array, and getting them wrong desyncs the
// block-4 cursor: AllocLoad_raw_byte is 1, AllocLoad_XBlendInfo is 2,
// AllocLoad_FxElemVisStateSample is 4.

enum { AT_XANIM = 4, SZ_XANIM = 152 };

static void tSimpleArray(Reader &r, Prelink &z, Prelink::Loc obj, uint32_t field,
                         uint32_t tag, uint32_t count, uint32_t elemSize,
                         uint32_t allocParam) {
    if (tag == TAG_NULL) { z.putPtr(obj, field, Prelink::none()); return; }
    const uint32_t n = count * elemSize;
    Prelink::Loc buf = z.alloc(OUT, n ? n : 1, elemSize);
    if (n) r.bytes(z.at(buf), n);
    b4Reserve(allocParam, n, buf);
    z.putPtr(obj, field, buf);
}

// Array loaders come in two flavours. tSimpleArray is for the ones that take
// any non-zero pointer as "the data follows inline". The ones that test the
// pointer against -1 first -- XModel's bone arrays, most of WeaponDef -- treat
// every other non-zero value as an offset that consumes no stream, so they go
// through here instead. Getting the two mixed up silently eats or leaves
// behind a whole array.
static void tOffsetOrArray(Reader &r, Prelink &z, Prelink::Loc obj, uint32_t field,
                           uint32_t tag, uint32_t count, uint32_t elemSize,
                           uint32_t allocParam) {
    if (tag == TAG_NULL)   { z.putPtr(obj, field, Prelink::none()); return; }
    if (tag != TAG_INLINE) { putStructOffsetRef(z, obj, field, tag); return; }
    tSimpleArray(r, z, obj, field, tag, count, elemSize, allocParam);
}

// XAnimDeltaPart and friends. The `indices` field in XAnimPartTransFrames and
// XAnimDeltaPartQuatDataFrames is a placeholder: the real index array is
// written inline right after the header, so both are variable-length. Element
// width is 2 bytes when the animation has 256 frames or more, 1 otherwise.

// XAnimPartTrans: header 8 (LP64) + XAnimPartTransData.
// When size != 0 the union holds XAnimPartTransFrames: mins[3]@0, size[3]@12,
// frames@24, then the inline index array at 32.
static void tXAnimPartTrans(Reader &r, Prelink &z, Prelink::Loc obj,
                            uint32_t field, uint16_t numframes) {
    uint16_t size = r.u16();
    uint8_t  smallTrans = 0;
    r.bytes(&smallTrans, 1);
    r.skip(1);                              // padding

    const uint32_t idxWidth = (numframes >= 256) ? 2u : 1u;
    const uint32_t idxCount = size ? (uint32_t)size + 1u : 0u;
    const uint32_t idxBytes = idxCount * idxWidth;

    const uint32_t total = size ? (8 + 32 + idxBytes) : (8 + 12);
    Prelink::Loc t = z.alloc(OUT, total, 8);
    b4Reserve(3, size ? (4 + 28 + idxBytes) : (4 + 12), t);

    uint8_t *o = z.at(t);
    memcpy(o, &size, 2);
    o[2] = smallTrans;

    if (!size) {
        r.bytes(o + 8, 12);                 // a single vec3 lives in the union
        z.putPtr(obj, field, t);
        return;
    }

    r.bytes(o + 8, 24);                     // mins[3], size[3]
    uint32_t framesTag = r.u32();           // the frames pointer slot
    if (idxBytes) r.bytes(o + 40, idxBytes);// inline index array

    if (framesTag != TAG_NULL) {
        const uint32_t elem  = smallTrans ? 3u : 6u;   // byte or ushort triple
        const uint32_t align = smallTrans ? 0u : 3u;
        const uint32_t n     = idxCount * elem;
        Prelink::Loc f = z.alloc(OUT, n ? n : 1, smallTrans ? 1 : 2);
        if (n) r.bytes(z.at(f), n);
        b4Reserve(align, n, f);
        z.putPtr(t, 32, f);                 // XAnimPartTransFrames.frames
    }

    z.putPtr(obj, field, t);
}

// XAnimDeltaPartQuat: header 8 (LP64) + XAnimDeltaPartQuatData.
// When size != 0 the union holds frames@0 then the inline index array at 8.
static void tXAnimDeltaPartQuat(Reader &r, Prelink &z, Prelink::Loc obj,
                                uint32_t field, uint16_t numframes) {
    uint16_t size = r.u16();
    r.skip(2);                              // padding

    const uint32_t idxWidth = (numframes >= 256) ? 2u : 1u;
    const uint32_t idxCount = size ? (uint32_t)size + 1u : 0u;
    const uint32_t idxBytes = idxCount * idxWidth;

    const uint32_t total = size ? (8 + 8 + idxBytes) : (8 + 4);
    Prelink::Loc q = z.alloc(OUT, total, 8);
    b4Reserve(3, size ? (4 + 4 + idxBytes) : (4 + 4), q);

    uint8_t *o = z.at(q);
    memcpy(o, &size, 2);

    if (!size) {
        r.bytes(o + 8, 4);                  // a single XQuat2
        z.putPtr(obj, field, q);
        return;
    }

    uint32_t framesTag = r.u32();
    if (idxBytes) r.bytes(o + 16, idxBytes);

    if (framesTag != TAG_NULL) {
        const uint32_t n = idxCount * 4;    // XQuat2 is int16[2]
        Prelink::Loc f = z.alloc(OUT, n ? n : 1, 2);
        if (n) r.bytes(z.at(f), n);
        b4Reserve(3, n, f);
        z.putPtr(q, 8, f);                  // XAnimDeltaPartQuatDataFrames.frames
    }

    z.putPtr(obj, field, q);
}

// XAnimDeltaPart: 8 -> 16, two pointers.
static void tXAnimDeltaPart(Reader &r, Prelink &z, Prelink::Loc obj,
                            uint32_t field, uint16_t numframes) {
    uint32_t transTag = r.u32();
    uint32_t quatTag  = r.u32();

    Prelink::Loc d = z.alloc(OUT, 16, 8);
    b4Reserve(3, 8, d);

    if (transTag != TAG_NULL) tXAnimPartTrans(r, z, d, 0, numframes);
    if (quatTag  != TAG_NULL) tXAnimDeltaPartQuat(r, z, d, 8, numframes);

    z.putPtr(obj, field, d);
}

static void tXAnimParts(Reader &r, Prelink &z, Prelink::Loc obj) {
    if (getenv("FFDBG")) {
        fprintf(stderr, "  xanim raw header:\n");
        for (int i = 0; i < 112; i += 16) {
            fprintf(stderr, "    %3d:", i);
            for (int j = 0; j < 16; ++j) fprintf(stderr, " %02x", r.p[i + j]);
            fprintf(stderr, "  ");
            for (int j = 0; j < 16; ++j) {
                unsigned char c = r.p[i + j];
                fputc((c >= 32 && c < 127) ? c : '.', stderr);
            }
            fprintf(stderr, "\n");
        }
    }

    uint32_t nameTag = r.u32();
    uint8_t  counts[16] = {0};        // dataByteCount .. bStreamable (@4..19)
    r.bytes(counts, 16);
    uint32_t streamedFileSize = r.u32();
    uint8_t  bones[13];               // boneCount[10], notifyCount, assetType, isDefault
    r.bytes(bones, 13);
    r.skip(3);                        // padding
    uint8_t  mid[24];                 // randomDataShortCount .. loopEntryTime
    r.bytes(mid, 24);

    uint32_t namesTag   = r.u32();
    uint32_t byteTag    = r.u32();
    uint32_t shortTag   = r.u32();
    uint32_t intTag     = r.u32();
    uint32_t rShortTag  = r.u32();
    uint32_t rByteTag   = r.u32();
    uint32_t rIntTag    = r.u32();
    uint32_t indicesTag = r.u32();
    uint32_t notifyTag  = r.u32();
    uint32_t deltaTag   = r.u32();

    uint8_t *o = z.at(obj);
    memcpy(o + 8,  counts, 16);
    memcpy(o + 24, &streamedFileSize, 4);
    memcpy(o + 28, bones, 13);
    memcpy(o + 44, mid, 24);

    // counts we need, read back from the copied bytes
    uint16_t dataByteCount, dataShortCount, dataIntCount;
    uint16_t rDataByteCount, rDataIntCount, numframes;
    memcpy(&dataByteCount,  counts + 0, 2);
    memcpy(&dataShortCount, counts + 2, 2);
    memcpy(&dataIntCount,   counts + 4, 2);
    memcpy(&rDataByteCount, counts + 6, 2);
    memcpy(&rDataIntCount,  counts + 8, 2);
    memcpy(&numframes,      counts + 10, 2);
    uint32_t rDataShortCount, indexCount;
    memcpy(&rDataShortCount, mid + 0, 4);
    memcpy(&indexCount,      mid + 4, 4);

    if (getenv("FFDBG")) {
        fprintf(stderr,
            "  xanim: numframes=%u bones9=%u notifyCount=%u\n"
            "    counts: byte=%u short=%u int=%u rByte=%u rInt=%u rShort=%u idx=%u\n"
            "    tags: names=%08x notify=%08x delta=%08x\n"
            "          byte=%08x short=%08x int=%08x\n"
            "          rShort=%08x rByte=%08x rInt=%08x indices=%08x\n",
            numframes, bones[9], bones[10],
            dataByteCount, dataShortCount, dataIntCount,
            rDataByteCount, rDataIntCount, rDataShortCount, indexCount,
            namesTag, notifyTag, deltaTag,
            byteTag, shortTag, intTag,
            rShortTag, rByteTag, rIntTag, indicesTag);
    }

    putXStringFromTag(r, z, obj, 0, nameTag);

    // names: ScriptString array, boneCount[9] entries of 2 bytes
    tSimpleArray(r, z, obj, 72, namesTag, bones[9], 2, 1);

    if (notifyTag != TAG_NULL) {
        // XAnimNotifyInfo is 8 bytes: {ScriptString name; float time}. No
        // pointers, so it keeps its size.
        tSimpleArray(r, z, obj, 136, notifyTag, bones[10], 8, 3);
    }

    if (deltaTag != TAG_NULL)
        tXAnimDeltaPart(r, z, obj, 144, numframes);

    tSimpleArray(r, z, obj, 80,  byteTag,   dataByteCount,   1, 0);
    tSimpleArray(r, z, obj, 88,  shortTag,  dataShortCount,  2, 1);
    tSimpleArray(r, z, obj, 96,  intTag,    dataIntCount,    4, 3);
    tSimpleArray(r, z, obj, 104, rShortTag, rDataShortCount, 2, 1);
    tSimpleArray(r, z, obj, 112, rByteTag,  rDataByteCount,  1, 0);
    tSimpleArray(r, z, obj, 120, rIntTag,   rDataIntCount,   4, 3);

    // indices: bytes when numframes < 256, shorts otherwise
    if (numframes >= 256)
        tSimpleArray(r, z, obj, 128, indicesTag, indexCount, 2, 1);
    else
        tSimpleArray(r, z, obj, 128, indicesTag, indexCount, 1, 0);
}

// XSurfaceCollisionTree: 40 -> 56. Bottom of the xmodel tree -- neither
// XSurfaceCollisionNode (16) nor XSurfaceCollisionLeaf (2) holds a pointer.
enum { SZ_COLLTREE = 56 };

static void tXSurfaceCollisionTree(Reader &r, Prelink &z, Prelink::Loc obj,
                                   uint32_t field) {
    Prelink::Loc t = z.alloc(OUT, SZ_COLLTREE, 8);
    b4Reserve(3, 40, t);

    uint8_t  bounds[24];               // trans[3], scale[3]
    r.bytes(bounds, 24);
    uint32_t nodeCount = r.u32();
    uint32_t nodesTag  = r.u32();
    uint32_t leafCount = r.u32();
    uint32_t leafsTag  = r.u32();

    uint8_t *o = z.at(t);
    memcpy(o,      bounds, 24);
    memcpy(o + 24, &nodeCount, 4);
    memcpy(o + 40, &leafCount, 4);

    if (nodesTag != TAG_NULL) {
        const uint32_t n = nodeCount * 16;   // AllocLoad_GfxPackedVertex0 -> 16
        Prelink::Loc b = z.alloc(OUT, n ? n : 1, 16);
        if (n) r.bytes(z.at(b), n);
        b4Reserve(15, n, b);
        z.putPtr(t, 32, b);
    }

    if (leafsTag != TAG_NULL) {
        const uint32_t n = leafCount * 2;    // AllocLoad_XBlendInfo -> 2
        Prelink::Loc b = z.alloc(OUT, n ? n : 1, 2);
        if (n) r.bytes(z.at(b), n);
        b4Reserve(1, n, b);
        z.putPtr(t, 48, b);
    }

    z.putPtr(obj, field, t);
}

// XSurface: 68 -> 104. Load_XSurface (db_load.cpp:1843) reads the fixed block,
// then vertInfo, then verts0, then vertList, then triIndices. vb0 and
// indexBuffer are runtime D3D objects and stay null.

enum { SZ_XSURFACE = 104, SZ_VERTINFO = 24, SZ_RIGIDVERT = 16 };

// Load_XSurfaceArray reads every 68-byte surface header as one run before it
// walks them, so the two phases have to stay apart: a model with more than one
// surface desyncs the moment they are interleaved.
struct SurfTags {
    uint32_t triIdx, blend, tension, verts0, vertList;
    int16_t  vc[4];
    uint16_t vertListCount, vertCount, triCount;
};

static SurfTags tXSurfaceFixed(Reader &r, Prelink &z, Prelink::Loc obj) {
    SurfTags t;
    uint8_t  head[12];                 // tileMode .. baseVertIndex
    r.bytes(head, 12);
    t.triIdx = r.u32();

    r.bytes(t.vc, 8);                  // vertInfo.vertCount[4]
    t.blend   = r.u32();
    t.tension = r.u32();

    t.verts0 = r.u32();
    r.u32();                           // vb0 -- runtime object
    t.vertList = r.u32();
    r.u32();                           // indexBuffer -- runtime object
    uint8_t  partBits[20];
    r.bytes(partBits, 20);

    uint8_t *o = z.at(obj);
    memcpy(o,       head, 12);
    memcpy(o + 24,  t.vc, 8);          // vertInfo.vertCount
    memcpy(o + 80,  partBits, 20);

    t.vertListCount = head[1];
    memcpy(&t.vertCount, head + 4, 2);
    memcpy(&t.triCount,  head + 6, 2);
    return t;
}

static void tXSurfaceRefs(Reader &r, Prelink &z, Prelink::Loc obj, const SurfTags &t) {
    const uint32_t blendTag = t.blend, tensionTag = t.tension;
    const uint32_t verts0Tag = t.verts0, vertListTag = t.vertList, triIdxTag = t.triIdx;
    const int16_t *vcInfo = t.vc;
    const uint16_t vertListCount = t.vertListCount, vertCount = t.vertCount,
                   triCount = t.triCount;

    // vertInfo lives inline at offset 24; its two pointers are at 32 and 40.
    Prelink::Loc vi{obj.blk, obj.off + 24};

    {
        const uint32_t n = 7u * (uint32_t)vcInfo[3] + 5u * (uint32_t)vcInfo[2]
                         + 3u * (uint32_t)vcInfo[1] + (uint32_t)vcInfo[0];
        tOffsetOrArray(r, z, vi, 8, blendTag, n, 2, 1);
    }

    {
        // Load_floatArray(1, 12 * sum(vertCount)), AllocLoad_FxElemVisStateSample.
        const uint32_t n = 12u * ((uint32_t)vcInfo[0] + (uint32_t)vcInfo[1]
                                + (uint32_t)vcInfo[2] + (uint32_t)vcInfo[3]);
        tOffsetOrArray(r, z, vi, 16, tensionTag, n, 4, 3);
    }

    if (verts0Tag == TAG_INLINE) {
        // AllocLoad_GfxPackedVertex0 -> align 16. GfxPackedVertex has no
        // pointers, so it keeps its 32 bytes.
        const uint32_t n = (uint32_t)vertCount * 32;
        Prelink::Loc b = z.alloc(OUT, n ? n : 1, 16);
        if (n) r.bytes(z.at(b), n);
        b4Reserve(15, n, b);
        z.putPtr(obj, 48, b);
    } else if (verts0Tag != TAG_NULL) {
        putStructOffsetRef(z, obj, 48, verts0Tag);
    }

    if (vertListTag == TAG_INLINE) {
        Prelink::Loc tbl = z.alloc(OUT, (size_t)vertListCount * SZ_RIGIDVERT, 8);
        b4Reserve(3, (uint32_t)vertListCount * 12, tbl);
        std::vector<uint32_t> treeTags(vertListCount);
        for (uint32_t i = 0; i < vertListCount; ++i) {
            Prelink::Loc e{tbl.blk, tbl.off + i * SZ_RIGIDVERT};
            uint8_t counts[8] = {0};
            r.bytes(counts, 8);
            memcpy(z.at(e), counts, 8);
            treeTags[i] = r.u32();
        }
        for (uint32_t i = 0; i < vertListCount; ++i) {
            Prelink::Loc e{tbl.blk, tbl.off + i * SZ_RIGIDVERT};
            if (treeTags[i] == TAG_INLINE) {
                tXSurfaceCollisionTree(r, z, e, 8);
            } else if (treeTags[i] != TAG_NULL) {
                putStructOffsetRef(z, e, 8, treeTags[i]);
            }
        }
        z.putPtr(obj, 64, tbl);
    } else if (vertListTag != TAG_NULL) {
        putStructOffsetRef(z, obj, 64, vertListTag);
    }

    if (triIdxTag == TAG_INLINE) {
        const uint32_t n = 3u * (uint32_t)triCount * 2u;
        Prelink::Loc b = z.alloc(OUT, n ? n : 1, 16);
        if (n) r.bytes(z.at(b), n);
        b4Reserve(15, n, b);
        z.putPtr(obj, 16, b);
    } else if (triIdxTag != TAG_NULL) {
        putStructOffsetRef(z, obj, 16, triIdxTag);
    }
}

// ---- xmodel -----------------------------------------------------------------
// XModel: 252 -> 328. Load_XModel (db_load.cpp:3171) is linear: the fixed
// block, then the name, then a run of arrays sized from counters already in
// the header. XModelLodInfo, XBoneInfo, DObjAnimMat and XModelHighMipBounds
// hold no pointers and keep their sizes.

enum { AT_XMODEL = 5, SZ_XMODEL = 328, SZ_COLLSURF = 48 };
enum { SZ_COLLTRI = 48 };   // confirm against XModelCollTri_s

static void tXModel(Reader &r, Prelink &z, Prelink::Loc obj) {
    uint32_t nameTag = r.u32();
    uint8_t  cnt[4];                       // numBones, numRootBones, numsurfs, lodRampType
    r.bytes(cnt, 4);

    uint32_t boneNamesTag = r.u32();
    uint32_t parentTag    = r.u32();
    uint32_t quatsTag     = r.u32();
    uint32_t transTag     = r.u32();
    uint32_t partClassTag = r.u32();
    uint32_t baseMatTag   = r.u32();
    uint32_t surfsTag     = r.u32();
    uint32_t matHandlesTag= r.u32();

    uint8_t lodInfo[128];
    r.bytes(lodInfo, 128);
    uint8_t lodAuto = 0;
    r.bytes(&lodAuto, 1);
    r.skip(3);

    uint32_t collSurfsTag = r.u32();
    int32_t  numCollSurfs = r.i32();
    int32_t  contents     = r.i32();
    uint32_t boneInfoTag  = r.u32();

    uint8_t bounds[28];                    // radius, mins[3], maxs[3]
    r.bytes(bounds, 28);
    uint8_t lods[4] = {0};                 // numLods, collLod
    r.bytes(lods, 4);

    uint32_t streamTag    = r.u32();       // streamInfo.highMipBounds
    int32_t  memUsage     = r.i32();
    int32_t  flags        = r.i32();
    uint8_t  bad = 0;
    r.bytes(&bad, 1);
    r.skip(3);
    uint32_t physPresetTag = r.u32();
    uint8_t  numCollmaps = 0;
    r.bytes(&numCollmaps, 1);
    r.skip(3);
    uint32_t collmapsTag  = r.u32();
    uint32_t physConstrTag= r.u32();

    uint8_t *o = z.at(obj);
    memcpy(o + 8,   cnt, 4);
    memcpy(o + 80,  lodInfo, 128);
    o[208] = lodAuto;
    memcpy(o + 224, &numCollSurfs, 4);
    memcpy(o + 228, &contents, 4);
    memcpy(o + 240, bounds, 28);
    memcpy(o + 268, lods, 4);
    memcpy(o + 280, &memUsage, 4);
    memcpy(o + 284, &flags, 4);
    o[288] = bad;
    o[304] = numCollmaps;

    const uint32_t nBones = cnt[0];
    const uint32_t nRoot  = cnt[1];
    const uint32_t nSurfs = cnt[2];
    const uint32_t nDiff  = nBones - nRoot;

    putXStringFromTag(r, z, obj, 0, nameTag);

    tOffsetOrArray(r, z, obj, 16, boneNamesTag, nBones, 2, 1);

    tOffsetOrArray(r, z, obj, 24, parentTag, nDiff, 1, 0);

    tOffsetOrArray(r, z, obj, 32, quatsTag, nDiff * 4, 2, 1);

    tOffsetOrArray(r, z, obj, 40, transTag, nDiff * 4, 4, 3);

    tOffsetOrArray(r, z, obj, 48, partClassTag, nBones, 1, 0);

    tOffsetOrArray(r, z, obj, 56, baseMatTag, nBones, 32, 3);

    if (surfsTag != TAG_NULL) {
        Prelink::Loc tbl = z.alloc(OUT, (size_t)nSurfs * SZ_XSURFACE, 8);
        b4Reserve(3, nSurfs * 68, tbl);
        std::vector<SurfTags> st(nSurfs);
        for (uint32_t i = 0; i < nSurfs; ++i)
            st[i] = tXSurfaceFixed(r, z, Prelink::Loc{tbl.blk, tbl.off + i * SZ_XSURFACE});
        for (uint32_t i = 0; i < nSurfs; ++i)
            tXSurfaceRefs(r, z, Prelink::Loc{tbl.blk, tbl.off + i * SZ_XSURFACE}, st[i]);
        z.putPtr(obj, 64, tbl);
    }

    if (matHandlesTag != TAG_NULL) {
        Prelink::Loc tbl = z.alloc(OUT, (size_t)nSurfs * 8, 8);
        const uint32_t tblX86 = b4Reserve(3, nSurfs * 4, tbl);
        // Load_MaterialHandleArray reads the whole 4*count pointer run first.
        std::vector<uint32_t> tags(nSurfs);
        for (uint32_t i = 0; i < nSurfs; ++i) tags[i] = r.u32();
        for (uint32_t i = 0; i < nSurfs; ++i)
            tMaterialHandle(r, z, Prelink::Loc{tbl.blk, tbl.off + i * 8}, 0, tags[i],
                            tblX86 + i * 4);
        z.putPtr(obj, 72, tbl);
    }

    if (collSurfsTag != TAG_NULL) {
        Prelink::Loc tbl = z.alloc(OUT, (size_t)numCollSurfs * SZ_COLLSURF, 8);
        b4Reserve(3, (uint32_t)numCollSurfs * 44, tbl);
        std::vector<uint32_t> triTags(numCollSurfs);
        std::vector<int32_t>  triCounts(numCollSurfs);
        for (int i = 0; i < numCollSurfs; ++i) {
            Prelink::Loc c{tbl.blk, tbl.off + (uint32_t)i * SZ_COLLSURF};
            triTags[i]   = r.u32();
            triCounts[i] = r.i32();
            uint8_t rest[36];               // mins, maxs, boneIdx, contents, surfFlags
            r.bytes(rest, 36);
            memcpy(z.at(c) + 8, &triCounts[i], 4);
            memcpy(z.at(c) + 12, rest, 36);
        }
        for (int i = 0; i < numCollSurfs; ++i) {
            Prelink::Loc c{tbl.blk, tbl.off + (uint32_t)i * SZ_COLLSURF};
            tSimpleArray(r, z, c, 0, triTags[i], (uint32_t)triCounts[i], SZ_COLLTRI, 3);
        }
        z.putPtr(obj, 216, tbl);
    }

    if (boneInfoTag != TAG_NULL)
        tSimpleArray(r, z, obj, 232, boneInfoTag, nBones, 44, 3);

    if (streamTag != TAG_NULL)
        tSimpleArray(r, z, obj, 272, streamTag, nSurfs, 16, 3);

    if (physPresetTag == TAG_INLINE || physPresetTag == TAG_ALIAS) {
        Prelink::Loc p = z.alloc(OUT, SZ_PHYSPRESET, 8);
        tempReserve(84);       // Load_PhysPresetPtr -> block 0
        if (physPresetTag == TAG_ALIAS) g_aliasMap[insertPointerSlot()] = p;
        tPhysPreset(r, z, p);
        z.putPtr(obj, 296, p);
    } else if (physPresetTag != TAG_NULL) {
        putAssetHandleRef(z, obj, 296, physPresetTag);
    }

    if (collmapsTag != TAG_NULL) {
        fprintf(stderr, "xmodel has collmaps; PhysGeomList is not transcoded yet\n");
        exit(9);
    }

    if (physConstrTag == TAG_INLINE || physConstrTag == TAG_ALIAS) {
        Prelink::Loc p = z.alloc(OUT, SZ_PHYSCONSTRAINTS, 8);
        tempReserve(2696);     // Load_PhysConstraintsPtr -> block 0
        if (physConstrTag == TAG_ALIAS) g_aliasMap[insertPointerSlot()] = p;
        tPhysConstraints(r, z, p);
        z.putPtr(obj, 320, p);
    } else if (physConstrTag != TAG_NULL) {
        putAssetHandleRef(z, obj, 320, physConstrTag);
    }
}

// Reads the rpn array of an ExpressionStatement whose 16-byte header has
// already been consumed, and hooks it into `dest` at `field`.
enum { SZ_EXPRSTMT = 24, SZ_EXPRRPN = 24 };
enum { VAL_STRING = 2 };   // expDataType: VAL_INT=0, VAL_FLOAT=1, VAL_STRING=2

static void tExprRpnArray(Reader &r, Prelink &z, Prelink::Loc dest,
                          uint32_t field, uint32_t tag, int32_t numRpn) {
    if (tag == TAG_NULL) return;
    if (tag != TAG_INLINE) { putStructOffsetRef(z, dest, field, tag); return; }

    Prelink::Loc tbl = z.alloc(OUT, (size_t)numRpn * SZ_EXPRRPN, 8);
    b4Reserve(3, (uint32_t)numRpn * 12, tbl);

    std::vector<int32_t>  types(numRpn), aVals(numRpn);
    std::vector<uint32_t> bVals(numRpn);
    for (int i = 0; i < numRpn; ++i) {
        types[i] = r.i32(); aVals[i] = r.i32(); bVals[i] = r.u32();
    }
    for (int i = 0; i < numRpn; ++i) {
        Prelink::Loc c{tbl.blk, tbl.off + (uint32_t)i * SZ_EXPRRPN};
        uint8_t *d = z.at(c);
        memcpy(d, &types[i], 4);
        memcpy(d + 8, &aVals[i], 4);
        if (types[i] == 0 && aVals[i] == VAL_STRING)
            putXStringFromTag(r, z, c, 16, bVals[i]);
        else
            memcpy(d + 16, &bVals[i], 4);
    }
    z.putPtr(dest, field, tbl);
}

// ---- expressions ------------------------------------------------------------
// ExpressionStatement: 16 -> 24. expressionRpn 12 -> 24, because the union at
// its tail holds an Operand whose internals can be a pointer.
// Only VAL_STRING operands consume stream (Load_operandInternalDataUnion).



static void tExpressionStatement(Reader &r, Prelink &z, Prelink::Loc obj,
                                 uint32_t field) {
    uint32_t fileTag = r.u32();
    int32_t  line    = r.i32();
    int32_t  numRpn  = r.i32();
    uint32_t rpnTag  = r.u32();

    Prelink::Loc e{obj.blk, obj.off + field};
    uint8_t *o = z.at(e);
    memcpy(o + 8,  &line, 4);
    memcpy(o + 12, &numRpn, 4);

    putXStringFromTag(r, z, e, 0, fileTag);

    if (rpnTag == TAG_NULL) return;
    if (rpnTag != TAG_INLINE) { putStructOffsetRef(z, e, 16, rpnTag); return; }

    Prelink::Loc tbl = z.alloc(OUT, (size_t)numRpn * SZ_EXPRRPN, 8);
    b4Reserve(3, (uint32_t)numRpn * 12, tbl);

    // The array loader reads every 12-byte record first, then walks them.
    std::vector<int32_t>  types(numRpn), aVals(numRpn);
    std::vector<uint32_t> bVals(numRpn);
    for (int i = 0; i < numRpn; ++i) {
        types[i] = r.i32();
        aVals[i] = r.i32();
        bVals[i] = r.u32();
    }
    for (int i = 0; i < numRpn; ++i) {
        Prelink::Loc c{tbl.blk, tbl.off + (uint32_t)i * SZ_EXPRRPN};
        uint8_t *d = z.at(c);
        memcpy(d, &types[i], 4);
        memcpy(d + 8, &aVals[i], 4);          // dataType, or cmdIdx

        if (types[i] == 0 && aVals[i] == VAL_STRING)
            putXStringFromTag(r, z, c, 16, bVals[i]);
        else
            memcpy(d + 16, &bVals[i], 4);     // int, float or command index
    }
    z.putPtr(e, 16, tbl);
}

// ---- menu building blocks ---------------------------------------------------

enum { SZ_WINDOWDEF = 176, SZ_EVENTSCRIPT = 64, SZ_EVENTHANDLER = 24 };

// windowDef_t: 164 -> 176. Two rectDef_s of 24 bytes each hold no pointers;
// only name, group and background need resolving.
// windowDef_t in two phases: the caller reads its whole fixed block first,
// then resolves references. Load_Window(0) never re-reads the 164 bytes.
struct WindowTags { uint32_t name, group, background; };

static WindowTags tWindowDefFixed(Reader &r, Prelink &z, Prelink::Loc w) {
    WindowTags t;
    t.name = r.u32();
    uint8_t rects[48];
    r.bytes(rects, 48);
    t.group = r.u32();
    uint8_t body[104];
    r.bytes(body, 104);
    t.background = r.u32();

    uint8_t *o = z.at(w);
    memcpy(o + 8,  rects, 48);
    memcpy(o + 64, body, 104);
    return t;
}

// `winX86` is the window's own x86 offset in block 4, or 0 when the enclosing
// struct lives in the temp block (a menuDef does).
static void tWindowDefRefs(Reader &r, Prelink &z, Prelink::Loc w, WindowTags t,
                           uint32_t winX86 = 0) {
    putXStringFromTag(r, z, w, 0,  t.name);
    putXStringFromTag(r, z, w, 56, t.group);

    tMaterialHandle(r, z, w, 168, t.background, winX86 ? winX86 + 160 : 0);
}

// ScriptCondition: 16 -> 24, a linked list of {fireOnTrue, constructID,
// blockID, next}. Load_ScriptConditionNext re-reads the 16-byte block for each
// link, so the recursion mirrors the loader.
enum { SZ_SCRIPTCONDITION = 24 };

static void tScriptCondition(Reader &r, Prelink &z, Prelink::Loc obj, uint32_t field) {
    Prelink::Loc c = z.alloc(OUT, SZ_SCRIPTCONDITION, 8);
    b4Reserve(3, 16, c);

    uint8_t  head[12];                     // fireOnTrue + pad, constructID, blockID
    r.bytes(head, 12);
    uint32_t nextTag = r.u32();

    memcpy(z.at(c), head, 12);
    z.putPtr(obj, field, c);

    if (nextTag != TAG_NULL) tScriptCondition(r, z, c, 16);
    else                     z.putPtr(c, 16, Prelink::none());
}

// GenericEventScript: 44 -> 64. A linked list; each node carries a condition
// expression and an optional ScriptCondition chain.
static void tGenericEventScript(Reader &r, Prelink &z, Prelink::Loc obj,
                                uint32_t field) {
    Prelink::Loc s = z.alloc(OUT, SZ_EVENTSCRIPT, 8);
    b4Reserve(3, 44, s);

    uint32_t prereqTag = r.u32();
    // condition is inline at offset 8
    uint32_t condFileTag = r.u32();
    int32_t  condLine    = r.i32();
    int32_t  condNumRpn  = r.i32();
    uint32_t condRpnTag  = r.u32();
    int32_t  type        = r.i32();
    uint8_t  fireOnTrue  = 0;
    r.bytes(&fireOnTrue, 1);
    r.skip(3);
    uint32_t actionTag   = r.u32();
    int32_t  blockID     = r.i32();
    int32_t  constructID = r.i32();
    uint32_t nextTag     = r.u32();

    uint8_t *o = z.at(s);
    memcpy(o + 16, &condLine, 4);          // condition.line
    memcpy(o + 20, &condNumRpn, 4);        // condition.numRpn
    memcpy(o + 32, &type, 4);
    o[36] = fireOnTrue;
    memcpy(o + 48, &blockID, 4);
    memcpy(o + 52, &constructID, 4);

    z.putPtr(obj, field, s);

    if (prereqTag != TAG_NULL) tScriptCondition(r, z, s, 0);
    else                       z.putPtr(s, 0, Prelink::none());

    // The condition's own referenced data follows, in Load_ExpressionStatement
    // order: filename, then the rpn array.
    putXStringFromTag(r, z, s, 8, condFileTag);
    if (condRpnTag == TAG_INLINE) {
        Prelink::Loc tbl = z.alloc(OUT, (size_t)condNumRpn * SZ_EXPRRPN, 8);
        b4Reserve(3, (uint32_t)condNumRpn * 12, tbl);
        std::vector<int32_t>  types(condNumRpn), aVals(condNumRpn);
        std::vector<uint32_t> bVals(condNumRpn);
        for (int i = 0; i < condNumRpn; ++i) {
            types[i] = r.i32(); aVals[i] = r.i32(); bVals[i] = r.u32();
        }
        for (int i = 0; i < condNumRpn; ++i) {
            Prelink::Loc c{tbl.blk, tbl.off + (uint32_t)i * SZ_EXPRRPN};
            uint8_t *d = z.at(c);
            memcpy(d, &types[i], 4);
            memcpy(d + 8, &aVals[i], 4);
            if (types[i] == 0 && aVals[i] == VAL_STRING)
                putXStringFromTag(r, z, c, 16, bVals[i]);
            else
                memcpy(d + 16, &bVals[i], 4);
        }
        z.putPtr(s, 24, tbl);              // condition.rpn
    } else if (condRpnTag != TAG_NULL) {
        putStructOffsetRef(z, s, 24, condRpnTag);
    }

    putXStringFromTag(r, z, s, 40, actionTag);

    if (nextTag != TAG_NULL)
        tGenericEventScript(r, z, s, 56);
}

// GenericEventHandler and ItemKeyHandler share the shape {a, script, next},
// 12 -> 24.
static void tEventHandler(Reader &r, Prelink &z, Prelink::Loc obj,
                          uint32_t field, bool firstIsString) {
    Prelink::Loc h = z.alloc(OUT, SZ_EVENTHANDLER, 8);
    b4Reserve(3, 12, h);

    uint32_t aTag      = r.u32();          // name (handler) or key (key handler)
    uint32_t scriptTag = r.u32();
    uint32_t nextTag   = r.u32();

    z.putPtr(obj, field, h);

    if (firstIsString) putXStringFromTag(r, z, h, 0, aTag);
    else               memcpy(z.at(h), &aTag, 4);

    if (scriptTag != TAG_NULL) tGenericEventScript(r, z, h, 8);
    if (nextTag   != TAG_NULL) tEventHandler(r, z, h, 16, firstIsString);
}

// ---- menu leaf definitions --------------------------------------------------
// textExp_s (cmd.h:7), imageDef_s and ownerDrawDef_s are all a lone
// ExpressionStatement, 16 -> 24. Their loaders (Load_textExp_t at
// db_load.cpp:5574, Load_imageDef_t, Load_ownerDrawDef_t) allocate with
// AllocLoad_FxElemVisStateSample (align 4), read the 16-byte block, then call
// Load_ExpressionStatement(0) -- which does not re-read that header, so the
// whole thing is one tExpressionStatement at offset 0.
// None of these ptr loaders tests for -1, unlike ExpressionStatement::rpn:
// any non-zero value means "follows inline", so there is no offset case.
enum { SZ_EXPRWRAPPER = SZ_EXPRSTMT };

static void tExprWrapper(Reader &r, Prelink &z, Prelink::Loc obj, uint32_t field) {
    Prelink::Loc e = z.alloc(OUT, SZ_EXPRWRAPPER, 8);
    b4Reserve(3, 16, e);
    z.putPtr(obj, field, e);
    tExpressionStatement(r, z, e, 0);
}

// editFieldDef_s: 36 flat ints and floats, no pointers, so LP64 keeps 36.
enum { SZ_EDITFIELDDEF = 36 };

static void tEditFieldDef(Reader &r, Prelink &z, Prelink::Loc obj, uint32_t field) {
    Prelink::Loc e = z.alloc(OUT, SZ_EDITFIELDDEF, 4);
    b4Reserve(3, 36, e);
    r.bytes(z.at(e), 36);
    z.putPtr(obj, field, e);
}

// enumDvarDef_s: a single string, 4 -> 8.
enum { SZ_ENUMDVARDEF = 8 };

static void tEnumDvarDef(Reader &r, Prelink &z, Prelink::Loc obj, uint32_t field) {
    Prelink::Loc e = z.alloc(OUT, SZ_ENUMDVARDEF, 8);
    b4Reserve(3, 4, e);
    uint32_t nameTag = r.u32();
    z.putPtr(obj, field, e);
    putXStringFromTag(r, z, e, 0, nameTag);
}

// gameMsgDef_s: two ints, 8 -> 8, nothing to resolve.
enum { SZ_GAMEMSGDEF = 8 };

static void tGameMsgDef(Reader &r, Prelink &z, Prelink::Loc obj, uint32_t field) {
    Prelink::Loc g = z.alloc(OUT, SZ_GAMEMSGDEF, 4);
    b4Reserve(3, 8, g);
    r.bytes(z.at(g), 8);
    z.putPtr(obj, field, g);
}

// multiDef_s: 396 -> 656. dvarList[32] and dvarStr[32] are string arrays,
// dvarValue[32] plain floats. Load_multiDef_t reaches dvarList through a cast
// of the struct base -- it is simply the field at offset 0.
enum { SZ_MULTIDEF = 656 };

static void tMultiDef(Reader &r, Prelink &z, Prelink::Loc obj, uint32_t field) {
    Prelink::Loc md = z.alloc(OUT, SZ_MULTIDEF, 8);
    b4Reserve(3, 396, md);

    uint32_t listTags[32], strTags[32];
    for (int i = 0; i < 32; ++i) listTags[i] = r.u32();
    for (int i = 0; i < 32; ++i) strTags[i]  = r.u32();
    uint8_t values[128];
    r.bytes(values, 128);
    int32_t count      = r.i32();
    int32_t enterPress = r.i32();
    int32_t strDef     = r.i32();

    uint8_t *o = z.at(md);
    memcpy(o + 512, values, 128);
    memcpy(o + 640, &count, 4);
    memcpy(o + 644, &enterPress, 4);
    memcpy(o + 648, &strDef, 4);

    z.putPtr(obj, field, md);

    for (int i = 0; i < 32; ++i)
        putXStringFromTag(r, z, md, (uint32_t)i * 8, listTags[i]);
    for (int i = 0; i < 32; ++i)
        putXStringFromTag(r, z, md, 256 + (uint32_t)i * 8, strTags[i]);
}

// MenuCell: 12 -> 16. stringValue is a raw maxChars-byte buffer, not an
// XString, so it is read with AllocLoad_raw_byte + Load_charArray.
// MenuRow: 24 -> 40. eventName and onFocusEventName are fixed 32-byte buffers.
enum { SZ_MENUCELL = 16, SZ_MENUROW = 40, SZ_LISTBOXDEF = 688 };

static void tMenuCellArray(Reader &r, Prelink &z, Prelink::Loc row,
                           uint32_t field, int32_t numColumns) {
    if (numColumns <= 0) { z.putPtr(row, field, Prelink::none()); return; }

    Prelink::Loc tbl = z.alloc(OUT, (size_t)numColumns * SZ_MENUCELL, 8);
    b4Reserve(3, (uint32_t)numColumns * 12, tbl);

    std::vector<int32_t>  types(numColumns), maxChars(numColumns);
    std::vector<uint32_t> valTags(numColumns);
    for (int i = 0; i < numColumns; ++i) {
        types[i] = r.i32(); maxChars[i] = r.i32(); valTags[i] = r.u32();
    }
    for (int i = 0; i < numColumns; ++i) {
        Prelink::Loc c{tbl.blk, tbl.off + (uint32_t)i * SZ_MENUCELL};
        uint8_t *d = z.at(c);
        memcpy(d, &types[i], 4);
        memcpy(d + 4, &maxChars[i], 4);
        if (valTags[i] == TAG_NULL || maxChars[i] <= 0) {
            z.putPtr(c, 8, Prelink::none());
        } else {
            Prelink::Loc buf = z.alloc(OUT, (size_t)maxChars[i], 1);
            r.bytes(z.at(buf), (size_t)maxChars[i]);
            b4Reserve(0, (uint32_t)maxChars[i], buf);
            z.putPtr(c, 8, buf);
        }
    }
    z.putPtr(row, field, tbl);
}

static void tMenuRowArray(Reader &r, Prelink &z, Prelink::Loc lb, uint32_t field,
                          int32_t maxRows, int32_t numColumns) {
    if (maxRows <= 0) { z.putPtr(lb, field, Prelink::none()); return; }

    Prelink::Loc tbl = z.alloc(OUT, (size_t)maxRows * SZ_MENUROW, 8);
    b4Reserve(3, (uint32_t)maxRows * 24, tbl);

    struct RowTags { uint32_t cells, eventName, onFocusEventName; };
    std::vector<RowTags> rt(maxRows);
    for (int i = 0; i < maxRows; ++i) {
        rt[i].cells            = r.u32();
        rt[i].eventName        = r.u32();
        rt[i].onFocusEventName = r.u32();
        uint8_t tail[12];                  // disableArg + pad, status, name
        r.bytes(tail, 12);
        memcpy(z.at(Prelink::Loc{tbl.blk, tbl.off + (uint32_t)i * SZ_MENUROW}) + 24,
               tail, 12);
    }
    for (int i = 0; i < maxRows; ++i) {
        Prelink::Loc row{tbl.blk, tbl.off + (uint32_t)i * SZ_MENUROW};
        if (rt[i].cells != TAG_NULL) tMenuCellArray(r, z, row, 0, numColumns);
        else                         z.putPtr(row, 0, Prelink::none());

        for (int k = 0; k < 2; ++k) {      // eventName@8, onFocusEventName@16
            uint32_t tag = k ? rt[i].onFocusEventName : rt[i].eventName;
            uint32_t off = k ? 16u : 8u;
            if (tag == TAG_NULL) { z.putPtr(row, off, Prelink::none()); continue; }
            Prelink::Loc buf = z.alloc(OUT, 32, 1);
            r.bytes(z.at(buf), 32);
            b4Reserve(0, 32, buf);
            z.putPtr(row, off, buf);
        }
    }
    z.putPtr(lb, field, tbl);
}

// listBoxDef_s: 668 -> 688. Everything up to selectIcon is plain data, so the
// first 640 bytes copy across unchanged; only the three material handles and
// the row table widen. rows is sized by maxRows, cells by numColumns.
static void tListBoxDef(Reader &r, Prelink &z, Prelink::Loc obj, uint32_t field) {
    Prelink::Loc lb = z.alloc(OUT, SZ_LISTBOXDEF, 8);
    b4Reserve(3, 668, lb);

    uint8_t  head[640];                    // mousePos .. elementBackgroundColor
    r.bytes(head, 640);
    uint32_t selIconTag = r.u32();
    uint32_t bgItemTag  = r.u32();
    uint32_t hlTexTag   = r.u32();
    int32_t  noBlink    = r.i32();
    uint32_t rowsTag    = r.u32();
    int32_t  maxRows    = r.i32();
    int32_t  rowCount   = r.i32();

    uint8_t *o = z.at(lb);
    memcpy(o, head, 640);
    memcpy(o + 664, &noBlink, 4);
    memcpy(o + 680, &maxRows, 4);
    memcpy(o + 684, &rowCount, 4);

    int32_t numColumns;
    memcpy(&numColumns, head + 28, 4);

    z.putPtr(obj, field, lb);

    tMaterialHandle(r, z, lb, 640, selIconTag);
    tMaterialHandle(r, z, lb, 648, bgItemTag);
    tMaterialHandle(r, z, lb, 656, hlTexTag);

    if (rowsTag != TAG_NULL) tMenuRowArray(r, z, lb, 672, maxRows, numColumns);
    else                     z.putPtr(lb, 672, Prelink::none());
}

// focusDefData_t and textDefData_t are unions discriminated by the type of the
// enclosing itemDef, not by anything in the struct that holds them, so that
// type has to be threaded down from tItemDef.
// A non-zero tag for a type the switch does not cover loads nothing at all in
// the game; the slot is left null here rather than pointing at nothing.
static void tFocusDefData(Reader &r, Prelink &z, Prelink::Loc f, uint32_t field,
                          int32_t itemType, uint32_t tag) {
    if (tag == TAG_NULL) { z.putPtr(f, field, Prelink::none()); return; }
    switch (itemType) {
    case 4:
        tListBoxDef(r, z, f, field); break;
    case 0xA:
        tMultiDef(r, z, f, field); break;
    case 5: case 0xD: case 7: case 0xE: case 0x1E:
    case 9: case 0xC: case 0x10: case 8: case 0x16:
        tEditFieldDef(r, z, f, field); break;
    case 0xB:
        tEnumDvarDef(r, z, f, field); break;
    default:
        z.putPtr(f, field, Prelink::none()); break;
    }
}

// focusItemDef_s: 24 -> 48. Four strings, an ItemKeyHandler chain and the
// type-discriminated focusTypeData union.
enum { SZ_FOCUSITEMDEF = 48 };

static void tFocusItemDef(Reader &r, Prelink &z, Prelink::Loc obj, uint32_t field,
                          int32_t itemType) {
    Prelink::Loc f = z.alloc(OUT, SZ_FOCUSITEMDEF, 8);
    b4Reserve(3, 24, f);

    uint32_t enterTextTag = r.u32();
    uint32_t exitTextTag  = r.u32();
    uint32_t enterTag     = r.u32();
    uint32_t exitTag      = r.u32();
    uint32_t onKeyTag     = r.u32();
    uint32_t focusDataTag = r.u32();

    z.putPtr(obj, field, f);

    putXStringFromTag(r, z, f, 0,  enterTextTag);
    putXStringFromTag(r, z, f, 8,  exitTextTag);
    putXStringFromTag(r, z, f, 16, enterTag);
    putXStringFromTag(r, z, f, 24, exitTag);

    if (onKeyTag != TAG_NULL) tEventHandler(r, z, f, 32, false);  // ItemKeyHandler
    else                      z.putPtr(f, 32, Prelink::none());

    tFocusDefData(r, z, f, 40, itemType, focusDataTag);
}

// textDef_s: 68 -> 80. The item type in code_post_gfx_mp's default menu is 3,
// which Load_itemDefData_t routes here.
enum { SZ_TEXTDEF = 80 };

static void tTextDef(Reader &r, Prelink &z, Prelink::Loc obj, uint32_t field,
                     int32_t itemType) {
    Prelink::Loc t = z.alloc(OUT, SZ_TEXTDEF, 8);
    b4Reserve(3, 68, t);

    uint8_t  head[56];                 // textRect + the eight ints
    r.bytes(head, 56);
    uint32_t textTag    = r.u32();
    uint32_t expDataTag = r.u32();
    uint32_t typeDataTag= r.u32();

    memcpy(z.at(t), head, 56);
    z.putPtr(obj, field, t);

    putXStringFromTag(r, z, t, 56, textTag);

    if (expDataTag != TAG_NULL) tExprWrapper(r, z, t, 64);   // textExp_s
    else                        z.putPtr(t, 64, Prelink::none());

    // textTypeData -- Load_textDefData_t (db_load.cpp:5545)
    if (typeDataTag == TAG_NULL) {
        z.putPtr(t, 72, Prelink::none());
    } else switch (itemType) {
    case 3: case 4: case 0x15: case 0x14: case 0xA: case 5: case 0xD:
    case 7: case 0xE: case 0x1E: case 9: case 0xC: case 0x10: case 8:
    case 0xB: case 0x16:
        tFocusItemDef(r, z, t, 72, itemType); break;
    case 0xF:
        tGameMsgDef(r, z, t, 72); break;
    default:
        z.putPtr(t, 72, Prelink::none()); break;
    }
}

// animParamsDef_t: 108 -> 120. Load_animParamsDef_t (db_load.cpp:5344) reads
// the fixed block, the name string, then an optional GenericEventHandler.
// UIAnimInfo: 236 -> 272. Load_UIAnimInfo (db_load.cpp:5660) reads the whole
// block -- the two inline animParamsDef_t scratch states included, whose own
// name/onEvent pointers it never resolves -- then the animStates array.
enum { SZ_ANIMPARAMS = 120, SZ_UIANIMINFO = 272 };

// Copy the pointer-free body of an x86 animParamsDef_t into its LP64 record.
static void animParamsBody(uint8_t *d, const uint8_t *src) {
    memcpy(d + 8,   src + 4,  24);     // rectClient
    memcpy(d + 32,  src + 28, 4);      // borderSize
    memcpy(d + 36,  src + 32, 64);     // fore/back/border/outlineColor
    memcpy(d + 100, src + 96, 8);      // textScale, rotation
}

static void tAnimParamsDef(Reader &r, Prelink &z, Prelink::Loc obj, uint32_t field) {
    Prelink::Loc a = z.alloc(OUT, SZ_ANIMPARAMS, 8);
    b4Reserve(3, 108, a);

    uint8_t blk[108];
    r.bytes(blk, 108);
    uint32_t nameTag, onEventTag;
    memcpy(&nameTag, blk, 4);
    memcpy(&onEventTag, blk + 104, 4);

    animParamsBody(z.at(a), blk);
    z.putPtr(obj, field, a);

    putXStringFromTag(r, z, a, 0, nameTag);
    if (onEventTag != TAG_NULL) tEventHandler(r, z, a, 112, true);
    else                        z.putPtr(a, 112, Prelink::none());
}

static void tUIAnimInfo(Reader &r, Prelink &z, Prelink::Loc obj, uint32_t field) {
    Prelink::Loc u = z.alloc(OUT, SZ_UIANIMINFO, 8);
    b4Reserve(3, 236, u);

    int32_t  stateCount = r.i32();
    uint32_t statesTag  = r.u32();
    uint8_t  cur[108] = {0}, nxt[108] = {0}, tail[12] = {0};
    r.bytes(cur, 108);
    r.bytes(nxt, 108);
    r.bytes(tail, 12);                 // animating, animStartTime, animDuration

    uint8_t *o = z.at(u);
    memcpy(o, &stateCount, 4);
    animParamsBody(o + 16,  cur);      // currentAnimState
    animParamsBody(o + 136, nxt);      // nextAnimState
    memcpy(o + 256, tail, 12);

    z.putPtr(obj, field, u);

    // Load_animParamsDef_ptr tests only against zero: any other value means
    // the record follows inline.
    if (statesTag == TAG_NULL) { z.putPtr(u, 8, Prelink::none()); return; }
    int32_t n = stateCount > 0 ? stateCount : 0;
    Prelink::Loc tbl = z.alloc(OUT, n ? (size_t)n * 8 : 1, 8);
    b4Reserve(3, (uint32_t)n * 4, tbl);
    std::vector<uint32_t> tags(n);
    for (int i = 0; i < n; ++i) tags[i] = r.u32();
    for (int i = 0; i < n; ++i)
        if (tags[i] != TAG_NULL) tAnimParamsDef(r, z, tbl, (uint32_t)i * 8);
    z.putPtr(u, 8, tbl);
}

// itemDef_s: 272 -> 336. Load_itemDef_t (db_load.cpp:5671) walks the window,
// three dvar strings, the type-discriminated typeData union, rectExpData,
// two expressions, the event handler and the animation info. `parent` is a
// runtime backpointer and is never loaded.

enum { SZ_ITEMDEF = 336, SZ_RECTDATA = 96 };

static void tItemDef(Reader &r, Prelink &z, Prelink::Loc obj, uint32_t field) {
    Prelink::Loc it = z.alloc(OUT, SZ_ITEMDEF, 8);
    const uint32_t itX86 = b4Reserve(7, 272, it);   // AllocLoad_itemDef_t, align 8

    // --- the 272-byte fixed block ---
    // window is inline at offset 0; tWindowDef reads its 164 bytes.
    WindowTags wt = tWindowDefFixed(r, z, it);

    uint8_t  mid[12];                      // type, dataType, imageTrack
    r.bytes(mid, 12);
    uint32_t dvarTag       = r.u32();
    uint32_t dvarTestTag   = r.u32();
    uint32_t enableDvarTag = r.u32();
    int32_t  dvarFlags     = r.i32();
    uint32_t typeDataTag   = r.u32();
    r.u32();                               // parent -- runtime only
    uint32_t rectExpTag    = r.u32();

    uint32_t visFileTag = r.u32();
    int32_t  visLine    = r.i32();
    int32_t  visNumRpn  = r.i32();
    uint32_t visRpnTag  = r.u32();
    r.skip(4);                             // x86 padding before showBits

    uint8_t  bits[16];                     // showBits, hideBits
    r.bytes(bits, 16);

    uint32_t foreFileTag = r.u32();
    int32_t  foreLine    = r.i32();
    int32_t  foreNumRpn  = r.i32();
    uint32_t foreRpnTag  = r.u32();

    int32_t  ui3dWindowId = r.i32();
    uint32_t onEventTag   = r.u32();
    uint32_t animInfoTag  = r.u32();
    r.skip(4);                             // trailing x86 padding (align 8)

    uint8_t *o = z.at(it);
    memcpy(o + 176, mid, 12);
    memcpy(o + 216, &dvarFlags, 4);
    memcpy(o + 256, &visLine, 4);          // visibleExp.line
    memcpy(o + 260, &visNumRpn, 4);
    memcpy(o + 272, bits, 16);
    memcpy(o + 296, &foreLine, 4);         // forecolorAExp.line
    memcpy(o + 300, &foreNumRpn, 4);
    memcpy(o + 312, &ui3dWindowId, 4);

    z.putPtr(obj, field, it);

    // --- referenced data, in load order ---
    {
        int32_t ty; memcpy(&ty, mid, 4);
        mtrace(r, "itemDef type", ty, wt.name);
    }
    tWindowDefRefs(r, z, it, wt, itX86);
    putXStringFromTag(r, z, it, 192, dvarTag);
    putXStringFromTag(r, z, it, 200, dvarTestTag);
    putXStringFromTag(r, z, it, 208, enableDvarTag);

    // typeData -- Load_itemDefData_t (db_load.cpp:5620)
    int32_t itemType;
    memcpy(&itemType, mid, 4);
    if (typeDataTag == TAG_NULL) {
        z.putPtr(it, 224, Prelink::none());
    } else switch (itemType) {
    case 1: case 3: case 0xF: case 0x12: case 0x14: case 4: case 0xA:
    case 5: case 0xE: case 7: case 0xD: case 9: case 0xC: case 0x10:
    case 8: case 0xB: case 0x16:
        tTextDef(r, z, it, 224, itemType); break;
    case 2:                                      // imageDef_s
    case 6:                                      // ownerDrawDef_s
        tExprWrapper(r, z, it, 224); break;
    case 0x15: case 0x13:                        // blankButtonDef
        tFocusItemDef(r, z, it, 224, itemType); break;
    default:
        z.putPtr(it, 224, Prelink::none()); break;
    }

    if (rectExpTag != TAG_NULL) {
        Prelink::Loc rd = z.alloc(OUT, SZ_RECTDATA, 8);
        b4Reserve(3, 64, rd);
        // Load_rectData_t (db_load.cpp:5605) reads the whole 64-byte block --
        // all four ExpressionStatement headers -- and only then calls
        // Load_ExpressionStatement(0) four times, which re-read nothing.
        // Interleaving header and referenced data reads the first rpn array
        // out of the following headers.
        uint32_t fileTag[4], rpnTag[4];
        int32_t  numRpn[4];
        for (int i = 0; i < 4; ++i) {
            fileTag[i]    = r.u32();
            int32_t line  = r.i32();
            numRpn[i]     = r.i32();
            rpnTag[i]     = r.u32();
            uint8_t *d = z.at(rd) + (uint32_t)i * SZ_EXPRSTMT;
            memcpy(d + 8,  &line, 4);
            memcpy(d + 12, &numRpn[i], 4);
        }
        for (int i = 0; i < 4; ++i) {
            Prelink::Loc e{rd.blk, rd.off + (uint32_t)i * SZ_EXPRSTMT};
            putXStringFromTag(r, z, e, 0, fileTag[i]);
            tExprRpnArray(r, z, e, 16, rpnTag[i], numRpn[i]);
        }
        z.putPtr(it, 240, rd);
    }

    // visibleExp and forecolorAExp: their headers were read above, only the
    // referenced parts follow now.
    putXStringFromTag(r, z, it, 248, visFileTag);
    tExprRpnArray(r, z, it, 264, visRpnTag, visNumRpn);
    putXStringFromTag(r, z, it, 288, foreFileTag);
    tExprRpnArray(r, z, it, 304, foreRpnTag, foreNumRpn);

    if (onEventTag != TAG_NULL)
        tEventHandler(r, z, it, 320, true);

    if (animInfoTag != TAG_NULL) tUIAnimInfo(r, z, it, 328);
    else                         z.putPtr(it, 328, Prelink::none());
}

// menuDef_t: 400 -> 456. The root of the UI tree.
enum { SZ_MENUDEF = 456 };

static Prelink::Loc tMenuDef(Reader &r, Prelink &z, Prelink::Loc obj, uint32_t field) {
    Prelink::Loc m = z.alloc(OUT, SZ_MENUDEF, 8);
    tempReserve(400);                      // Load_menuDef_ptr -> block 0

    WindowTags wt = tWindowDefFixed(r, z, m);            // window is inline at offset 0

    uint32_t fontTag = r.u32();
    uint8_t  ints15[60];                   // fullScreen .. closeSlideDirection
    r.bytes(ints15, 60);
    uint8_t  initialRect[24];
    r.bytes(initialRect, 24);
    uint8_t  ints4[16] = {0};                   // the four fade/slide counters
    r.bytes(ints4, 16);

    uint32_t onEventTag = r.u32();
    uint32_t onKeyTag   = r.u32();

    uint32_t visFileTag = r.u32();
    int32_t  visLine    = r.i32();
    int32_t  visNumRpn  = r.i32();
    uint32_t visRpnTag  = r.u32();
    r.skip(4);                             // x86 padding before showBits

    uint8_t  bits[16] = {0};
    r.bytes(bits, 16);

    uint32_t allowedTag = r.u32();
    uint32_t soundTag   = r.u32();
    uint8_t  trackCtl[8];                  // imageTrack, control
    r.bytes(trackCtl, 8);
    uint8_t  colors[32];                   // focusColor, disableColor
    r.bytes(colors, 32);

    uint32_t rxFileTag = r.u32();
    int32_t  rxLine    = r.i32();
    int32_t  rxNumRpn  = r.i32();
    uint32_t rxRpnTag  = r.u32();

    uint32_t ryFileTag = r.u32();
    int32_t  ryLine    = r.i32();
    int32_t  ryNumRpn  = r.i32();
    uint32_t ryRpnTag  = r.u32();

    uint32_t itemsTag = r.u32();
    r.skip(4);                             // trailing x86 padding

    uint8_t *o = z.at(m);
    memcpy(o + 184, ints15, 60);
    memcpy(o + 244, initialRect, 24);
    memcpy(o + 268, ints4, 16);
    memcpy(o + 312, &visLine, 4);          // visibleExp.line
    memcpy(o + 316, &visNumRpn, 4);        // visibleExp.numRpn
    memcpy(o + 328, bits, 16);             // showBits, hideBits
    memcpy(o + 360, trackCtl, 8);          // imageTrack, control
    memcpy(o + 368, colors, 32);           // focusColor, disableColor
    memcpy(o + 408, &rxLine, 4);           // rectXExp.line
    memcpy(o + 412, &rxNumRpn, 4);
    memcpy(o + 432, &ryLine, 4);           // rectYExp.line
    memcpy(o + 436, &ryNumRpn, 4);
    int32_t itemCount;
    memcpy(&itemCount, ints15 + 8, 4);

    z.putPtr(obj, field, m);

    // --- referenced data, in Load_menuDef_t order ---
    mtrace(r, "menuDef itemCount", itemCount, wt.name);
    tWindowDefRefs(r, z, m, wt);
    putXStringFromTag(r, z, m, 176, fontTag);

    if (onEventTag != TAG_NULL) tEventHandler(r, z, m, 288, true);
    if (onKeyTag   != TAG_NULL) tEventHandler(r, z, m, 296, false);

    putXStringFromTag(r, z, m, 304, visFileTag);
    tExprRpnArray(r, z, m, 328, visRpnTag, visNumRpn);

    putXStringFromTag(r, z, m, 344, allowedTag);
    putXStringFromTag(r, z, m, 352, soundTag);

    putXStringFromTag(r, z, m, 400, rxFileTag);
    tExprRpnArray(r, z, m, 416, rxRpnTag, rxNumRpn);
    putXStringFromTag(r, z, m, 424, ryFileTag);
    tExprRpnArray(r, z, m, 440, ryRpnTag, ryNumRpn);

    if (itemsTag != TAG_NULL) {
        Prelink::Loc tbl = z.alloc(OUT, (size_t)itemCount * 8, 8);
        b4Reserve(3, (uint32_t)itemCount * 4, tbl);
        std::vector<uint32_t> tags(itemCount);
        for (int i = 0; i < itemCount; ++i) tags[i] = r.u32();
        for (int i = 0; i < itemCount; ++i)
            if (tags[i] != TAG_NULL) {
                g_itemIdx = i;
                tItemDef(r, z, tbl, (uint32_t)i * 8);
            }
        z.putPtr(m, 448, tbl);
    }
    return m;
}    

// ---- fx ---------------------------------------------------------------------
// FxEffectDef -> FxElemDef[] -> visuals / trail / sounds.
// FxEffectDefRef is a union of {const FxEffectDef*; const char* name} and the
// loader always takes the string branch (Load_FxEffectDefRef -> Load_XString);
// the handle is resolved at runtime by Load_FxEffectDefFromName.

enum {
    AT_WEAPON      = 0x18,
    AT_FX          = 0x1C,
    SZ_FXEFFECTDEF = 72,    // x86 60
    SZ_FXELEMDEF   = 336,   // x86 292
    SZ_FXTRAILDEF  = 40,    // x86 28
    SZ_FXMARKVIS   = 16,    // x86 8, two material handles
    SZ_FXVISUALS   = 8      // x86 4, a union of one pointer
};

// A flat array of pointer-free records copies across byte for byte. It cannot
// go through tSimpleArray, which derives the output alignment from the element
// size -- 96, 48 and 20 here are not powers of two.
static void tFlatArray(Reader &r, Prelink &z, Prelink::Loc obj, uint32_t field,
                       uint32_t nbytes, uint32_t allocParam) {
    Prelink::Loc b = z.alloc(OUT, nbytes ? nbytes : 1, 4);
    if (nbytes) r.bytes(z.at(b), nbytes);
    b4Reserve(allocParam, nbytes, b);
    z.putPtr(obj, field, b);
}

// FxElemVisuals: what the 4-byte union holds is decided by the elem type
// (Load_FxElemVisuals, db_load.cpp:3889). Types 8 and 9 load nothing at all --
// FX_CopyVisuals zeroes the slot for them -- so the slot is left null here too.
static void tXModelHandle(Reader &r, Prelink &z, Prelink::Loc obj,
                          uint32_t field, uint32_t tag, uint32_t x86slot = 0);

static void tFxElemVisuals(Reader &r, Prelink &z, Prelink::Loc obj, uint32_t field,
                           uint8_t elemType, uint32_t tag, uint32_t x86slot = 0) {
    switch (elemType) {
    case 7:                                          // XModel
        tXModelHandle(r, z, obj, field, tag, x86slot);
        break;
    case 0xC:                                        // FxEffectDefRef, by name
    case 0xA:                                        // soundName
        putXStringFromTag(r, z, obj, field, tag);
        break;
    case 8: case 9:
        z.putPtr(obj, field, Prelink::none());
        break;
    default:
        tMaterialHandle(r, z, obj, field, tag, x86slot);
        break;
    }
}

// FxElemDefVisuals (db_load.cpp:3947): elemType 11 means an array of
// FxElemMarkVisuals, visualCount > 1 an array of FxElemVisuals, and anything
// else a single inline FxElemVisuals.
static void tFxElemDefVisuals(Reader &r, Prelink &z, Prelink::Loc ed, uint32_t field,
                              uint8_t elemType, uint8_t visualCount, uint32_t tag,
                              uint32_t x86slot = 0) {
    if (elemType == 11) {
        if (tag == TAG_NULL) { z.putPtr(ed, field, Prelink::none()); return; }
        Prelink::Loc tbl = z.alloc(OUT, (size_t)visualCount * SZ_FXMARKVIS, 8);
        const uint32_t tblX86 = b4Reserve(3, (uint32_t)visualCount * 8, tbl);
        std::vector<uint32_t> mats((size_t)visualCount * 2);
        for (size_t i = 0; i < mats.size(); ++i) mats[i] = r.u32();
        for (int i = 0; i < visualCount; ++i) {
            Prelink::Loc mv{tbl.blk, tbl.off + (uint32_t)i * SZ_FXMARKVIS};
            tMaterialHandle(r, z, mv, 0, mats[(size_t)i * 2], tblX86 + (uint32_t)i * 8);
            tMaterialHandle(r, z, mv, 8, mats[(size_t)i * 2 + 1], tblX86 + (uint32_t)i * 8 + 4);
        }
        z.putPtr(ed, field, tbl);
    } else if (visualCount > 1) {
        if (tag == TAG_NULL) { z.putPtr(ed, field, Prelink::none()); return; }
        Prelink::Loc tbl = z.alloc(OUT, (size_t)visualCount * SZ_FXVISUALS, 8);
        const uint32_t tblX86 = b4Reserve(3, (uint32_t)visualCount * 4, tbl);
        std::vector<uint32_t> tags(visualCount);
        for (int i = 0; i < visualCount; ++i) tags[i] = r.u32();
        for (int i = 0; i < visualCount; ++i)
            tFxElemVisuals(r, z, Prelink::Loc{tbl.blk, tbl.off + (uint32_t)i * SZ_FXVISUALS},
                           0, elemType, tags[i], tblX86 + (uint32_t)i * 4);
        z.putPtr(ed, field, tbl);
    } else {
        tFxElemVisuals(r, z, ed, field, elemType, tag, x86slot);
    }
}

// FxTrailDef: 28 -> 40. FxTrailVertex is 20 flat floats and does not widen.
static void tFxTrailDef(Reader &r, Prelink &z, Prelink::Loc obj, uint32_t field) {
    Prelink::Loc td = z.alloc(OUT, SZ_FXTRAILDEF, 8);
    b4Reserve(3, 28, td);

    uint8_t  head[16];                     // scrollTimeMsec .. vertCount
    r.bytes(head, 16);
    uint32_t vertsTag = r.u32();
    int32_t  indCount = r.i32();
    uint32_t indsTag  = r.u32();

    int32_t vertCount;
    memcpy(&vertCount, head + 12, 4);

    uint8_t *o = z.at(td);
    memcpy(o, head, 16);
    memcpy(o + 24, &indCount, 4);

    z.putPtr(obj, field, td);

    if (vertsTag != TAG_NULL)
        tFlatArray(r, z, td, 16, 20u * (uint32_t)(vertCount > 0 ? vertCount : 0), 3);
    else
        z.putPtr(td, 16, Prelink::none());

    if (indsTag != TAG_NULL)                       // AllocLoad_XBlendInfo -> align 2
        tSimpleArray(r, z, td, 32, indsTag, (uint32_t)(indCount > 0 ? indCount : 0), 2, 1);
    else
        z.putPtr(td, 32, Prelink::none());
}

// FxElemDef: 292 -> 336. Two padding holes open up, before velSamples (x86 188)
// and before spawnSound (x86 280); everything else shifts by whole pointers.
struct FxElemTags {
    uint32_t velSamples, visSamples, visuals;
    uint32_t onImpact, onDeath, emitted, attached;
    uint32_t trailDef, spawnSound;
    uint8_t  elemType, visualCount, velIntervals, visIntervals;
};

static FxElemTags tFxElemDefFixed(Reader &r, Prelink &z, Prelink::Loc ed) {
    FxElemTags t;
    uint8_t head[188];                     // flags .. visStateIntervalCount
    r.bytes(head, 188);
    t.velSamples = r.u32();
    t.visSamples = r.u32();
    t.visuals    = r.u32();
    uint8_t coll[24];                      // collMins[3], collMaxs[3]
    r.bytes(coll, 24);
    t.onImpact = r.u32();
    t.onDeath  = r.u32();
    t.emitted  = r.u32();
    uint8_t emit[16];                      // emitDist, emitDistVariance
    r.bytes(emit, 16);
    t.attached = r.u32();
    t.trailDef = r.u32();
    uint8_t tail[12];                      // sortOrder .. lifespanAtMaxWind
    r.bytes(tail, 12);
    uint8_t u[8];                          // FxElemDefUnion
    r.bytes(u, 8);
    t.spawnSound = r.u32();
    uint8_t pivot[8];                      // billboardPivot[2]
    r.bytes(pivot, 8);

    uint8_t *o = z.at(ed);
    memcpy(o,       head, 188);
    memcpy(o + 216, coll, 24);
    memcpy(o + 264, emit, 16);
    memcpy(o + 296, tail, 12);
    memcpy(o + 308, u, 8);
    memcpy(o + 328, pivot, 8);

    t.elemType     = head[184];
    t.visualCount  = head[185];
    t.velIntervals = head[186];
    t.visIntervals = head[187];
    return t;
}

static void tFxElemDefRefs(Reader &r, Prelink &z, Prelink::Loc ed, const FxElemTags &t,
                           uint32_t edX86 = 0) {
    // FxElemVelStateSample is 96 flat bytes, FxElemVisStateSample 48; neither
    // holds a pointer, so both arrays copy across unchanged.
    if (t.velSamples != TAG_NULL)
        tFlatArray(r, z, ed, 192, 96u * ((uint32_t)t.velIntervals + 1), 3);
    else
        z.putPtr(ed, 192, Prelink::none());

    if (t.visSamples != TAG_NULL)
        tFlatArray(r, z, ed, 200, 48u * ((uint32_t)t.visIntervals + 1), 3);
    else
        z.putPtr(ed, 200, Prelink::none());

    tFxElemDefVisuals(r, z, ed, 208, t.elemType, t.visualCount, t.visuals,
                      edX86 ? edX86 + 196 : 0);   // FxElemDef.visuals

    putXStringFromTag(r, z, ed, 240, t.onImpact);
    putXStringFromTag(r, z, ed, 248, t.onDeath);
    putXStringFromTag(r, z, ed, 256, t.emitted);
    putXStringFromTag(r, z, ed, 280, t.attached);

    if (t.trailDef != TAG_NULL) tFxTrailDef(r, z, ed, 288);
    else                        z.putPtr(ed, 288, Prelink::none());

    putXStringFromTag(r, z, ed, 320, t.spawnSound);
}

// FxEffectDef: 60 -> 72. Load_FxElemDefArray reads every 292-byte elem block
// as one run before walking them, so the two phases stay separate here too.
static void tFxEffectDef(Reader &r, Prelink &z, Prelink::Loc obj) {
    uint32_t nameTag = r.u32();
    uint8_t  flags[4];                     // flags, efPriority, reserved[2]
    r.bytes(flags, 4);
    int32_t  totalSize    = r.i32();
    int32_t  msecLooping  = r.i32();
    int32_t  countLooping = r.i32();
    int32_t  countOneShot = r.i32();
    int32_t  countEmit    = r.i32();
    uint32_t elemDefsTag  = r.u32();
    uint8_t  bounds[28];                   // boundingBoxDim[3], boundingSphere[4]
    r.bytes(bounds, 28);

    uint8_t *o = z.at(obj);
    memcpy(o + 8,  flags, 4);
    memcpy(o + 12, &totalSize, 4);
    memcpy(o + 16, &msecLooping, 4);
    memcpy(o + 20, &countLooping, 4);
    memcpy(o + 24, &countOneShot, 4);
    memcpy(o + 28, &countEmit, 4);
    memcpy(o + 40, bounds, 28);

    putXStringFromTag(r, z, obj, 0, nameTag);

    const int32_t count = countEmit + countOneShot + countLooping;
    if (elemDefsTag == TAG_NULL || count <= 0) {
        z.putPtr(obj, 32, Prelink::none());
        return;
    }

    Prelink::Loc tbl = z.alloc(OUT, (size_t)count * SZ_FXELEMDEF, 8);
    const uint32_t tblX86 = b4Reserve(3, (uint32_t)count * 292, tbl);

    std::vector<FxElemTags> tags(count);
    for (int i = 0; i < count; ++i)
        tags[i] = tFxElemDefFixed(r, z, Prelink::Loc{tbl.blk, tbl.off + (uint32_t)i * SZ_FXELEMDEF});
    for (int i = 0; i < count; ++i)
        tFxElemDefRefs(r, z, Prelink::Loc{tbl.blk, tbl.off + (uint32_t)i * SZ_FXELEMDEF}, tags[i],
                       tblX86 + (uint32_t)i * 292);

    z.putPtr(obj, 32, tbl);
}

// ---- weapons ----------------------------------------------------------------
// WeaponDef is 2056 bytes of mostly flat scalars with 130-odd pointers sprinkled
// through it, and WeaponVariantDef and flameTable are the same shape. Counting
// those offsets by hand is not worth the risk, so the three tables below are
// generated from the headers (tools/ffconv/layout.py) and checked against the
// x86 `sizeof=` annotation before being pasted in.
//
// Each table is a set of {x86 offset, LP64 offset, byte count} runs of
// pointer-free data plus a list of {x86 offset, LP64 offset} pointer slots.
// tReadFixed walks the x86 block once: it copies the runs, collects each
// pointer slot as its raw stream tag, and skips whatever padding is left over.

struct FixedBlock {
    std::unordered_map<uint32_t, uint32_t> tag;   // LP64 offset -> stream tag
    std::unordered_map<uint32_t, uint32_t> x86;   // LP64 offset -> x86 offset
    uint32_t base = 0;                            // struct's own block-4 offset

    // Absolute x86 offset of a pointer slot, or 0 when the struct does not
    // live in block 4 and so cannot be aliased.
    uint32_t slot(uint32_t lp64off) const {
        if (!base) return 0;
        auto it = x86.find(lp64off);
        return it == x86.end() ? 0 : base + it->second;
    }

    // Looking up an offset that is not a pointer slot means the caller mistyped
    // one; that must not silently turn into a null.
    uint32_t at(uint32_t lp64off) const {
        auto it = tag.find(lp64off);
        if (it == tag.end()) {
            fprintf(stderr, "internal: no pointer slot at LP64 offset %u\n", lp64off);
            exit(30);
        }
        return it->second;
    }
};

static FixedBlock tReadFixed(Reader &r, Prelink &z, Prelink::Loc obj,
                             uint32_t x86size,
                             const uint32_t (*flat)[3], size_t nflat,
                             const uint32_t (*ptrs)[2], size_t nptrs,
                             uint32_t x86base = 0) {
    FixedBlock fb;
    fb.base = x86base;
    uint32_t cur = 0;
    size_t fi = 0, pi = 0;
    while (fi < nflat || pi < nptrs) {
        const bool takeFlat = pi == nptrs ||
                              (fi < nflat && flat[fi][0] < ptrs[pi][0]);
        const uint32_t at = takeFlat ? flat[fi][0] : ptrs[pi][0];
        if (at > cur) { r.skip(at - cur); cur = at; }   // x86 padding
        if (takeFlat) {
            r.bytes(z.at(obj) + flat[fi][1], flat[fi][2]);
            cur += flat[fi][2];
            ++fi;
        } else {
            fb.tag[ptrs[pi][1]] = r.u32();
            fb.x86[ptrs[pi][1]] = ptrs[pi][0];
            cur += 4;
            ++pi;
        }
    }
    if (x86size > cur) r.skip(x86size - cur);           // trailing padding
    return fb;
}

// generated from the header: x86 2056 -> LP64 2664
enum { SZ_WEAPONDEF = 2664, X86SZ_WEAPONDEF = 2056 };
static const uint32_t kWeaponDefFlat[][3] = {
    {   24,    48,    36},   // playerAnimType .. itemIndex
    {   64,    96,    34},   // iJamFireTime .. fuelTankWeapon
    {  100,   132,    16},   // iTankLifeTime .. stance
    {  388,   696,    12},   // standMountedIndex .. proneMountedIndex
    {  424,   760,   356},   // iReticleCenterSize .. fProneRotMinSpeed
    {  804,  1168,     4},   // hudIconRatio
    {  812,  1184,     4},   // indicatorIconRatio
    {  820,  1200,    24},   // ammoCounterIconRatio .. shotCount
    {  848,  1232,    10},   // iSharedAmmoCapIndex .. ammoCountClipRelative
    {  860,  1244,    26},   // damage .. explosionTag
    {  888,  1272,    28},   // iFireDelay .. spinRate
    {  940,  1352,   211},   // iFireTime .. avoidDropCleanup
    { 1152,  1564,    12},   // stackFire .. stackFireAccuracyDecay
    { 1168,  1584,     9},   // autoAimRange .. mountableWeapon
    { 1180,  1596,     9},   // aimPadding .. crosshairColorChange
    { 1192,  1608,   174},   // moveSpeedScale .. bReloadWhileAds
    { 1368,  1784,    31},   // adsViewErrorMin .. bUseOnlyAltWeaoponHideTagsInAltMode
    { 1404,  1824,    16},   // killIconRatio .. iReloadStartAdd
    { 1428,  1856,    22},   // dualWieldWeaponIndex .. bShowIndicator
    { 1452,  1880,    60},   // isRollingGrenade .. projectileCurvature
    { 1516,  1952,     4},   // projExplosion
    { 1524,  1968,     1},   // projExplosionEffectForceNormalUp
    { 1532,  1984,     1},   // projExplosionEffect2ForceNormalUp
    { 1540,  2000,     1},   // projExplosionEffect3ForceNormalUp
    { 1548,  2016,     1},   // projExplosionEffect4ForceNormalUp
    { 1556,  2032,     1},   // projExplosionEffect5ForceNormalUp
    { 1580,  2080,     2},   // bProjImpactExplode .. bBulletImpactExplode
    { 1584,  2084,    17},   // stickiness .. freezeMovementWhenFiring
    { 1604,  2104,    11},   // lowAmmoWarningThreshold .. isAcousticSensor
    { 1628,  2144,    24},   // vProjectileColor .. projIgnitionDelay
    { 1660,  2184,   152},   // fAdsAimPitch .. maxDist
    { 1836,  2384,    88},   // accuracyGraphKnotCount .. fPlayerPositionDist
    { 1932,  2488,    16},   // iUseHintStringIndex .. vertViewJitter
    { 1952,  2512,    28},   // minDamage .. destabilizeDistance
    { 1996,  2576,    28},   // adsDofStart .. scanPauseTime
    { 2048,  2656,     1},   // doGibbing
    { 2052,  2660,     4},   // maxGibDistance
};
static const uint32_t kWeaponDefPtrs[][2] = {
    {    0,     0},   // szOverlayName
    {    4,     8},   // gunXModel
    {    8,    16},   // handXModel
    {   12,    24},   // szModeName
    {   16,    32},   // notetrackSoundMapKeys
    {   20,    40},   // notetrackSoundMapValues
    {   60,    88},   // parentWeaponName
    {  116,   152},   // viewFlashEffect
    {  120,   160},   // worldFlashEffect
    {  124,   168},   // pickupSound
    {  128,   176},   // pickupSoundPlayer
    {  132,   184},   // ammoPickupSound
    {  136,   192},   // ammoPickupSoundPlayer
    {  140,   200},   // projectileSound
    {  144,   208},   // pullbackSound
    {  148,   216},   // pullbackSoundPlayer
    {  152,   224},   // fireSound
    {  156,   232},   // fireSoundPlayer
    {  160,   240},   // fireLoopSound
    {  164,   248},   // fireLoopSoundPlayer
    {  168,   256},   // fireLoopEndSound
    {  172,   264},   // fireLoopEndSoundPlayer
    {  176,   272},   // fireStopSound
    {  180,   280},   // fireStopSoundPlayer
    {  184,   288},   // fireLastSound
    {  188,   296},   // fireLastSoundPlayer
    {  192,   304},   // emptyFireSound
    {  196,   312},   // emptyFireSoundPlayer
    {  200,   320},   // crackSound
    {  204,   328},   // whizbySound
    {  208,   336},   // meleeSwipeSound
    {  212,   344},   // meleeSwipeSoundPlayer
    {  216,   352},   // meleeHitSound
    {  220,   360},   // meleeMissSound
    {  224,   368},   // rechamberSound
    {  228,   376},   // rechamberSoundPlayer
    {  232,   384},   // reloadSound
    {  236,   392},   // reloadSoundPlayer
    {  240,   400},   // reloadEmptySound
    {  244,   408},   // reloadEmptySoundPlayer
    {  248,   416},   // reloadStartSound
    {  252,   424},   // reloadStartSoundPlayer
    {  256,   432},   // reloadEndSound
    {  260,   440},   // reloadEndSoundPlayer
    {  264,   448},   // rotateLoopSound
    {  268,   456},   // rotateLoopSoundPlayer
    {  272,   464},   // deploySound
    {  276,   472},   // deploySoundPlayer
    {  280,   480},   // finishDeploySound
    {  284,   488},   // finishDeploySoundPlayer
    {  288,   496},   // breakdownSound
    {  292,   504},   // breakdownSoundPlayer
    {  296,   512},   // finishBreakdownSound
    {  300,   520},   // finishBreakdownSoundPlayer
    {  304,   528},   // detonateSound
    {  308,   536},   // detonateSoundPlayer
    {  312,   544},   // nightVisionWearSound
    {  316,   552},   // nightVisionWearSoundPlayer
    {  320,   560},   // nightVisionRemoveSound
    {  324,   568},   // nightVisionRemoveSoundPlayer
    {  328,   576},   // altSwitchSound
    {  332,   584},   // altSwitchSoundPlayer
    {  336,   592},   // raiseSound
    {  340,   600},   // raiseSoundPlayer
    {  344,   608},   // firstRaiseSound
    {  348,   616},   // firstRaiseSoundPlayer
    {  352,   624},   // putawaySound
    {  356,   632},   // putawaySoundPlayer
    {  360,   640},   // overheatSound
    {  364,   648},   // overheatSoundPlayer
    {  368,   656},   // adsZoomSound
    {  372,   664},   // bounceSound
    {  376,   672},   // standMountedWeapdef
    {  380,   680},   // crouchMountedWeapdef
    {  384,   688},   // proneMountedWeapdef
    {  400,   712},   // viewShellEjectEffect
    {  404,   720},   // worldShellEjectEffect
    {  408,   728},   // viewLastShotEjectEffect
    {  412,   736},   // worldLastShotEjectEffect
    {  416,   744},   // reticleCenter
    {  420,   752},   // reticleSide
    {  780,  1120},   // worldModel
    {  784,  1128},   // worldClipModel
    {  788,  1136},   // rocketModel
    {  792,  1144},   // mountedModel
    {  796,  1152},   // additionalMeleeModel
    {  800,  1160},   // hudIcon
    {  808,  1176},   // indicatorIcon
    {  816,  1192},   // ammoCounterIcon
    {  844,  1224},   // szSharedAmmoCapName
    {  916,  1304},   // spinLoopSound
    {  920,  1312},   // spinLoopSoundPlayer
    {  924,  1320},   // startSpinSound
    {  928,  1328},   // startSpinSoundPlayer
    {  932,  1336},   // stopSpinSound
    {  936,  1344},   // stopSpinSoundPlayer
    { 1164,  1576},   // stackSound
    { 1400,  1816},   // killIcon
    { 1420,  1840},   // szSpawnedGrenadeWeaponName
    { 1424,  1848},   // szDualWieldWeaponName
    { 1512,  1944},   // projectileModel
    { 1520,  1960},   // projExplosionEffect
    { 1528,  1976},   // projExplosionEffect2
    { 1536,  1992},   // projExplosionEffect3
    { 1544,  2008},   // projExplosionEffect4
    { 1552,  2024},   // projExplosionEffect5
    { 1560,  2040},   // projDudEffect
    { 1564,  2048},   // projExplosionSound
    { 1568,  2056},   // projDudSound
    { 1572,  2064},   // mortarShellSound
    { 1576,  2072},   // tankShellSound
    { 1616,  2120},   // parallelBounce
    { 1620,  2128},   // perpendicularBounce
    { 1624,  2136},   // projTrailEffect
    { 1652,  2168},   // projIgnitionEffect
    { 1656,  2176},   // projIgnitionSound
    { 1812,  2336},   // accuracyGraphName[0]
    { 1816,  2344},   // accuracyGraphName[1]
    { 1820,  2352},   // accuracyGraphKnots[0]
    { 1824,  2360},   // accuracyGraphKnots[1]
    { 1828,  2368},   // originalAccuracyGraphKnots[0]
    { 1832,  2376},   // originalAccuracyGraphKnots[1]
    { 1924,  2472},   // szUseHintString
    { 1928,  2480},   // dropHintString
    { 1948,  2504},   // szScript
    { 1980,  2544},   // locationDamageMultipliers
    { 1984,  2552},   // fireRumble
    { 1988,  2560},   // meleeImpactRumble
    { 1992,  2568},   // reloadRumble
    { 2024,  2608},   // flameTableFirstPerson
    { 2028,  2616},   // flameTableThirdPerson
    { 2032,  2624},   // flameTableFirstPersonPtr
    { 2036,  2632},   // flameTableThirdPersonPtr
    { 2040,  2640},   // tagFx_preparationEffect
    { 2044,  2648},   // tagFlash_preparationEffect
};

// generated from the header: x86 228 -> LP64 288
enum { SZ_WEAPONVARIANTDEF = 288, X86SZ_WEAPONVARIANTDEF = 228 };
static const uint32_t kWeaponVariantDefFlat[][3] = {
    {    4,     8,     4},   // iVariantCount
    {   28,    56,    36},   // altWeaponIndex .. iAltRaiseTime
    {   68,   104,     4},   // iAmmoIndex
    {   76,   120,    61},   // iClipIndex .. bRapidFire
    {  152,   208,    76},   // dpadIconRatio .. ikLeftHandUiViewerRotation
};
static const uint32_t kWeaponVariantDefPtrs[][2] = {
    {    0,     0},   // szInternalName
    {    8,    16},   // weapDef
    {   12,    24},   // szDisplayName
    {   16,    32},   // szXAnims
    {   20,    40},   // szAltWeaponName
    {   24,    48},   // hideTags
    {   64,    96},   // szAmmoName
    {   72,   112},   // szClipName
    {  140,   184},   // overlayMaterial
    {  144,   192},   // overlayMaterialLowRes
    {  148,   200},   // dpadIcon
};

// generated from the header: x86 476 -> LP64 528
enum { SZ_FLAMETABLE = 528, X86SZ_FLAMETABLE = 476 };
static const uint32_t kFlameTableFlat[][3] = {
    {    0,     0,   424},   // flameVar_streamChunkGravityStart .. flameVar_collisionVolumeScale
};
static const uint32_t kFlameTablePtrs[][2] = {
    {  424,   424},   // name
    {  428,   432},   // fire
    {  432,   440},   // smoke
    {  436,   448},   // heat
    {  440,   456},   // drips
    {  444,   464},   // streamFuel
    {  448,   472},   // streamFuel2
    {  452,   480},   // streamFlame
    {  456,   488},   // streamFlame2
    {  460,   496},   // flameOffLoopSound
    {  464,   504},   // flameIgniteSound
    {  468,   512},   // flameOnLoopSound
    {  472,   520},   // flameCooldownSound
};
// Load_XModelPtr (db_load.cpp:3300) and Load_FxEffectDefHandle (3814) share the
// asset-handle shape: -1 and -2 mean the asset follows inline, anything else
// non-zero is an offset alias.
static void tXModelHandle(Reader &r, Prelink &z, Prelink::Loc obj,
                          uint32_t field, uint32_t tag, uint32_t x86slot) {
    if (tag == TAG_INLINE || tag == TAG_ALIAS) {
        Prelink::Loc m = z.alloc(OUT, SZ_XMODEL, 8);
        tempReserve(252);      // Load_XModelPtr -> block 0
        if (tag == TAG_ALIAS) g_aliasMap[insertPointerSlot()] = m;
        noteAliasSlot(x86slot, m);
        tXModel(r, z, m);
        z.putPtr(obj, field, m);
    } else if (tag != TAG_NULL) {
        noteAliasSlot(x86slot, putAssetHandleRef(z, obj, field, tag));
    } else {
        z.putPtr(obj, field, Prelink::none());
    }
}

static void tFxEffectDefHandle(Reader &r, Prelink &z, Prelink::Loc obj,
                               uint32_t field, uint32_t tag, uint32_t x86slot = 0) {
    if (tag == TAG_INLINE || tag == TAG_ALIAS) {
        Prelink::Loc f = z.alloc(OUT, SZ_FXEFFECTDEF, 8);
        tempReserve(60);       // Load_FxEffectDefHandle -> block 0
        if (tag == TAG_ALIAS) g_aliasMap[insertPointerSlot()] = f;
        noteAliasSlot(x86slot, f);
        tFxEffectDef(r, z, f);
        z.putPtr(obj, field, f);
    } else if (tag != TAG_NULL) {
        noteAliasSlot(x86slot, putAssetHandleRef(z, obj, field, tag));
    } else {
        z.putPtr(obj, field, Prelink::none());
    }
}

// An array of asset handles: the pointer run is read whole before the elements,
// exactly as Load_XModelPtrArray and Load_XStringArray do.
static void tXModelHandleArray(Reader &r, Prelink &z, Prelink::Loc obj,
                               uint32_t field, uint32_t tag, uint32_t count) {
    if (tag == TAG_NULL) { z.putPtr(obj, field, Prelink::none()); return; }
    if (tag != TAG_INLINE) { putStructOffsetRef(z, obj, field, tag); return; }

    Prelink::Loc tbl = z.alloc(OUT, (size_t)count * 8, 8);
    b4Reserve(3, count * 4, tbl);
    std::vector<uint32_t> tags(count);
    for (uint32_t i = 0; i < count; ++i) tags[i] = r.u32();
    for (uint32_t i = 0; i < count; ++i)
        tXModelHandle(r, z, Prelink::Loc{tbl.blk, tbl.off + i * 8}, 0, tags[i]);
    z.putPtr(obj, field, tbl);
}

static void tXStringArrayPtr(Reader &r, Prelink &z, Prelink::Loc obj,
                             uint32_t field, uint32_t tag, uint32_t count) {
    if (tag == TAG_NULL) { z.putPtr(obj, field, Prelink::none()); return; }
    if (tag != TAG_INLINE) { putStructOffsetRef(z, obj, field, tag); return; }

    Prelink::Loc tbl = z.alloc(OUT, (size_t)count * 8, 8);
    b4Reserve(3, count * 4, tbl);
    std::vector<uint32_t> tags(count);
    for (uint32_t i = 0; i < count; ++i) tags[i] = r.u32();
    for (uint32_t i = 0; i < count; ++i)
        putXStringFromTag(r, z, tbl, i * 8, tags[i]);
    z.putPtr(obj, field, tbl);
}

// flameTable: 476 -> 528. A block of floats followed by eight materials and
// four sound names.
static void tFlameTable(Reader &r, Prelink &z, Prelink::Loc obj, uint32_t field,
                        uint32_t tag) {
    if (tag == TAG_NULL) { z.putPtr(obj, field, Prelink::none()); return; }
    if (tag != TAG_INLINE) { putStructOffsetRef(z, obj, field, tag); return; }

    Prelink::Loc ft = z.alloc(OUT, SZ_FLAMETABLE, 8);
    const uint32_t ftX86 = b4Reserve(3, X86SZ_FLAMETABLE, ft);
    FixedBlock fb = tReadFixed(r, z, ft, X86SZ_FLAMETABLE,
                               kFlameTableFlat, sizeof kFlameTableFlat / 12,
                               kFlameTablePtrs, sizeof kFlameTablePtrs / 8, ftX86);
    z.putPtr(obj, field, ft);

    putXStringFromTag(r, z, ft, 424, fb.at(424));            // name
    for (uint32_t o = 432; o <= 488; o += 8)                 // fire .. streamFlame2
        tMaterialHandle(r, z, ft, o, fb.at(o), fb.slot(o));
    for (uint32_t o = 496; o <= 520; o += 8)                 // the four sounds
        putXStringFromTag(r, z, ft, o, fb.at(o));
}

// WeaponDef: 2056 -> 2664. Load_WeaponDef walks its references in an order that
// is mostly, but not entirely, the field order -- indicatorIcon comes after
// killIcon, and the two accuracy graphs are interleaved per index.
static void tWeaponDef(Reader &r, Prelink &z, Prelink::Loc obj, uint32_t field,
                       uint32_t tag) {
    if (tag == TAG_NULL) { z.putPtr(obj, field, Prelink::none()); return; }
    if (tag != TAG_INLINE) { putStructOffsetRef(z, obj, field, tag); return; }

    Prelink::Loc w = z.alloc(OUT, SZ_WEAPONDEF, 8);
    const uint32_t wX86 = b4Reserve(3, X86SZ_WEAPONDEF, w);
    FixedBlock fb = tReadFixed(r, z, w, X86SZ_WEAPONDEF,
                               kWeaponDefFlat, sizeof kWeaponDefFlat / 12,
                               kWeaponDefPtrs, sizeof kWeaponDefPtrs / 8, wX86);
    z.putPtr(obj, field, w);

    putXStringFromTag(r, z, w, 0, fb.at(0));                 // szOverlayName
    tXModelHandleArray(r, z, w, 8, fb.at(8), 16);            // gunXModel[16]
    tXModelHandle(r, z, w, 16, fb.at(16), fb.slot(16));                   // handXModel
    putXStringFromTag(r, z, w, 24, fb.at(24));               // szModeName

    // notetrackSoundMapKeys / Values: 20 script strings, AllocLoad_XBlendInfo.
    tOffsetOrArray(r, z, w, 32, fb.at(32), 20, 2, 1);
    tOffsetOrArray(r, z, w, 40, fb.at(40), 20, 2, 1);

    putXStringFromTag(r, z, w, 88, fb.at(88));               // parentWeaponName
    tFxEffectDefHandle(r, z, w, 152, fb.at(152), fb.slot(152));            // viewFlashEffect
    tFxEffectDefHandle(r, z, w, 160, fb.at(160), fb.slot(160));            // worldFlashEffect

    // pickupSound .. adsZoomSound: 62 plain strings in field order.
    for (uint32_t o = 168; o <= 656; o += 8)
        putXStringFromTag(r, z, w, o, fb.at(o));

    tXStringArrayPtr(r, z, w, 664, fb.at(664), 31);          // bounceSound[31]
    for (uint32_t o = 672; o <= 688; o += 8)                 // the mounted weapdefs
        putXStringFromTag(r, z, w, o, fb.at(o));

    for (uint32_t o = 712; o <= 736; o += 8)                 // the four eject effects
        tFxEffectDefHandle(r, z, w, o, fb.at(o), fb.slot(o));
    tMaterialHandle(r, z, w, 744, fb.at(744), fb.slot(744));               // reticleCenter
    tMaterialHandle(r, z, w, 752, fb.at(752), fb.slot(752));               // reticleSide

    tXModelHandleArray(r, z, w, 1120, fb.at(1120), 16);      // worldModel[16]
    for (uint32_t o = 1128; o <= 1152; o += 8)               // worldClip .. addMelee
        tXModelHandle(r, z, w, o, fb.at(o), fb.slot(o));

    tMaterialHandle(r, z, w, 1160, fb.at(1160), fb.slot(1160));             // hudIcon
    tMaterialHandle(r, z, w, 1192, fb.at(1192), fb.slot(1192));             // ammoCounterIcon
    putXStringFromTag(r, z, w, 1224, fb.at(1224));           // szSharedAmmoCapName
    // explosionTag is a script string inside the fixed block; it loads nothing.

    for (uint32_t o = 1304; o <= 1344; o += 8)               // the six spin sounds
        putXStringFromTag(r, z, w, o, fb.at(o));
    putXStringFromTag(r, z, w, 1576, fb.at(1576));           // stackSound

    tMaterialHandle(r, z, w, 1816, fb.at(1816), fb.slot(1816));             // killIcon
    tMaterialHandle(r, z, w, 1176, fb.at(1176), fb.slot(1176));             // indicatorIcon
    putXStringFromTag(r, z, w, 1840, fb.at(1840));           // szSpawnedGrenadeWeaponName
    putXStringFromTag(r, z, w, 1848, fb.at(1848));           // szDualWieldWeaponName

    tXModelHandle(r, z, w, 1944, fb.at(1944), fb.slot(1944));               // projectileModel
    for (uint32_t o = 1960; o <= 2040; o += 16)              // projExplosionEffect 1..5, dud
        tFxEffectDefHandle(r, z, w, o, fb.at(o), fb.slot(o));
    for (uint32_t o = 2048; o <= 2072; o += 8)               // the four projectile sounds
        putXStringFromTag(r, z, w, o, fb.at(o));

    tOffsetOrArray(r, z, w, 2120, fb.at(2120), 31, 4, 3);    // parallelBounce[31]
    tOffsetOrArray(r, z, w, 2128, fb.at(2128), 31, 4, 3);    // perpendicularBounce[31]
    tFxEffectDefHandle(r, z, w, 2136, fb.at(2136), fb.slot(2136));          // projTrailEffect
    tFxEffectDefHandle(r, z, w, 2168, fb.at(2168), fb.slot(2168));          // projIgnitionEffect
    putXStringFromTag(r, z, w, 2176, fb.at(2176));           // projIgnitionSound

    // Both knot arrays of a graph are sized by accuracyGraphKnotCount[i]; the
    // originalAccuracyGraphKnotCount fields are not used by the loader.
    for (uint32_t i = 0; i < 2; ++i) {
        int32_t knots;
        memcpy(&knots, z.at(w) + 2384 + i * 4, 4);
        const uint32_t n = knots > 0 ? (uint32_t)knots : 0;
        putXStringFromTag(r, z, w, 2336 + i * 8, fb.at(2336 + i * 8));
        tOffsetOrArray(r, z, w, 2352 + i * 8, fb.at(2352 + i * 8), n * 2, 4, 3);
        tOffsetOrArray(r, z, w, 2368 + i * 8, fb.at(2368 + i * 8), n * 2, 4, 3);
    }

    putXStringFromTag(r, z, w, 2472, fb.at(2472));           // szUseHintString
    putXStringFromTag(r, z, w, 2480, fb.at(2480));           // dropHintString
    putXStringFromTag(r, z, w, 2504, fb.at(2504));           // szScript
    tOffsetOrArray(r, z, w, 2544, fb.at(2544), 19, 4, 3);    // locationDamageMultipliers
    for (uint32_t o = 2552; o <= 2568; o += 8)               // the three rumbles
        putXStringFromTag(r, z, w, o, fb.at(o));
    putXStringFromTag(r, z, w, 2608, fb.at(2608));           // flameTableFirstPerson
    putXStringFromTag(r, z, w, 2616, fb.at(2616));           // flameTableThirdPerson
    tFlameTable(r, z, w, 2624, fb.at(2624));
    tFlameTable(r, z, w, 2632, fb.at(2632));
    tFxEffectDefHandle(r, z, w, 2640, fb.at(2640), fb.slot(2640));          // tagFx_preparationEffect
    tFxEffectDefHandle(r, z, w, 2648, fb.at(2648), fb.slot(2648));          // tagFlash_preparationEffect
}

// WeaponVariantDef: 228 -> 288, the asset type 24 payload.
static void tWeaponVariantDef(Reader &r, Prelink &z, Prelink::Loc obj) {
    FixedBlock fb = tReadFixed(r, z, obj, X86SZ_WEAPONVARIANTDEF,
                               kWeaponVariantDefFlat, sizeof kWeaponVariantDefFlat / 12,
                               kWeaponVariantDefPtrs, sizeof kWeaponVariantDefPtrs / 8);

    putXStringFromTag(r, z, obj, 0, fb.at(0));               // szInternalName
    tWeaponDef(r, z, obj, 16, fb.at(16));                    // weapDef
    putXStringFromTag(r, z, obj, 24, fb.at(24));             // szDisplayName
    putXStringFromTag(r, z, obj, 40, fb.at(40));             // szAltWeaponName
    tXStringArrayPtr(r, z, obj, 32, fb.at(32), 66);          // szXAnims[66]
    tOffsetOrArray(r, z, obj, 48, fb.at(48), 32, 2, 1);      // hideTags[32]
    putXStringFromTag(r, z, obj, 96, fb.at(96));             // szAmmoName
    putXStringFromTag(r, z, obj, 112, fb.at(112));           // szClipName
    tMaterialHandle(r, z, obj, 184, fb.at(184));             // overlayMaterial
    tMaterialHandle(r, z, obj, 192, fb.at(192));             // overlayMaterialLowRes
    tMaterialHandle(r, z, obj, 200, fb.at(200));             // dpadIcon
}

// ---- impact fx --------------------------------------------------------------
// FxImpactTable is a name and a fixed table of 21 FxImpactEntry, each of which
// is nothing but 31 + 4 FxEffectDef handles: 140 -> 280.
enum { AT_IMPACTFX = 0x1D, SZ_FXIMPACTTABLE = 16, SZ_FXIMPACTENTRY = 280,
       FXIMPACT_ROWS = 21, FXIMPACT_HANDLES = 35 };

static void tFxImpactTable(Reader &r, Prelink &z, Prelink::Loc obj) {
    uint32_t nameTag  = r.u32();
    uint32_t tableTag = r.u32();

    putXStringFromTag(r, z, obj, 0, nameTag);
    if (tableTag == TAG_NULL) { z.putPtr(obj, 8, Prelink::none()); return; }

    Prelink::Loc tbl = z.alloc(OUT, (size_t)FXIMPACT_ROWS * SZ_FXIMPACTENTRY, 8);
    b4Reserve(3, FXIMPACT_ROWS * 140, tbl);

    // Load_FxImpactEntryArray reads all 21 entries as one run, then walks them.
    std::vector<uint32_t> tags((size_t)FXIMPACT_ROWS * FXIMPACT_HANDLES);
    for (size_t i = 0; i < tags.size(); ++i) tags[i] = r.u32();

    for (int e = 0; e < FXIMPACT_ROWS; ++e) {
        Prelink::Loc entry{tbl.blk, tbl.off + (uint32_t)e * SZ_FXIMPACTENTRY};
        for (int h = 0; h < FXIMPACT_HANDLES; ++h)   // nonflesh[31] then flesh[4]
            tFxEffectDefHandle(r, z, entry, (uint32_t)h * 8,
                               tags[(size_t)e * FXIMPACT_HANDLES + h]);
    }
    z.putPtr(obj, 8, tbl);
}

// ---- sound driver globals ---------------------------------------------------
// SndDriverGlobals: 52 -> 104, a name plus six {count, array} pairs. Every
// element struct (snd_group, snd_curve, snd_pan, snd_snapshot_group,
// snd_context, snd_master) is pointer-free and keeps its x86 size, so the
// arrays copy across byte for byte.
enum { AT_SNDDRIVERGLOBALS = 0x1B, SZ_SNDDRIVERGLOBALS = 104 };

static void tSndDriverGlobals(Reader &r, Prelink &z, Prelink::Loc obj) {
    struct { uint32_t count, tag, field, elemSize; } arr[6] = {
        {0, 0, 16, 80},    // groups         snd_group
        {0, 0, 32, 100},   // curves         snd_curve
        {0, 0, 48, 60},    // pans           snd_pan
        {0, 0, 64, 32},    // snapshotGroups snd_snapshot_group
        {0, 0, 80, 40},    // contexts       snd_context
        {0, 0, 96, 176},   // masters        snd_master
    };

    uint32_t nameTag = r.u32();
    for (int i = 0; i < 6; ++i) { arr[i].count = r.u32(); arr[i].tag = r.u32(); }

    uint8_t *o = z.at(obj);
    for (int i = 0; i < 6; ++i)
        memcpy(o + arr[i].field - 8, &arr[i].count, 4);   // count precedes its array

    putXStringFromTag(r, z, obj, 0, nameTag);
    for (int i = 0; i < 6; ++i) {
        if (arr[i].tag == TAG_NULL) { z.putPtr(obj, arr[i].field, Prelink::none()); continue; }
        tFlatArray(r, z, obj, arr[i].field, arr[i].count * arr[i].elemSize, 3);
    }
}

// ---- font -------------------------------------------------------------------
// Font_s: 24 -> 40. Glyph is 24 pointer-free bytes and does not widen, so the
// glyph table is a straight copy; the loader tests it against -1.
enum { AT_FONT = 0x14, SZ_FONT = 40 };

static void tFont(Reader &r, Prelink &z, Prelink::Loc obj) {
    uint32_t nameTag    = r.u32();
    int32_t  pixelH     = r.i32();
    int32_t  glyphCount = r.i32();
    uint32_t matTag     = r.u32();
    uint32_t glowTag    = r.u32();
    uint32_t glyphsTag  = r.u32();

    uint8_t *o = z.at(obj);
    memcpy(o + 8,  &pixelH, 4);
    memcpy(o + 12, &glyphCount, 4);

    putXStringFromTag(r, z, obj, 0, nameTag);
    tMaterialHandle(r, z, obj, 16, matTag);
    tMaterialHandle(r, z, obj, 24, glowTag);

    if (glyphsTag == TAG_NULL)        z.putPtr(obj, 32, Prelink::none());
    else if (glyphsTag != TAG_INLINE) putStructOffsetRef(z, obj, 32, glyphsTag);
    else tFlatArray(r, z, obj, 32, 24u * (uint32_t)(glyphCount > 0 ? glyphCount : 0), 3);
}

// ---- ddl --------------------------------------------------------------------
// ddlRoot_t -> ddlDef_t (a linked list) -> struct and enum tables.
// Every array loader here reads its whole fixed run before walking it, and none
// of them tests the pointer against -1: any non-zero value means inline.
enum { AT_DDL = 0x28, SZ_DDLROOT = 16, SZ_DDLDEF = 48, SZ_DDLSTRUCT = 24,
       SZ_DDLENUM = 24, SZ_DDLMEMBER = 56 };

static void tDdlMemberArray(Reader &r, Prelink &z, Prelink::Loc sd,
                            uint32_t field, int32_t count) {
    if (count <= 0) { z.putPtr(sd, field, Prelink::none()); return; }
    Prelink::Loc tbl = z.alloc(OUT, (size_t)count * SZ_DDLMEMBER, 8);
    b4Reserve(3, (uint32_t)count * 48, tbl);

    std::vector<uint32_t> names(count);
    for (int i = 0; i < count; ++i) {
        names[i] = r.u32();
        uint8_t rest[44];                  // size .. permission
        r.bytes(rest, 44);
        memcpy(z.at(Prelink::Loc{tbl.blk, tbl.off + (uint32_t)i * SZ_DDLMEMBER}) + 8,
               rest, 44);
    }
    for (int i = 0; i < count; ++i)
        putXStringFromTag(r, z, Prelink::Loc{tbl.blk, tbl.off + (uint32_t)i * SZ_DDLMEMBER},
                          0, names[i]);
    z.putPtr(sd, field, tbl);
}

static void tDdlStructArray(Reader &r, Prelink &z, Prelink::Loc dd,
                            uint32_t field, int32_t count) {
    if (count <= 0) { z.putPtr(dd, field, Prelink::none()); return; }
    Prelink::Loc tbl = z.alloc(OUT, (size_t)count * SZ_DDLSTRUCT, 8);
    b4Reserve(3, (uint32_t)count * 16, tbl);

    std::vector<uint32_t> names(count), memberTags(count);
    std::vector<int32_t>  memberCounts(count);
    for (int i = 0; i < count; ++i) {
        names[i] = r.u32();
        int32_t size = r.i32();
        memberCounts[i] = r.i32();
        memberTags[i] = r.u32();
        uint8_t *d = z.at(Prelink::Loc{tbl.blk, tbl.off + (uint32_t)i * SZ_DDLSTRUCT});
        memcpy(d + 8, &size, 4);
        memcpy(d + 12, &memberCounts[i], 4);
    }
    for (int i = 0; i < count; ++i) {
        Prelink::Loc sd{tbl.blk, tbl.off + (uint32_t)i * SZ_DDLSTRUCT};
        putXStringFromTag(r, z, sd, 0, names[i]);
        if (memberTags[i] != TAG_NULL) tDdlMemberArray(r, z, sd, 16, memberCounts[i]);
        else                           z.putPtr(sd, 16, Prelink::none());
    }
    z.putPtr(dd, field, tbl);
}

static void tDdlEnumArray(Reader &r, Prelink &z, Prelink::Loc dd,
                          uint32_t field, int32_t count) {
    if (count <= 0) { z.putPtr(dd, field, Prelink::none()); return; }
    Prelink::Loc tbl = z.alloc(OUT, (size_t)count * SZ_DDLENUM, 8);
    b4Reserve(3, (uint32_t)count * 12, tbl);

    std::vector<uint32_t> names(count), memberTags(count);
    std::vector<int32_t>  memberCounts(count);
    for (int i = 0; i < count; ++i) {
        names[i] = r.u32();
        memberCounts[i] = r.i32();
        memberTags[i] = r.u32();
        memcpy(z.at(Prelink::Loc{tbl.blk, tbl.off + (uint32_t)i * SZ_DDLENUM}) + 8,
               &memberCounts[i], 4);
    }
    for (int i = 0; i < count; ++i) {
        Prelink::Loc ed{tbl.blk, tbl.off + (uint32_t)i * SZ_DDLENUM};
        putXStringFromTag(r, z, ed, 0, names[i]);
        if (memberTags[i] == TAG_NULL || memberCounts[i] <= 0) {
            z.putPtr(ed, 16, Prelink::none());
            continue;
        }
        Prelink::Loc strs = z.alloc(OUT, (size_t)memberCounts[i] * 8, 8);
        b4Reserve(3, (uint32_t)memberCounts[i] * 4, strs);
        std::vector<uint32_t> tags(memberCounts[i]);
        for (int k = 0; k < memberCounts[i]; ++k) tags[k] = r.u32();
        for (int k = 0; k < memberCounts[i]; ++k)
            putXStringFromTag(r, z, strs, (uint32_t)k * 8, tags[k]);
        z.putPtr(ed, 16, strs);
    }
    z.putPtr(dd, field, tbl);
}

// ddlDef_t: 28 -> 48, chained through `next`. Load_ddlDefNext re-reads the
// 28-byte block for each link, so the recursion mirrors the loader exactly.
static void tDdlDef(Reader &r, Prelink &z, Prelink::Loc obj, uint32_t field) {
    Prelink::Loc dd = z.alloc(OUT, SZ_DDLDEF, 8);
    b4Reserve(3, 28, dd);

    int32_t  version     = r.i32();
    int32_t  size        = r.i32();
    uint32_t structTag   = r.u32();
    int32_t  structCount = r.i32();
    uint32_t enumTag     = r.u32();
    int32_t  enumCount   = r.i32();
    uint32_t nextTag     = r.u32();

    uint8_t *o = z.at(dd);
    memcpy(o,      &version, 4);
    memcpy(o + 4,  &size, 4);
    memcpy(o + 16, &structCount, 4);
    memcpy(o + 32, &enumCount, 4);

    z.putPtr(obj, field, dd);

    if (structTag != TAG_NULL) tDdlStructArray(r, z, dd, 8, structCount);
    else                       z.putPtr(dd, 8, Prelink::none());
    if (enumTag != TAG_NULL)   tDdlEnumArray(r, z, dd, 24, enumCount);
    else                       z.putPtr(dd, 24, Prelink::none());
    if (nextTag != TAG_NULL)   tDdlDef(r, z, dd, 40);
    else                       z.putPtr(dd, 40, Prelink::none());
}

static void tDdlRoot(Reader &r, Prelink &z, Prelink::Loc obj) {
    uint32_t nameTag = r.u32();
    uint32_t defTag  = r.u32();
    putXStringFromTag(r, z, obj, 0, nameTag);
    if (defTag != TAG_NULL) tDdlDef(r, z, obj, 8);
    else                    z.putPtr(obj, 8, Prelink::none());
}

// ---- sound ------------------------------------------------------------------
// SndBank -> snd_alias_list_t[] -> snd_alias_t[] -> SoundFile -> loaded or
// streamed sound. snd_radverb, snd_snapshot and SndIndexEntry are pointer-free
// and keep their x86 sizes.
//
// The PCM payloads are the one place the game leaves block 4: Load_snd_asset
// wraps them in DB_PushStreamPos(5)/(6) and allocates with
// AllocLoad_snd_align_char == DB_AllocStreamPos(2047). They go into their own
// output block here, and the block-4 cursor is deliberately left alone.
// NOTE: nx_kbz.cpp allocates every block with plain malloc, so the 2048-byte
// alignment the game gives these buffers is not reproduced on device.
enum { AT_SOUND = 9, SNDBLK = 5,
       SZ_SNDBANK = 72, SZ_SNDALIASLIST = 32, SZ_SNDALIAS = 104,
       SZ_SOUNDFILE = 16, SZ_LOADEDSOUND = 80, SZ_SNDASSET = 72,
       SZ_PRIMEDSOUND = 24, SZ_STREAMEDSOUND = 16 };

// A 2048-aligned PCM payload in the sound block. No b4Reserve: the game
// allocates these from blocks 5 and 6, not from block 4.
static void tSndPayload(Reader &r, Prelink &z, Prelink::Loc obj, uint32_t field,
                        uint32_t nbytes) {
    Prelink::Loc b = z.alloc(SNDBLK, nbytes ? nbytes : 1, 2048);
    if (nbytes) r.bytes(z.at(b), nbytes);
    z.putPtr(obj, field, b);
}

// snd_asset sits inline inside LoadedSound; its 56-byte block has already been
// read by the caller, so only seek_table and data follow.
static void tSndAsset(Reader &r, Prelink &z, Prelink::Loc ls, uint32_t base,
                      uint32_t seekTag, uint32_t seekCount,
                      uint32_t dataTag, uint32_t dataSize) {
    if (seekTag != TAG_NULL)
        tSimpleArray(r, z, ls, base + 48, seekTag, seekCount, 4, 3);
    else
        z.putPtr(ls, base + 48, Prelink::none());

    if (dataTag != TAG_NULL) tSndPayload(r, z, ls, base + 64, dataSize);
    else                     z.putPtr(ls, base + 64, Prelink::none());
}

// LoadedSound: 60 -> 80, a name followed by an inline snd_asset (56 -> 72).
static void tLoadedSound(Reader &r, Prelink &z, Prelink::Loc obj, uint32_t field) {
    Prelink::Loc ls = z.alloc(OUT, SZ_LOADEDSOUND, 8);
    b4Reserve(3, 60, ls);

    uint32_t nameTag = r.u32();
    uint8_t  head[44];                     // version .. seek_table_count
    r.bytes(head, 44);
    uint32_t seekTag  = r.u32();
    uint32_t dataSize = r.u32();
    uint32_t dataTag  = r.u32();

    uint8_t *o = z.at(ls);
    memcpy(o + 8,  head, 44);              // snd_asset starts at LP64 offset 8
    memcpy(o + 64, &dataSize, 4);          // 8 + snd_asset.data_size@56

    uint32_t seekCount;
    memcpy(&seekCount, head + 40, 4);

    z.putPtr(obj, field, ls);
    putXStringFromTag(r, z, ls, 0, nameTag);
    tSndAsset(r, z, ls, 8, seekTag, seekCount, dataTag, dataSize);
}

// PrimedSound: 12 -> 24. buffer is another 2048-aligned payload.
static void tPrimedSound(Reader &r, Prelink &z, Prelink::Loc obj, uint32_t field) {
    Prelink::Loc ps = z.alloc(OUT, SZ_PRIMEDSOUND, 8);
    b4Reserve(3, 12, ps);

    uint32_t nameTag = r.u32();
    uint32_t bufTag  = r.u32();
    uint32_t size    = r.u32();

    memcpy(z.at(ps) + 16, &size, 4);
    z.putPtr(obj, field, ps);

    putXStringFromTag(r, z, ps, 0, nameTag);
    if (bufTag != TAG_NULL) tSndPayload(r, z, ps, 8, size);
    else                    z.putPtr(ps, 8, Prelink::none());
}

// StreamedSound: 8 -> 16. primeSnd is one of the pointers that tests -1.
static void tStreamedSound(Reader &r, Prelink &z, Prelink::Loc obj, uint32_t field) {
    Prelink::Loc ss = z.alloc(OUT, SZ_STREAMEDSOUND, 8);
    b4Reserve(3, 8, ss);

    uint32_t fileTag  = r.u32();
    uint32_t primeTag = r.u32();

    z.putPtr(obj, field, ss);
    putXStringFromTag(r, z, ss, 0, fileTag);

    if (primeTag == TAG_NULL)        z.putPtr(ss, 8, Prelink::none());
    else if (primeTag != TAG_INLINE) putStructOffsetRef(z, ss, 8, primeTag);
    else                             tPrimedSound(r, z, ss, 8);
}

// SoundFile: 8 -> 16. The union is a LoadedSound when type == 1 and a
// StreamedSound otherwise (Load_SoundFileRef, db_load.cpp:1424).
static void tSoundFile(Reader &r, Prelink &z, Prelink::Loc obj, uint32_t field) {
    Prelink::Loc sf = z.alloc(OUT, SZ_SOUNDFILE, 8);
    b4Reserve(3, 8, sf);

    uint32_t uTag = r.u32();
    uint8_t  tail[4];                      // type, exists, padding
    r.bytes(tail, 4);
    memcpy(z.at(sf) + 8, tail, 4);

    z.putPtr(obj, field, sf);

    if (uTag == TAG_NULL)        { z.putPtr(sf, 0, Prelink::none()); return; }
    if (uTag != TAG_INLINE)      { putStructOffsetRef(z, sf, 0, uTag); return; }
    if (tail[0] == 1)            tLoadedSound(r, z, sf, 0);
    else                         tStreamedSound(r, z, sf, 0);
}

// snd_alias_t: 84 -> 104. Load_snd_alias_tArray reads the whole 84*count run
// before walking it.
static void tSndAliasArray(Reader &r, Prelink &z, Prelink::Loc list,
                           uint32_t field, int32_t count) {
    if (count <= 0) { z.putPtr(list, field, Prelink::none()); return; }
    Prelink::Loc tbl = z.alloc(OUT, (size_t)count * SZ_SNDALIAS, 8);
    b4Reserve(3, (uint32_t)count * 84, tbl);

    struct AliasTags { uint32_t name, subtitle, secondary, soundFile; };
    std::vector<AliasTags> at(count);
    for (int i = 0; i < count; ++i) {
        Prelink::Loc a{tbl.blk, tbl.off + (uint32_t)i * SZ_SNDALIAS};
        at[i].name = r.u32();
        uint32_t id = r.u32();
        at[i].subtitle  = r.u32();
        at[i].secondary = r.u32();
        at[i].soundFile = r.u32();
        uint8_t rest[64];                  // flags .. snapshotGroup, plus padding
        r.bytes(rest, 64);
        uint8_t *d = z.at(a);
        memcpy(d + 8,  &id, 4);
        memcpy(d + 40, rest, 63);          // the 64th byte is x86 tail padding
    }
    for (int i = 0; i < count; ++i) {
        Prelink::Loc a{tbl.blk, tbl.off + (uint32_t)i * SZ_SNDALIAS};
        putXStringFromTag(r, z, a, 0,  at[i].name);
        putXStringFromTag(r, z, a, 16, at[i].subtitle);
        putXStringFromTag(r, z, a, 24, at[i].secondary);
        if (at[i].soundFile == TAG_NULL)        z.putPtr(a, 32, Prelink::none());
        else if (at[i].soundFile != TAG_INLINE) putStructOffsetRef(z, a, 32, at[i].soundFile);
        else                                    tSoundFile(r, z, a, 32);
    }
    z.putPtr(list, field, tbl);
}

// snd_alias_list_t: 20 -> 32.
static void tSndAliasListArray(Reader &r, Prelink &z, Prelink::Loc bank,
                               uint32_t field, int32_t count) {
    if (count <= 0) { z.putPtr(bank, field, Prelink::none()); return; }
    Prelink::Loc tbl = z.alloc(OUT, (size_t)count * SZ_SNDALIASLIST, 8);
    b4Reserve(3, (uint32_t)count * 20, tbl);

    std::vector<uint32_t> names(count), heads(count);
    std::vector<int32_t>  counts(count);
    for (int i = 0; i < count; ++i) {
        names[i] = r.u32();
        uint32_t id = r.u32();
        heads[i] = r.u32();
        counts[i] = r.i32();
        int32_t sequence = r.i32();
        uint8_t *d = z.at(Prelink::Loc{tbl.blk, tbl.off + (uint32_t)i * SZ_SNDALIASLIST});
        memcpy(d + 8,  &id, 4);
        memcpy(d + 24, &counts[i], 4);
        memcpy(d + 28, &sequence, 4);
    }
    for (int i = 0; i < count; ++i) {
        Prelink::Loc l{tbl.blk, tbl.off + (uint32_t)i * SZ_SNDALIASLIST};
        putXStringFromTag(r, z, l, 0, names[i]);
        if (heads[i] == TAG_NULL)        z.putPtr(l, 16, Prelink::none());
        else if (heads[i] != TAG_INLINE) putStructOffsetRef(z, l, 16, heads[i]);
        else                             tSndAliasArray(r, z, l, 16, counts[i]);
    }
    z.putPtr(bank, field, tbl);
}

// SndBank: 40 -> 72.
static void tSndBank(Reader &r, Prelink &z, Prelink::Loc obj) {
    uint32_t nameTag       = r.u32();
    int32_t  aliasCount    = r.i32();
    uint32_t aliasTag      = r.u32();
    uint32_t aliasIndexTag = r.u32();
    uint32_t packHash      = r.u32();
    uint32_t packLocation  = r.u32();
    int32_t  radverbCount  = r.i32();
    uint32_t radverbTag    = r.u32();
    int32_t  snapshotCount = r.i32();
    uint32_t snapshotTag   = r.u32();

    uint8_t *o = z.at(obj);
    memcpy(o + 8,  &aliasCount, 4);
    memcpy(o + 32, &packHash, 4);
    memcpy(o + 36, &packLocation, 4);
    memcpy(o + 40, &radverbCount, 4);
    memcpy(o + 56, &snapshotCount, 4);

    putXStringFromTag(r, z, obj, 0, nameTag);

    if (aliasTag != TAG_NULL) tSndAliasListArray(r, z, obj, 16, aliasCount);
    else                      z.putPtr(obj, 16, Prelink::none());

    // aliasIndex is SndIndexEntry[aliasCount], 4 pointer-free bytes each.
    if (aliasIndexTag != TAG_NULL)
        tFlatArray(r, z, obj, 24, 4u * (uint32_t)(aliasCount > 0 ? aliasCount : 0), 3);
    else
        z.putPtr(obj, 24, Prelink::none());

    if (radverbTag != TAG_NULL)          // snd_radverb, 96 flat bytes
        tFlatArray(r, z, obj, 48, 96u * (uint32_t)(radverbCount > 0 ? radverbCount : 0), 3);
    else
        z.putPtr(obj, 48, Prelink::none());

    if (snapshotTag != TAG_NULL)         // snd_snapshot, 348 flat bytes
        tFlatArray(r, z, obj, 64, 348u * (uint32_t)(snapshotCount > 0 ? snapshotCount : 0), 3);
    else
        z.putPtr(obj, 64, Prelink::none());
}

// ---- emblems ----------------------------------------------------------------
// EmblemSet -> layers / categories / icons / backgrounds / backgroundLookup.
// Load_EmblemSet (db_load.cpp:7211) reads the 44-byte block, then the five
// arrays in that order. Every array loader reads all its fixed records as one
// Load_Stream before walking them.

enum { AT_EMBLEMSET = 42, SZ_EMBLEMSET = 80,
       SZ_EMBLEMCATEGORY = 16, SZ_EMBLEMICON = 48, SZ_EMBLEMBACKGROUND = 32 };

// Load_GfxImagePtr (db_load.cpp:1978): like Load_MaterialHandle, -1 and -2
// both mean an inline GfxImage follows; any other non-zero value is an alias.
static void tGfxImagePtr(Reader &r, Prelink &z, Prelink::Loc obj, uint32_t field,
                         uint32_t tag, uint32_t x86slot = 0) {
    if (tag == TAG_INLINE || tag == TAG_ALIAS) {
        Prelink::Loc im = z.alloc(OUT, SZ_IMAGE, 8);
        tempReserve(52);       // Load_GfxImagePtr -> block 0
        if (tag == TAG_ALIAS) g_aliasMap[insertPointerSlot()] = im;
        noteAliasSlot(x86slot, im);
        tGfxImage(r, z, im);
        z.putPtr(obj, field, im);
    } else if (tag != TAG_NULL) {
        noteAliasSlot(x86slot, putAssetHandleRef(z, obj, field, tag));
    } else {
        z.putPtr(obj, field, Prelink::none());
    }
}

// EmblemCategory: 8 -> 16, two strings.
static void tEmblemCategoryArray(Reader &r, Prelink &z, Prelink::Loc obj,
                                 uint32_t field, int32_t count) {
    Prelink::Loc tbl = z.alloc(OUT, count ? (size_t)count * SZ_EMBLEMCATEGORY : 1, 8);
    b4Reserve(3, (uint32_t)count * 8, tbl);

    std::vector<uint32_t> nameTag(count), descTag(count);
    for (int i = 0; i < count; ++i) { nameTag[i] = r.u32(); descTag[i] = r.u32(); }
    for (int i = 0; i < count; ++i) {
        Prelink::Loc c{tbl.blk, tbl.off + (uint32_t)i * SZ_EMBLEMCATEGORY};
        putXStringFromTag(r, z, c, 0, nameTag[i]);
        putXStringFromTag(r, z, c, 8, descTag[i]);
    }
    z.putPtr(obj, field, tbl);
}

// EmblemIcon: 40 -> 48. An image handle, a string, and eight scalars.
static void tEmblemIconArray(Reader &r, Prelink &z, Prelink::Loc obj,
                             uint32_t field, int32_t count) {
    Prelink::Loc tbl = z.alloc(OUT, count ? (size_t)count * SZ_EMBLEMICON : 1, 8);
    b4Reserve(3, (uint32_t)count * 40, tbl);

    std::vector<uint32_t> imgTag(count), descTag(count);
    for (int i = 0; i < count; ++i) {
        imgTag[i]  = r.u32();
        descTag[i] = r.u32();
        uint8_t scalars[32] = {0};     // outlineSize .. category
        r.bytes(scalars, 32);
        memcpy(z.at(Prelink::Loc{tbl.blk, tbl.off + (uint32_t)i * SZ_EMBLEMICON}) + 16,
               scalars, 32);
    }
    for (int i = 0; i < count; ++i) {
        Prelink::Loc c{tbl.blk, tbl.off + (uint32_t)i * SZ_EMBLEMICON};
        tGfxImagePtr(r, z, c, 0, imgTag[i]);
        putXStringFromTag(r, z, c, 8, descTag[i]);
    }
    z.putPtr(obj, field, tbl);
}

// EmblemBackground: 24 -> 32. A material handle, a string, four ints.
static void tEmblemBackgroundArray(Reader &r, Prelink &z, Prelink::Loc obj,
                                   uint32_t field, int32_t count) {
    Prelink::Loc tbl = z.alloc(OUT, count ? (size_t)count * SZ_EMBLEMBACKGROUND : 1, 8);
    b4Reserve(3, (uint32_t)count * 24, tbl);

    std::vector<uint32_t> matTag(count), descTag(count);
    for (int i = 0; i < count; ++i) {
        matTag[i]  = r.u32();
        descTag[i] = r.u32();
        uint8_t scalars[16] = {0};     // cost, unlockLevel, unlockPLevel, unclassifyAt
        r.bytes(scalars, 16);
        memcpy(z.at(Prelink::Loc{tbl.blk, tbl.off + (uint32_t)i * SZ_EMBLEMBACKGROUND}) + 16,
               scalars, 16);
    }
    for (int i = 0; i < count; ++i) {
        Prelink::Loc c{tbl.blk, tbl.off + (uint32_t)i * SZ_EMBLEMBACKGROUND};
        tMaterialHandle(r, z, c, 0, matTag[i]);
        putXStringFromTag(r, z, c, 8, descTag[i]);
    }
    z.putPtr(obj, field, tbl);
}

// EmblemSet: 44 -> 80. Each of the five pointers is tested only against zero.
static void tEmblemSet(Reader &r, Prelink &z, Prelink::Loc obj) {
    int32_t  colorCount   = r.i32();
    int32_t  layerCount   = r.i32();
    uint32_t layersTag    = r.u32();
    int32_t  catCount     = r.i32();
    uint32_t catsTag      = r.u32();
    int32_t  iconCount    = r.i32();
    uint32_t iconsTag     = r.u32();
    int32_t  bgCount      = r.i32();
    uint32_t bgsTag       = r.u32();
    int32_t  lookupCount  = r.i32();
    uint32_t lookupTag    = r.u32();

    uint8_t *o = z.at(obj);
    memcpy(o,      &colorCount, 4);
    memcpy(o + 4,  &layerCount, 4);
    memcpy(o + 16, &catCount, 4);
    memcpy(o + 32, &iconCount, 4);
    memcpy(o + 48, &bgCount, 4);
    memcpy(o + 64, &lookupCount, 4);

    // EmblemLayer is three ints, so the array copies across byte for byte.
    if (layersTag != TAG_NULL)
        tFlatArray(r, z, obj, 8, 12u * (uint32_t)(layerCount > 0 ? layerCount : 0), 3);
    else
        z.putPtr(obj, 8, Prelink::none());

    if (catsTag != TAG_NULL)
        tEmblemCategoryArray(r, z, obj, 24, catCount > 0 ? catCount : 0);
    else
        z.putPtr(obj, 24, Prelink::none());

    if (iconsTag != TAG_NULL)
        tEmblemIconArray(r, z, obj, 40, iconCount > 0 ? iconCount : 0);
    else
        z.putPtr(obj, 40, Prelink::none());

    if (bgsTag != TAG_NULL)
        tEmblemBackgroundArray(r, z, obj, 56, bgCount > 0 ? bgCount : 0);
    else
        z.putPtr(obj, 56, Prelink::none());

    if (lookupTag != TAG_NULL) {       // AllocLoad_XBlendInfo -> align 2
        uint32_t n = 2u * (uint32_t)(lookupCount > 0 ? lookupCount : 0);
        Prelink::Loc b = z.alloc(OUT, n ? n : 1, 2);
        if (n) r.bytes(z.at(b), n);
        b4Reserve(1, n, b);
        z.putPtr(obj, 72, b);
    } else {
        z.putPtr(obj, 72, Prelink::none());
    }
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
    g_zoneBase = zone.data();
    g_mtrace = getenv("FFMTRACE") != nullptr;
    g_traceB4 = getenv("FFB4") != nullptr;
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

        if (dbg) {
        std::map<uint32_t, int> hist;
        for (uint32_t i = 0; i < assetCount; ++i) hist[types[i]]++;
        fprintf(stderr, "zone has %u assets:\n", assetCount);
        for (const auto &kv : hist)
            fprintf(stderr, "  type %2u  x%d\n", kv.first, kv.second);
    }

    // The XAsset array follows the script strings in block 4, so reserve it
    // rather than resetting the cursor. AllocLoad_FxElemVisStateSample, align 4.
    if (dbg) fprintf(stderr, "script strings: count=%u  block4 cursor now %u\n",
                     stringCount, g_x86b4);
    const uint32_t assetArrayOff = b4Reserve(3, 8 * assetCount, Prelink::none());
    
    for (uint32_t i = 0; i < assetCount; ++i) {
        const uint8_t *pre = r.p;
        g_curAsset = (int)i;
        switch (types[i]) {
        case AT_RAWFILE:     { Prelink::Loc o = z.alloc(OUT, 24, 8); tRawFile(r, z, o);      z.addAsset(AT_RAWFILE, o); break; }
        case AT_STRINGTABLE: { Prelink::Loc o = z.alloc(OUT, 32, 8); tStringTable(r, z, o);  z.addAsset(AT_STRINGTABLE, o); break; }
        case AT_LOCALIZE:    { Prelink::Loc o = z.alloc(OUT, 16, 8); tLocalizeEntry(r, z, o); z.addAsset(AT_LOCALIZE, o); break; }
        case AT_TECHSET:     { Prelink::Loc o = z.alloc(OUT, SZ_TECHSET, 8); tMaterialTechniqueSet(r, z, o); z.addAsset(AT_TECHSET, o); break; }
        case AT_IMAGE:       { Prelink::Loc o = z.alloc(OUT, SZ_IMAGE, 8); tGfxImage(r, z, o); z.addAsset(AT_IMAGE, o); break; }
        case AT_MATERIAL:    { Prelink::Loc o = z.alloc(OUT, SZ_MATERIAL, 8); tMaterial(r, z, o); z.addAsset(AT_MATERIAL, o); break; }
        case AT_PHYSPRESET:  { Prelink::Loc o = z.alloc(OUT, SZ_PHYSPRESET, 8); tPhysPreset(r, z, o); z.addAsset(AT_PHYSPRESET, o); break; }
        case AT_XGLOBALS:    { Prelink::Loc o = z.alloc(OUT, SZ_XGLOBALS, 8);   tXGlobals(r, z, o);   z.addAsset(AT_XGLOBALS, o);   break; }        
        case AT_LIGHTDEF:    { Prelink::Loc o = z.alloc(OUT, SZ_LIGHTDEF, 8);  tGfxLightDef(r, z, o); z.addAsset(AT_LIGHTDEF, o); break; }        
        case AT_PHYSCONSTRAINTS: { Prelink::Loc o = z.alloc(OUT, SZ_PHYSCONSTRAINTS, 8); tPhysConstraints(r, z, o); z.addAsset(AT_PHYSCONSTRAINTS, o); break; }        
        case AT_XANIM:       { Prelink::Loc o = z.alloc(OUT, SZ_XANIM, 8); tXAnimParts(r, z, o); z.addAsset(AT_XANIM, o); break; }
        case AT_XMODEL:      { Prelink::Loc o = z.alloc(OUT, SZ_XMODEL, 8); tXModel(r, z, o); z.addAsset(AT_XMODEL, o); break; }        
        case AT_MENUFILE:    { Prelink::Loc o = z.alloc(OUT, SZ_MENULIST, 8); tMenuList(r, z, o); z.addAsset(AT_MENUFILE, o); break; }
        case AT_FX:          { Prelink::Loc o = z.alloc(OUT, SZ_FXEFFECTDEF, 8); tFxEffectDef(r, z, o); z.addAsset(AT_FX, o); break; }
        case AT_WEAPON:      { Prelink::Loc o = z.alloc(OUT, SZ_WEAPONVARIANTDEF, 8); tWeaponVariantDef(r, z, o); z.addAsset(AT_WEAPON, o); break; }
        case AT_IMPACTFX:    { Prelink::Loc o = z.alloc(OUT, SZ_FXIMPACTTABLE, 8); tFxImpactTable(r, z, o); z.addAsset(AT_IMPACTFX, o); break; }
        case AT_SNDDRIVERGLOBALS: { Prelink::Loc o = z.alloc(OUT, SZ_SNDDRIVERGLOBALS, 8); tSndDriverGlobals(r, z, o); z.addAsset(AT_SNDDRIVERGLOBALS, o); break; }
        case AT_FONT:        { Prelink::Loc o = z.alloc(OUT, SZ_FONT, 8); tFont(r, z, o); z.addAsset(AT_FONT, o); break; }
        case AT_DDL:         { Prelink::Loc o = z.alloc(OUT, SZ_DDLROOT, 8); tDdlRoot(r, z, o); z.addAsset(AT_DDL, o); break; }
        case AT_SOUND:       { Prelink::Loc o = z.alloc(OUT, SZ_SNDBANK, 8); tSndBank(r, z, o); z.addAsset(AT_SOUND, o); break; }
        case AT_EMBLEMSET:   { Prelink::Loc o = z.alloc(OUT, SZ_EMBLEMSET, 8); tEmblemSet(r, z, o); z.addAsset(AT_EMBLEMSET, o); break; }
        default:
            fprintf(stderr, "unsupported asset type %u at index %u (Stage 1 = rawfile/stringtable/localize)\n", types[i], i);
            return 3;
        }
        if (dbg)
            printf("[%3u] type=%2u off=%6zu consumed=%3zd hdrTag=%08x\n",
                   i, types[i], (size_t)(pre - zone.data()), (ptrdiff_t)(r.p - pre), hdrTag[i]);
        if (r.overran) { fprintf(stderr, "stream overran at asset %u -- transcoder desync\n", i); return 4; }
        // XAsset[i].header now holds this asset's pointer, and that slot is
        // what a later DB_ConvertOffsetToAlias reference dereferences.
        if (!z.assets.empty()) {
            const Prelink::AssetRef &a = z.assets.back();
            g_aliasMap[assetArrayOff + i * 8 + 4] = Prelink::Loc{(int)a.blk, a.off};
        }
    }

    // pass 2: resolve deferred block-4 offset refs against the emulated map
    int resolved = 0, missing = 0, firstBad = -1;
    for (const Deferred &d : g_deferred) {
        auto it = g_b4map.find(d.x86off);
        if (it != g_b4map.end()) { z.putPtr(d.obj, d.field, it->second); ++resolved; }
        else {
            ++missing;
            if (firstBad < 0) firstBad = d.asset;
            if (dbg && missing <= 20) {
                uint32_t st = 0, sz = 0; int ln = 0;
                for (const auto &e : g_b4log)
                    if (e.off <= d.x86off && e.off >= st) { st = e.off; sz = e.size; ln = e.line; }
                printf("  UNRESOLVED off=%u (asset %d, ref from line %d) inside [%u,+%u) at +%u  reserved at line %d\n",
                       d.x86off, d.asset, d.line, st, sz, d.x86off - st, ln);
            }
        }
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
    printf("  asset handles: resolved=%d missing=%d  otherBlock=%d\n",
           g_aliasOk, g_aliasBad, g_cntAliasOther);
    if (missing) printf("  first unresolved ref comes from asset %d\n", firstBad);
    printf("  temp (block 0) bytes, not counted in block 4: %u\n", g_x86temp);
    return (b4ok && missing == 0 && g_aliasBad == 0) ? 0 : 5;
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

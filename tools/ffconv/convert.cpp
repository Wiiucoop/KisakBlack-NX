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

// Resolve a non-string pointer slot from its stream tag.
static void putStructOffsetRef(Prelink &z, Prelink::Loc obj, uint32_t field, uint32_t tag) {
    uint32_t blk = (tag - 1) >> 29, off = (tag - 1) & 0x1FFFFFFF;
    if (blk == 4) g_deferred.push_back({obj, field, off});
    else { ++g_cntOffOther; z.putPtr(obj, field, Prelink::none()); }
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
static void tGfxImageLoadDef(Reader &r, Prelink &z, Prelink::Loc img) {
    uint8_t  levelCount = 0, flags = 0;
    r.bytes(&levelCount, 1);
    r.bytes(&flags, 1);
    r.u16();                                  // padding
    int32_t format       = r.i32();
    int32_t resourceSize = r.i32();

    const uint32_t total = 12 + (uint32_t)(resourceSize > 0 ? resourceSize : 0);
    Prelink::Loc def = z.alloc(OUT, total, 4);
    b4Reserve(3, total, def);

    uint8_t *d = z.at(def);
    d[0] = levelCount;
    d[1] = flags;
    memcpy(d + 4, &format, 4);
    memcpy(d + 8, &resourceSize, 4);
    if (resourceSize > 0) r.bytes(d + 12, (size_t)resourceSize);

    z.putPtr(img, 0, def);                    // GfxImage.texture.loadDef
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
        tGfxImageLoadDef(r, z, obj);
    } else if (texTag != TAG_NULL) {
        putStructOffsetRef(z, obj, 0, texTag);
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
        b4Reserve(3, 528, t);
        tMaterialTechniqueSet(r, z, t);
        z.putPtr(obj, 176, t);
    } else if (techTag != TAG_NULL) {
        putStructOffsetRef(z, obj, 176, techTag);
    }

    const int nTex   = counts[0];
    const int nConst = counts[1];
    const int nSb    = counts[2];

    if (texTag == TAG_INLINE) {
        Prelink::Loc tbl = z.alloc(OUT, (size_t)nTex * SZ_TEXDEF, 8);
        b4Reserve(3, (uint32_t)nTex * 16, tbl);

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
            if (uTags[i] == TAG_INLINE || uTags[i] == TAG_ALIAS) {
                Prelink::Loc im = z.alloc(OUT, SZ_IMAGE, 8);
                b4Reserve(3, 52, im);
                tGfxImage(r, z, im);
                z.putPtr(e, 16, im);
            } else if (uTags[i] != TAG_NULL) {
                putStructOffsetRef(z, e, 16, uTags[i]);
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

enum { AT_PHYSPRESET = 1, AT_LIGHTDEF = 18, AT_XGLOBALS = 40 };
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
        b4Reserve(3, 52, im);
        tGfxImage(r, z, im);
        z.putPtr(obj, 8, im);           // attenuation.image
    } else if (imageTag != TAG_NULL) {
        putStructOffsetRef(z, obj, 8, imageTag);
    }
}

// MenuList: 12 -> 24. Name, count, and an array of menuDef_t pointers.
// menuDef_t is the whole UI menu tree; a default menu file should not have
// any, so stop loudly rather than guess.
enum { AT_MENUFILE = 21, SZ_MENULIST = 24 };

static void tMenuList(Reader &r, Prelink &z, Prelink::Loc obj) {
    uint32_t nameTag   = r.u32();
    int32_t  menuCount = r.i32();
    uint32_t menusTag  = r.u32();

    memcpy(z.at(obj) + 8, &menuCount, 4);
    putXStringFromTag(r, z, obj, 0, nameTag);

    if (menusTag != TAG_NULL) {
        fprintf(stderr, "menulist has %d menus; menuDef_t is not transcoded yet\n",
                menuCount);
        exit(12);
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

        if (tags[i].mat == TAG_INLINE || tags[i].mat == TAG_ALIAS) {
            Prelink::Loc m = z.alloc(OUT, SZ_MATERIAL, 8);
            b4Reserve(3, 192, m);
            tMaterial(r, z, m);
            z.putPtr(c, 160, m);
        } else if (tags[i].mat != TAG_NULL) {
            putStructOffsetRef(z, c, 160, tags[i].mat);
        }
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
    if (tag == TAG_NULL) return;
    const uint32_t n = count * elemSize;
    Prelink::Loc buf = z.alloc(OUT, n ? n : 1, elemSize);
    if (n) r.bytes(z.at(buf), n);
    b4Reserve(allocParam, n, buf);
    z.putPtr(obj, field, buf);
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

static void tXSurface(Reader &r, Prelink &z, Prelink::Loc obj) {
    uint8_t  head[12];                 // tileMode .. baseVertIndex
    r.bytes(head, 12);
    uint32_t triIdxTag = r.u32();

    int16_t  vcInfo[4] = {0};          // vertInfo.vertCount[4]
    r.bytes(vcInfo, 8);
    uint32_t blendTag   = r.u32();
    uint32_t tensionTag = r.u32();

    uint32_t verts0Tag  = r.u32();
    r.u32();                           // vb0 -- runtime object
    uint32_t vertListTag= r.u32();
    r.u32();                           // indexBuffer -- runtime object
    uint8_t  partBits[20];
    r.bytes(partBits, 20);

    uint8_t *o = z.at(obj);
    memcpy(o,       head, 12);
    memcpy(o + 24,  vcInfo, 8);        // vertInfo.vertCount
    memcpy(o + 80,  partBits, 20);

    uint16_t vertListCount, vertCount, triCount;
    memcpy(&vertListCount, head + 1, 1);
    vertListCount = head[1];
    memcpy(&vertCount, head + 4, 2);
    memcpy(&triCount,  head + 6, 2);

    // vertInfo lives inline at offset 24; its two pointers are at 32 and 40.
    Prelink::Loc vi{obj.blk, obj.off + 24};

    if (blendTag == TAG_INLINE) {
        const uint32_t n = 7u * (uint32_t)vcInfo[3] + 5u * (uint32_t)vcInfo[2]
                         + 3u * (uint32_t)vcInfo[1] + (uint32_t)vcInfo[0];
        tSimpleArray(r, z, vi, 8, blendTag, n, 2, 1);
    } else if (blendTag != TAG_NULL) {
        putStructOffsetRef(z, vi, 8, blendTag);
    }

    if (tensionTag != TAG_NULL) {
        fprintf(stderr, "xsurface has tensionData; not transcoded yet\n");
        exit(10);
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

    if (boneNamesTag == TAG_INLINE) tSimpleArray(r, z, obj, 16, boneNamesTag, nBones, 2, 1);
    else if (boneNamesTag != TAG_NULL) putStructOffsetRef(z, obj, 16, boneNamesTag);

    if (parentTag == TAG_INLINE) tSimpleArray(r, z, obj, 24, parentTag, nDiff, 1, 0);
    else if (parentTag != TAG_NULL) putStructOffsetRef(z, obj, 24, parentTag);

    if (quatsTag == TAG_INLINE) tSimpleArray(r, z, obj, 32, quatsTag, nDiff * 4, 2, 1);
    else if (quatsTag != TAG_NULL) putStructOffsetRef(z, obj, 32, quatsTag);

    if (transTag == TAG_INLINE) tSimpleArray(r, z, obj, 40, transTag, nDiff * 4, 4, 3);
    else if (transTag != TAG_NULL) putStructOffsetRef(z, obj, 40, transTag);

    if (partClassTag == TAG_INLINE) tSimpleArray(r, z, obj, 48, partClassTag, nBones, 1, 0);
    else if (partClassTag != TAG_NULL) putStructOffsetRef(z, obj, 48, partClassTag);

    if (baseMatTag == TAG_INLINE) tSimpleArray(r, z, obj, 56, baseMatTag, nBones, 32, 3);
    else if (baseMatTag != TAG_NULL) putStructOffsetRef(z, obj, 56, baseMatTag);

    if (surfsTag != TAG_NULL) {
        Prelink::Loc tbl = z.alloc(OUT, (size_t)nSurfs * SZ_XSURFACE, 8);
        b4Reserve(3, nSurfs * 68, tbl);
        for (uint32_t i = 0; i < nSurfs; ++i) {
            Prelink::Loc s{tbl.blk, tbl.off + i * SZ_XSURFACE};
            tXSurface(r, z, s);
        }
        z.putPtr(obj, 64, tbl);
    }

    if (matHandlesTag != TAG_NULL) {
        Prelink::Loc tbl = z.alloc(OUT, (size_t)nSurfs * 8, 8);
        b4Reserve(3, nSurfs * 4, tbl);
        for (uint32_t i = 0; i < nSurfs; ++i) {
            uint32_t tag = r.u32();
            Prelink::Loc e{tbl.blk, tbl.off + i * 8};
            if (tag == TAG_INLINE || tag == TAG_ALIAS) {
                Prelink::Loc m = z.alloc(OUT, SZ_MATERIAL, 8);
                b4Reserve(3, 192, m);
                tMaterial(r, z, m);
                z.putPtr(e, 0, m);
            } else if (tag != TAG_NULL) {
                putStructOffsetRef(z, e, 0, tag);
            }
        }
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
        b4Reserve(3, 84, p);
        tPhysPreset(r, z, p);
        z.putPtr(obj, 296, p);
    } else if (physPresetTag != TAG_NULL) {
        putStructOffsetRef(z, obj, 296, physPresetTag);
    }

    if (collmapsTag != TAG_NULL) {
        fprintf(stderr, "xmodel has collmaps; PhysGeomList is not transcoded yet\n");
        exit(9);
    }

    if (physConstrTag == TAG_INLINE || physConstrTag == TAG_ALIAS) {
        Prelink::Loc p = z.alloc(OUT, SZ_PHYSCONSTRAINTS, 8);
        b4Reserve(3, 2696, p);
        tPhysConstraints(r, z, p);
        z.putPtr(obj, 320, p);
    } else if (physConstrTag != TAG_NULL) {
        putStructOffsetRef(z, obj, 320, physConstrTag);
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
    // rather than resetting the cursor.
    b4Reserve(0, 8 * assetCount, Prelink::none());
    
    for (uint32_t i = 0; i < assetCount; ++i) {
        const uint8_t *pre = r.p;
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

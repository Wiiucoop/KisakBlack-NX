// prelink.h -- "prelinked zone" builder + x86 stream reader for the offline
// fastfile converter. Shared by the per-asset transcoders.
//
// Output format (KBZ1): per-block LP64 images + a relocation table + an asset
// table. The on-device side becomes a generic relocator (alloc blocks, memcpy,
// add block base to each reloc slot, register assets) -- no per-asset code.
#pragma once
#include <cstdio>
#include <cstdint>
#include <cstring>
#include <cstdlib>
#include <vector>
#include <string>

// ------------------------------------------------------------ x86 stream reader
// Sequential cursor over the inflated x86 zone (after the 36-byte XFile header).
// The stream is gapless: DB_AllocStreamPos aligns the destination memory cursor,
// not the stream, so fields are read at their exact x86 sizes back to back.
struct Reader {
    const uint8_t *p;
    const uint8_t *end;
    bool overran = false;

    Reader(const uint8_t *base, size_t len) : p(base), end(base + len) {}

    uint32_t u32() {
        if (p + 4 > end) { overran = true; return 0; }
        uint32_t v; memcpy(&v, p, 4); p += 4; return v;
    }
    int32_t i32() { return (int32_t)u32(); }
    uint16_t u16() {
        if (p + 2 > end) { overran = true; return 0; }
        uint16_t v; memcpy(&v, p, 2); p += 2; return v;
    }
    void bytes(void *dst, size_t n) {
        if (p + n > end) { overran = true; return; }
        memcpy(dst, p, n); p += n;
    }
    void skip(size_t n) { p += n; if (p > end) overran = true; }
    // Read a NUL-terminated string inline (tag was 0xFFFFFFFF); returns it and
    // advances past the terminator.
    std::string cstr() {
        std::string s;
        while (p < end && *p) s.push_back((char)*p++);
        if (p < end) ++p; // NUL
        return s;
    }
};

// pointer tag values on disk
static const uint32_t TAG_NULL   = 0x00000000u;
static const uint32_t TAG_INLINE = 0xFFFFFFFFu; // -1: data follows inline
static const uint32_t TAG_ALIAS  = 0xFFFFFFFEu; // -2: back-reference

// ------------------------------------------------------------ prelinked builder
struct Prelink {
    static const int NBLOCK = 8;
    std::vector<uint8_t> block[NBLOCK];

    struct Reloc { uint8_t slotBlk; uint32_t slotOff; uint8_t tgtBlk; uint32_t tgtOff; };
    std::vector<Reloc> relocs;
    struct AssetRef { uint32_t type; uint8_t blk; uint32_t off; };
    std::vector<AssetRef> assets;
    std::vector<std::pair<uint8_t,uint32_t>> scriptStrings; // (blk,off) per script string

    // A location in the output zone.
    struct Loc { int blk; uint32_t off; bool valid() const { return blk >= 0; } };
    static Loc none() { return Loc{-1, 0}; }

    // Reserve `size` bytes in block `blk`, aligned to `align`, zero-filled.
    Loc alloc(int blk, size_t size, size_t align) {
        std::vector<uint8_t> &b = block[blk];
        size_t off = (b.size() + (align - 1)) & ~(align - 1);
        b.resize(off + size, 0);
        return Loc{blk, (uint32_t)off};
    }
    uint8_t *at(Loc l) { return block[l.blk].data() + l.off; }

    void putU32(Loc l, uint32_t field, uint32_t v) { memcpy(at(l) + field, &v, 4); }
    void putI32(Loc l, uint32_t field, int32_t v)  { memcpy(at(l) + field, &v, 4); }

    // Store an 8-byte pointer slot at field `field` of object `l`, pointing at
    // `tgt`, and record the relocation.
    void putPtr(Loc l, uint32_t field, Loc tgt) {
        if (!tgt.valid()) { uint64_t z = 0; memcpy(at(l) + field, &z, 8); return; }
        relocs.push_back({(uint8_t)l.blk, l.off + field, (uint8_t)tgt.blk, tgt.off});
    }

    void addAsset(uint32_t type, Loc hdr) { assets.push_back({type, (uint8_t)hdr.blk, hdr.off}); }

    // Copy a raw byte buffer (e.g. rawfile contents, string chars) into a block.
    Loc putBytes(int blk, const uint8_t *data, size_t n, size_t align) {
        Loc l = alloc(blk, n, align);
        memcpy(at(l), data, n);
        return l;
    }

    void write(const char *path);
};

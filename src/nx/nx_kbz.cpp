// nx_kbz.cpp -- on-device loader for KBZ1 "prelinked zones".
//
// KBZ1 files are produced offline by tools/ffconv/convert.exe from the shipped
// 32-bit .ff fastfiles. Each asset has already been transcoded to its native
// LP64 layout and every internal pointer flattened to a (block,offset) pair in
// a relocation table. Loading is therefore a generic relocate -- NO per-asset
// parsing on device:
//   1. allocate each block and copy its image,
//   2. walk the reloc table, writing block[tgt]+off into each pointer slot,
//   3. register each asset into the XAssetPool, running the same hooks
//      Load_<T>Ptr would have run (see step 3 below),
//   4. build the runtime objects db_load.cpp would have built (see step 4 below).
//
// Steps 1-2 need to know nothing about any asset type. Steps 3 and 4 do, and
// they are the two places where per-type knowledge lives: step 3 mirrors the
// Load_<T>Asset hooks of db_registry.cpp, step 4 the Load_Create* builders
// db_load.cpp calls into the subsystems.
//
// This sidesteps the fundamental blocker that the .ff stream format hard-wires
// 4-byte pointers and x86 struct sizes, which cannot be consumed directly by a
// 64-bit build. See tools/ffconv/ for the offline half and the format spec.
#ifdef KISAK_NX
#include <cstdio>
#include <cstdint>
#include <cstring>
#include <cstdlib>
#include <algorithm>
#include <utility>
#include <vector>

#include <database/database.h>
#include <database/db_assetnames.h>
#include <database/db_registry.h>
#include <qcommon/common.h>
#include <universal/q_shared.h>
#include <gfx_d3d/r_material.h>
#include <gfx_d3d/rb_resource.h>
#include <sound/snd_bank.h>
#include <ui/ui_shared.h>

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

// ---------------------------------------------------------------------------
// Step 3: register an asset, running the hooks Load_<T>Ptr would have run.
//
// Every Load_<T>Ptr in db_load.cpp ends with a call to Load_<T>Asset, and those
// hooks (db_registry.cpp) are not all the same: most are a bare DB_AddXAsset,
// but four of them also hand the asset to the subsystem that owns it. Calling
// DB_AddXAsset alone -- what this loader used to do -- registers the asset in
// the pool but leaves that subsystem unaware it exists.
//
// The census of the hooks that do more than register, over the whole of
// db_registry.cpp:
//
//   asset type         hook                   extra work
//   -----------------  ---------------------  ----------------------------------
//   IMAGE (8)          Load_GfxImageAsset     RB_Resource_Flush() *before* adding
//   SOUND (9)          Load_SndBankAsset      SND_AddBank -> g_snd_banks[]
//   SOUND_PATCH (10)   Load_SndPatchAsset     SND_AddPatch -> g_snd_patches[]
//   MENU (22)          Load_MenuAsset         items[i]->parent = the added menu
//
// SOUND is the one that bites first: SND_GetSnapshotById (snd_bank.cpp:398)
// walks g_snd_banks[], not the SndDriverGlobals asset, so with no bank ever
// added it returns null for every id -- including g_snd.defaultHash, whose
// `defaultHash != id` guard stops the fallback from recursing. SND_Init ->
// SND_InitSnapshot -> SND_UpdateSnapshot (snd.cpp:2940) then dereferences that
// null immediately, because snapshotGroupCount (61 in code_post_gfx_mp) makes
// the inner loop run. On PC the bank ships in the same fastfile and is added
// during the load, so the table is never empty there and the engine is right
// not to guard.
//
// Two details are load-bearing and copied from the hooks verbatim:
//  - DB_AddXAsset dedups. It returns the header of the entry that won, which
//    may not be the one passed in, and the hooks all hand the subsystem the
//    RETURNED pointer. Load_MenuAsset is the one that needs both: it reparents
//    the items of the menu it was given onto the menu that won.
//  - RB_Resource_Flush runs before the add, not after.
//
// This runs in the asset table's own order, which is the order the .ff had:
// SND_AddBank reapplies every patch registered so far (snd_bank.cpp:64-69), so
// banks and patches must interleave the way the fastfile wrote them.
// ---------------------------------------------------------------------------
static XAssetHeader registerAsset(XAssetType type, XAssetHeader h)
{
    if (type == ASSET_TYPE_IMAGE)
        RB_Resource_Flush();

    XAssetHeader given = h;
    XAssetHeader added = DB_AddXAsset(type, h);

    switch (type) {
    case ASSET_TYPE_SOUND:
        SND_AddBank(added.sound);
        break;
    case ASSET_TYPE_SOUND_PATCH:
        SND_AddPatch(added.soundPatch);
        break;
    case ASSET_TYPE_MENU:
        if (given.menu)
            for (int i = 0; i < given.menu->itemCount; ++i)
                if (given.menu->items[i])
                    given.menu->items[i]->parent = added.menu;
        break;
    default:
        break;
    }
    return added;
}

// ---------------------------------------------------------------------------
// Step 4: build the runtime objects the .ff loader would have built.
//
// db_load.cpp does more than fill structs. After reading a block it calls back
// into the owning subsystem to turn a load def into a live device object, and
// those calls are the half of loading that the KBZ path drops on the floor.
// Every field they would have written stays at the zero the transcoder left,
// because convert.cpp deliberately does not carry runtime objects across
// ("prog.vs (or .ps) is a runtime D3D object; only the load def carries over").
//
// The census of those hooks -- every Load_ function db_load.cpp calls that is
// defined in a subsystem translation unit and constructs something, rather than
// reading the stream:
//
//   asset type          hook (db_load.cpp call site)          builds
//   ------------------  ------------------------------------  --------------------------
//   TECHNIQUE_SET (7)   Load_CreateMaterialVertexShader :2099  MaterialVertexShader::prog.vs
//                       Load_CreateMaterialPixelShader  :2107  MaterialPixelShader::prog.ps
//                       Load_BuildVertexDecl            :2256  MaterialVertexDeclaration::routing.decl[18]
//   IMAGE (8)           Load_Texture                    :1923  GfxImage::texture.basemap
//   GFXWORLD (17)       Load_VertexBuffer         :7984,:8001  static vertex buffers
//   MATERIAL (6)        Load_PicmipWater                :2334  water_t FFT tables (CPU-side)
//
// Only TECHNIQUE_SET is wired up here: it is what the first draw call needs,
// since R_SetPixelShader (r_shade.cpp:876) asserts on a null prog.ps before it
// touches anything else. The table below is the extension point -- one row per
// asset type, run in table order, so a type that must be built after another
// just goes further down the list.
// ---------------------------------------------------------------------------

// A builder returns how many runtime objects it actually created, for logging.
typedef unsigned (*NxAssetBuilder)(XAssetHeader);

struct NxBuildStep {
    XAssetType     type;
    NxAssetBuilder build;
    const char    *what;   // plural noun, for the summary line
};

// Passes inside a zone share their shader and vertex-decl objects with every
// other pass that referenced the same one in the original .ff, so a builder is
// reached many times for the same object. The prog.vs / prog.ps / isLoaded
// tests below are what keep the build idempotent: without them the second
// visitor creates a duplicate and orphans the first.
static unsigned buildTechniqueSet(XAssetHeader h)
{
    MaterialTechniqueSet *techSet = h.techniqueSet;
    if (!techSet) return 0;

    unsigned built = 0;
    for (unsigned t = 0; t < ARRAY_COUNT(techSet->techniques); ++t) {
        MaterialTechnique *tech = techSet->techniques[t];
        if (!tech) continue;
        for (unsigned pass_i = 0; pass_i < tech->passCount; ++pass_i) {
            MaterialPass *pass = &tech->passArray[pass_i];

            if (pass->vertexShader && !pass->vertexShader->prog.vs) {
                Load_CreateMaterialVertexShader(&pass->vertexShader->prog.loadDef,
                                                pass->vertexShader);
                ++built;
            }
            if (pass->pixelShader && !pass->pixelShader->prog.ps) {
                Load_CreateMaterialPixelShader(&pass->pixelShader->prog.loadDef,
                                               pass->pixelShader);
                ++built;
            }
            if (pass->vertexDecl && !pass->vertexDecl->isLoaded) {
                Load_BuildVertexDecl(&pass->vertexDecl);
                ++built;
            }
        }
    }
    return built;
}

static const NxBuildStep kBuildSteps[] = {
    { ASSET_TYPE_TECHNIQUE_SET, buildTechniqueSet, "shader/vertex-decl objects" },
};

// Walk the zone's asset table once per step, in table order.
static void buildRuntimeObjects(const char *path, const uint8_t *assetTable,
                                const uint8_t *end, uint32_t assetCount,
                                uint8_t *const *block, const uint32_t *blockSize,
                                uint32_t nblk)
{
    for (unsigned step = 0; step < ARRAY_COUNT(kBuildSteps); ++step) {
        const NxBuildStep *bs = &kBuildSteps[step];
        const uint8_t *p = assetTable;
        unsigned built = 0, visited = 0;

        for (uint32_t i = 0; i < assetCount; ++i) {
            if (p + 9 > end) break;
            uint32_t type; memcpy(&type, p, 4); p += 4;
            uint8_t  ab = *p++; uint32_t ao; memcpy(&ao, p, 4); p += 4;
            if ((XAssetType)type != bs->type) continue;
            if (ab >= nblk || !block[ab] || ao >= blockSize[ab]) continue;
            XAssetHeader h;
            h.data = block[ab] + ao;
            built += bs->build(h);
            ++visited;
        }

        if (visited)
            Com_Printf(16, "NX_KBZ: '%s' built %u %s over %u assets of type %d\n",
                       path, built, bs->what, visited, (int)bs->type);
    }
}

// ---------------------------------------------------------------------------
// Point every reference at the pool entry, not at the block.
//
// DB_AddXAsset never keeps the header it is given. DB_LinkXAssetEntry allocates
// a pool entry and memcpys the struct into it (DB_CloneXAssetInternal,
// db_registry.cpp:1992), so the returned header is always a different pointer.
// Every Load_<T>Ptr in db_load.cpp closes on that fact: after loading an inline
// asset it calls Load_<T>Asset, and the hook writes the returned header back
// through the slot it was loaded from -- `*material = DB_AddXAsset(...)` for
// materials (db_registry.cpp:685), `menu->menu = DB_AddXAsset(...)` for menus
// (:879), and so on for all 32 of them. Every later reference in the zone is a
// DB_ConvertOffsetToAlias that reads one of those slots, so on PC nothing keeps
// pointing at the loaded copy.
//
// Here those slots are relocations to the block copy, filled before any asset
// is registered. ffconv flattened the aliases, so a reference that on PC read
// the slot is here a relocation of its own, and all of them have to move: after
// an asset is registered, every relocation whose target was its block copy is
// rewritten to the pool entry.
//
// The timing is the load-bearing part, and it is why this cannot be deferred to
// one sweep at the end. DB_AddXAsset *clones the struct*, so a pointer field
// inside an asset is frozen at the moment that asset is registered. Redirect a
// slot after its containing asset has been cloned and the fix lands in the
// block copy that nothing reads any more, while the pool entry keeps the stale
// pointer. So each asset's slots are rewritten immediately after it is
// registered, and ffconv emits an inline asset into the table ahead of the
// parent that holds it -- which is stream order, which is PC order.
//
// The pre-pass stays for the same reason it was written: it is keyed on asset
// header addresses (a few hundred per zone), so one binary search per
// relocation collects only the slots that can ever move, instead of carrying a
// map of every relocation in the zone (837k in patch_mp).
// ---------------------------------------------------------------------------
class AssetSlots
{
public:
    void collect(const uint8_t *assetTable, const uint8_t *end, uint32_t assetCount,
                 const uint8_t *relocTable, uint32_t relocCount,
                 uint8_t *block[], const uint32_t blockSize[], uint32_t nblk)
    {
        std::vector<const void *> headers;
        const uint8_t *p = assetTable;
        for (uint32_t i = 0; i < assetCount && p + 9 <= end; ++i) {
            p += 4;                             // type: every type moves, so it does not matter
            uint8_t  ab = *p++; uint32_t ao; memcpy(&ao, p, 4); p += 4;
            if (ab < nblk && block[ab] && ao < blockSize[ab])
                headers.push_back(block[ab] + ao);
        }
        if (headers.empty())
            return;
        std::sort(headers.begin(), headers.end());

        p = relocTable;
        for (uint32_t i = 0; i < relocCount && p + 10 <= end; ++i) {
            uint8_t  sb = *p++; uint32_t so; memcpy(&so, p, 4); p += 4;
            uint8_t  tb = *p++; uint32_t to; memcpy(&to, p, 4); p += 4;
            if (sb >= nblk || tb >= nblk || !block[sb] || !block[tb]) continue;
            if (so + sizeof(void *) > blockSize[sb] || to > blockSize[tb]) continue;
            const void *tgt = block[tb] + to;
            if (std::binary_search(headers.begin(), headers.end(), tgt))
                m_slots.emplace_back(tgt, block[sb] + so);
        }
        std::sort(m_slots.begin(), m_slots.end());
    }

    // Returns how many slots were rewritten, for the probe at the call site.
    uint32_t redirect(const void *given, void *added)
    {
        if (given == added)
            return 0;
        auto it = std::lower_bound(m_slots.begin(), m_slots.end(),
                                   std::make_pair(given, (uint8_t *)nullptr));
        uint32_t n = 0;
        for (; it != m_slots.end() && it->first == given; ++it, ++n)
            memcpy(it->second, &added, sizeof(void *));
        m_moved += n;
        if (n)
            ++m_assetsMoved;
        else
            ++m_assetsUnreferenced;   // a top-level asset nothing in the zone points at
        return n;
    }

    uint32_t slotCount() const { return (uint32_t)m_slots.size(); }
    uint32_t moved() const { return m_moved; }
    uint32_t assetsMoved() const { return m_assetsMoved; }
    uint32_t assetsUnreferenced() const { return m_assetsUnreferenced; }

private:
    uint32_t m_moved = 0;
    uint32_t m_assetsMoved = 0;
    uint32_t m_assetsUnreferenced = 0;
    std::vector<std::pair<const void *, uint8_t *>> m_slots; // asset header -> slot
};

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

#ifdef KISAK_NX
    // Probe: print where the blocks landed, so any pointer from a later assert
    // can be placed. Inside one of these ranges means the reference is still on
    // the zone's own copy; outside means it reached an XAssetPool entry.
    for (uint32_t i = 0; i < nblk; ++i)
        if (block[i])
            printf("[nx-kbz] '%s' block %u = [%p, %p) %u bytes\n",
                   path, i, (void *)block[i], (void *)(block[i] + blockSize[i]), blockSize[i]);
    fflush(stdout);
#endif

    // 2. apply relocations: each stored pointer slot becomes a real address.
    const uint8_t *relocTable = p;
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
    const uint8_t *assetTable = p;
    AssetSlots slots;
    slots.collect(assetTable, end, assetCount, relocTable, relocCount, block, blockSize, nblk);
    for (uint32_t i = 0; i < assetCount; ++i) {
        if (p + 9 > end) break;
        uint32_t type; memcpy(&type, p, 4); p += 4;
        uint8_t  ab = *p++; uint32_t ao; memcpy(&ao, p, 4); p += 4;
        if (ab >= nblk || !block[ab] || ao >= blockSize[ab]) continue;
        XAssetHeader h;
        h.data = block[ab] + ao;
        XAssetHeader added = registerAsset((XAssetType)type, h);
        uint32_t moved = slots.redirect(h.data, added.data);
#ifdef KISAK_NX
        // Probe: one line per asset the database moved, with the slot count.
        // DB_AddXAsset clones into the pool, so `added` differs for practically
        // every asset; what matters is `moved`. A zero there means no relocation
        // in this zone pointed at the block copy, so nothing in the zone can
        // reach what the database returned. A stub (name begins with ',') that
        // reports zero is the case we are hunting: DB_LinkXAssetEntry resolved
        // it to another zone's entry (db_registry.cpp:2528) and the references
        // stayed behind on the empty stub.
        if (added.data != h.data) {
            XAsset probe; probe.type = (XAssetType)type; probe.header = h;
            const char *nm = DB_GetXAssetNameNoAssert(&probe);
            printf("[nx-kbz] asset %u t%u '%s' given=%p added=%p moved=%u%s\n",
                   i, type, nm ? nm : "(unnamed)", h.data, added.data, moved,
                   (nm && nm[0] == ',' && !moved) ? "  <-- STUB, NOTHING REDIRECTED" : "");
            fflush(stdout);
        }
#endif
    }
    if (slots.slotCount())
        Com_Printf(16, "NX_KBZ: '%s' redirected %u of %u reference slots to pool entries"
                       " (%u assets moved, %u referenced by nothing)\n",
                   path, slots.moved(), slots.slotCount(),
                   slots.assetsMoved(), slots.assetsUnreferenced());

    // 4. build the runtime objects db_load.cpp would have built. This runs
    // after registration so a builder may look assets up by name if it needs to.
    buildRuntimeObjects(path, assetTable, end, assetCount, block, blockSize, nblk);

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

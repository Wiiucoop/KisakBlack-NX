# Pointer-offset reads

Sites where the decompiler wrote a raw indexed read — `*((unsigned int *)cent + 201)`
— instead of a named field, freezing an x86 byte offset into the source. On LP64
any pointer ahead of that field widens, the field moves, and the literal index
lands somewhere else entirely. Nothing warns.

Produced by the pipeline in [../README.md](../README.md). Counts are
"(struct, cast, index)" groups and the number of source sites in each.

## Closed

### `centity_s + 201` — 217 sites, 35 files

| | |
| --- | --- |
| shape | `*((unsigned int *)cent + 201)`, `*((_DWORD *)cent + 201)` |
| x86 | byte 804 — the 25-bit client flag group starting at `applyLeftHandIK` |
| LP64 | byte 804 is `nextSlideFX`; the flag group moved to byte 848 |
| cause | ten pointers sit between `nextSlideFX` and the flag group (`tree`, `destructible`, `nitrousVeh`, `linkInfo`, `vehicle`, `clientTagCache`, `aimTargetInfo`, `cScriptMover`, `compassMaterial`, `updateDelayedNext`) |

The single largest group in the census — 217 of 1357 hits, spread over 35 files
in `cgame`, `cgame_mp`, `aim_assist`, `flame`, `ik`, `physics`, `ragdoll` and
`EffectsCore`. Both reads (`(x >> 1) & 1`) and read-modify-writes
(`x |= 0x100`, `x &= ~0x20`) — so on LP64 this was reading *and corrupting*
`nextSlideFX` while every client flag stayed stuck at its initial value.

Fixed by naming the word. `centity_s` now wraps the bitfield group in an
anonymous union (`bg_local.h`) that aliases it as `clientFlags`, which does not
change the layout, and every site became `cent->clientFlags`. Verified: LP64
`offsetof(centity_s, clientFlags) == 848`, `sizeof(centity_s) == 856`, and a
rerun of the sweep drops from 1357 hits to 1140 with `centity_s` gone from the
type histogram entirely.

The bit *positions* were left as written (`(cent->clientFlags >> 1) & 1` rather
than `cent->nextValid`). Those are mechanical to convert now that the word is
named, but converting them is a separate, reviewable change.

### `r_rendercmds.cpp` command-buffer reservations — 17 sites

Not found by the scan — this family is an x86 size baked into an *allocation*,
not into an access. Each `R_GetCommandBuffer(RC_..., n)` call passed its command
struct's literal **x86** `sizeof` as the byte count, then filled the struct
through real LP64 field offsets. Every command holding a pointer therefore wrote
past its own reservation and into the command that followed it in the list.

| command | reserved (x86) | LP64 `sizeof` | overrun |
| --- | ---: | ---: | ---: |
| `GfxCmdDrawEmblemLayer` | 104 | 120 | +16 |
| `GfxCmdDrawText2D` | 96 + len | 112 + len | +16 |
| `GfxCmdPCCopyImageGenMIP` | 16 | 32 | +16 |
| `GfxCmdStretchComposite` | 44 | 56 | +12 |
| `GfxCmdStretchPicRotateXY` | 52 | 64 | +12 |
| `GfxCmdStretchPicRotateST` | 52 | 64 | +12 |
| `GfxCmdDrawQuadList2D` | 12 + 80n | 24 + 80n | +12 |
| `GfxCmdStretchPic` | 48 | 56 | +8 |
| `GfxCmdDrawQuadPic` | 48 | 56 | +8 |
| `GfxCmdResolveComposite` | 8 | 16 | +8 |
| `GfxCmdDrawFramed2D` | 44 | 48 | +4 |
| `GfxCmdSetCustomConstant` | 24 | 24 | — |
| `GfxCmdSetScissor` | 24 | 24 | — |
| `GfxCmdClearScreen` | 28 | 28 | — |
| `GfxCmdProjectionSet` | 8 | 8 | — |
| `GfxCmdHeader` (`RC_END_OF_LIST`) | 4 | 4 | — |

`GfxCmdStretchPic` is the worst of these in practice, not the largest: it is
`RC_FIRST_NONCRITICAL`, emitted for essentially every 2D element the HUD and menus
draw, so the overrun repeats thousands of times per frame.

Fixed by reserving the real size. A helper,

```c
static int R_CmdBytes(size_t size)
{
    return (int)((size + 7) & ~(size_t)7);
}
```

replaces every literal with `R_CmdBytes(sizeof(T))`. The rounding matters
independently of the sizes: `R_GetCommandBuffer` only asserts `(bytes & 3) == 0`,
so a 28-byte `GfxCmdClearScreen` would leave the *next* command at a 4-mod-8
address and its pointer members misaligned. Rounding to 8 keeps every command's
pointers naturally aligned.

Producer and consumer stay in step for free: the back end walks the list by the
same value, `execState->cmd += *(unsigned __int16 *)execState->cmd`, which is the
`header->byteCount` this function writes.

The two variable-length commands keep their original slack semantics —
`sizeof(GfxCmdDrawText2D) + len` still covers `text[len] = 0` because `text[3]`
sits at the tail of the struct — and `GfxCmdDrawQuadList2D`'s payload stride is
now `4 * sizeof(GfxQuadVertex)` rather than the literal 80 (identical on both
ABIs; `GfxQuadVertex` holds no pointer).

### `rb_backend.cpp` — `RB_StretchPicCmdFlipST` — 10 sites

The consumer side of the command list. `RB_StretchPicCmdFlipST` read a
`GfxCmdStretchPic` back out through raw x86 word indices instead of the struct:

```c
RB_DrawStretchPicFlipST(
    *((const Material **)execState->cmd + 1),   // x86 byte 4;  LP64 material is at 8
    *((float *)execState->cmd + 2),             // x86 x;       LP64 the low half of material
    ...
    *((unsigned int *)execState->cmd + 11),     // x86 color
    GFX_PRIM_STATS_HUD);
```

On LP64 word 1 is the *low half* of `material`, and every field from there on is
shifted by one word. A wrong-values defect rather than corruption, and
independent of the reservation fix above: correcting the producer does not
correct a consumer that reads by the wrong offsets.

Fixed by modelling it on its twin, `RB_StretchPicCmd` (line 951), which already
went through `cmd->material`, `cmd->x`, …:

```c
    const GfxCmdStretchPic *cmd = (const GfxCmdStretchPic *)execState->cmd;

    RB_DrawStretchPicFlipST(cmd->material, cmd->x, cmd->y, cmd->w, cmd->h,
                            cmd->s0, cmd->t0, cmd->s1, cmd->t1,
                            cmd->color.packed, GFX_PRIM_STATS_HUD);
```

Word 4 was skipped in the original because `RB_DrawStretchPicFlipST` takes `w`
but not `w0`; the named version keeps that.

**This was the last live one on the rendering path.** Two further handlers in the
same file still read by x86 word index — `RB_DrawFullScreenColoredQuadCmd`
(line 1337, `material` at word 1) and `RB_StretchRawCmd` (line 1350, a
`const unsigned __int8 **` at word 7, x86 byte 28 vs LP64 byte 56) — but both are
unreachable: they sit in the dispatch table for `RC_DRAW_FULL_SCREEN_COLORED_QUAD`
(0x10) and `RC_STRETCH_RAW` (0xE), and no `R_GetCommandBuffer` call anywhere emits
either command id. Latent, not live. They are worth fixing if a producer is ever
added, and they are a good illustration of the census's blind spot: the base is
`execState->cmd`, typed `void *`, so `classify.py` can never rule on it.

One cosmetic leftover in `RB_StretchPicCmd:959`,
`**((unsigned int **)execState->cmd + 1)`, is ABI-correct for *finding* material
(the cast is pointer-strided, so it widens with the ABI) but then tests only the
low half of the name pointer for null. It picks the profiler's zone label, nothing
more.

### `r_material.cpp` — `Material_Duplicate` — 6 sites

The fallback every missing material goes through. `Material_RegisterHandle` ->
`Material_Register` -> `Material_MakeDefault` -> `Material_Duplicate(rgp.defaultMaterial,
name)`: on PC a material the zone does not contain prints
`WARNING: Could not find material '%s'` and draws as `$default`. On LP64 that
path handed the renderer a wild pointer instead.

| | |
| --- | --- |
| shape | `*((unsigned int *)mtlNew + 45 / 46 / 47)`, plus the same three as `unsigned __int8 **` |
| x86 | bytes 180 / 184 / 188 — `textureTable`, `localConstantTable`, `stateBitsTable` |
| LP64 | those tables are at 184 / 192 / 200; `sizeof(Material)` is 208, not 192 |
| cause | four 8-byte pointers at the tail of a 192-byte struct |

Each 32-bit store landed in the *previous* pointer:

```
+45 -> byte 180 = upper half of techniqueSet  (176..183)
+46 -> byte 184 = lower half of textureTable  (184..191)
+47 -> byte 188 = upper half of textureTable
```

So the duplicate came out with `techniqueSet` non-null — the low half was copied
from `$default` and looks right — and its top 32 bits overwritten by a truncated
`Material_Alloc` return. Non-null, so every `!techniqueSet` assert passes; the
first draw dereferences it.

The census found six sites here. Four more in the same twenty lines it could not
see, because they are struct *sizes* rather than indexed bases:

| line | was | is |
| --- | --- | --- |
| alloc size | `Material_Alloc(v3 + 193)` | `v3 + 1 + sizeof(Material)` (209) |
| copy length | `memcpy(mtlNew, mtlCopy, 0xC0u)` | `sizeof(Material)` — 0xC0 left the last 16 bytes uninitialised |
| name slot | `*(unsigned int *)mtlNew = (unsigned int)(mtlNew + 192)` | `mtl->info.name = (const char *)(mtlNew + sizeof(Material))` — truncated *and* aimed 16 bytes short, writing the name over `constantTable` and `stateBitsTable` |
| texture table size | `16 * mtlCopy->textureCount` | `sizeof(MaterialTextureDef)` — 16 on x86, **24** on LP64 |

Fixed by naming every field. Verified against the DWARF layout probe:
`sizeof(Material)` ilp32 192 / lp64 208, `MaterialTextureDef` 16 / 24,
`MaterialConstantDef` 32 / 32, `GfxStateBits` 8 / 8.

Confirmed the zone itself was innocent first: `$default` in
`code_post_gfx_mp.kbz` carries `techniqueSet` -> the registered `2d` techset
(technique index 4, `stateBitsEntry[4] == 0`), and all 446 materials in that zone
have a non-null technique set. The pointer was correct on disk and destroyed on
the first duplicate.

Two more in the same file, same class, fixed with it:

- `Material_UpdatePicmipSingle` read `textureCount` and `textureTable` as
  `header.xmodelPieces[14].name` and `[15].name[16 * i]` — `XModelPieces` is 12
  bytes on x86 and 24 on LP64, so neither offset survives, and the stride was the
  x86 `MaterialTextureDef`. Reached from `Material_UpdatePicmipAll` /
  `R_SetPicmip`.
- `R_GetMaterialList` addressed its output as `&data[8 * count + 4]`, which is
  `MaterialMemory` at 8 bytes behind a 4-byte counter. On LP64 that is 16 behind
  8. Its caller `R_MaterialList_f` relied on `inData` and `v6[4097]` being
  adjacent on the x86 stack; both now share a real `MaterialList` struct, the one
  the surviving assert string already named. Console command only, not on the
  draw path.

With those, `r_material.cpp` compiles clean without `-w` or `-fpermissive` and
has graduated into `NX_SANITIZED_SOURCES`.

## Open

### `flameGeneric_s + 23` — 24 sites

| | |
| --- | --- |
| x86 | byte 92 — `type` |
| LP64 | byte 92 is the upper half of `listLocal.prev`; `type` moved to 112 |
| files | `flame/flame_class_chunk.cpp` 151, 209, 210, 211; `flame_class_drips.cpp` 149; `flame_class_fire.cpp` 59, 106, 108; `flame_class_smoke.cpp` 60, 88, 90; `flame_class_stream.cpp` 220; `flame_physics.cpp` 636, 637, 639 |

Reads *and* writes, so this corrupts an intrusive list pointer. Smaller than
`centity_s` but the same shape, and the same fix applies: name the field.

### `GfxStaticModelDrawStream + 10` — 6 sites

| | |
| --- | --- |
| x86 | byte 40 — `viewInfoIndex` |
| LP64 | byte 40 is `usingCrossFade`; `viewInfoIndex` moved to 76 |
| files | `gfx_d3d/r_draw_staticmodel.cpp` 26, 92, 293, 400, 491, 639 |

Also on the renderer path.

### `weapVariantDef->szXAnims + 1 / +64 / +65` — 3 sites

`cgame/cg_weapons.cpp` 1086, 1088, 1115. The base is `const char **`, so the
index counts *pointers*: on x86 element N, on LP64 half of element N/2. Two of
the three feed a `Com_Error` format argument; the third
(`*((unsigned int *)…+ 1) && **((_BYTE **)… + 1)`) is a null check that reads
half a pointer.

### `worldModel + 1 / +2` — 7 sites

`cgame_mp/cg_ents_mp.cpp` 3885, 4003, 4260; `game/g_items.cpp` 819, 1503, 1505;
`game_mp/g_scr_main_mp.cpp` 1525. Base type `XModel **`, same halving problem:
the index selects the upper half of element 0 or the lower half of element 1.

### Needs a closer read — 4 sites

`classify.py` reports these as landing outside the struct on *both* ABIs, which
usually means the base is not really pointing at what its static type says.

- `LerpEntityStateCreateDynEnt`, `*((_QWORD *)x + 2)` — `cgame/cg_spawn.cpp:333`,
  `physics/destructible.cpp:1523`, `:2842`
- `spawner_ent_t`, `*((unsigned __int16 *)x + 4)` — `game/g_items.cpp:1574`

### The unjudged remainder

About 1100 further hits have a base the compiler typed as something other than a
struct in `layouts.cpp` — `char *`, `float *`, `long long int *` — or no reported
type at all (133 of them). Most are genuine scalar array indexing and perfectly
fine. They are grouped by file in [`../data/scalar_groups.txt`](../data/scalar_groups.txt)
(334 groups over 115 files) for reading by hand; adding a struct to
`layouts.cpp` moves any group with that base into `classify.py`'s reach.

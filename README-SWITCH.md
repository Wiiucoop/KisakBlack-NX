# CBO1-NX — Call of Duty: Black Ops on Nintendo Switch

A port of [KisakBlack](https://github.com/SwagSoftware/KisakBlack), the
reverse-engineered Black Ops multiplayer executable, to Horizon — built with
devkitPro and libnx, rendering through [Mesa for Nintendo
Switch](https://github.com/danfromtico/mesa-switch).

**The multiplayer menus run, are drawn by the game's own shaders, and can be
navigated with a controller.** No map loads yet, so it is not playable. This
file is the technical record — what works, how it is put together, what is
known to be wrong and what is missing — written to be enough to pick the work
up from cold. How to build it is in [README.md](README.md).

Requires your own copy of the retail PC game. No game data is in this
repository and none ever should be.

---

## 1. Where it got to

<!-- TODO: screenshot of the multiplayer main menu -> docs/images/main-menu.png -->
*(screenshot to come: `docs/images/main-menu.png`)*

- The engine **boots**: it reads its configuration, starts its worker threads,
  creates a Direct3D device, initialises the render targets, the static model
  cache and the particle buffers, and runs its main loop **indefinitely,
  without crashing**.
- **Every multiplayer boot zone that has been converted to KBZ loads** — the
  set the engine asks for is `code_pre_gfx_mp`, `code_post_gfx_mp`, `common_mp`,
  `ui_mp`, `patch_mp` and their localised `en_` variants — with their
  materials, technique sets, images, menus, fonts, sounds, models and effects
  registered into the asset pools.
- **The menus render with the engine's own shaders.** Every shader the menus
  load (2531 of them) translates to GLSL, every program the menus use compiles
  and links, and every menu draw runs the game's own vertex and pixel shaders —
  including the settings menu's blurred background.
- **The menus work with a controller**: the D-pad or left stick moves focus,
  the right stick drives a cursor (ZR / touch click), popups show, and Play
  opens offline. Private match still crashes (not yet looked at).
- **A multiplayer map loads.** With `+devmap mp_nuked` in `cmdline.txt` the
  engine loads every zone including the map's own (converted: GfxWorld,
  clipMap, GameWorldMp, ComWorld, destructibles, glass), shows the load
  screen, spawns the server, compiles the gametype scripts and runs
  `G_InitGame` into the level scripts. Each device run so far has moved
  the crash further down that path; the fixes are LP64 ones (section 4).

Hardware and driver, as the log reports them: Mesa 26.2.1, OpenGL 4.3 core,
renderer `NV12B` — Mesa's native nvc0 driver on the Tegra X1, not Zink.

### Controls

Face buttons are mapped by **position**, not label (`src/nx/nx_xinput.cpp`):
the game is written for an Xbox pad, so the Switch's bottom button is the
Xbox A. On-screen prompts keep their Xbox glyphs.

| Switch | Game |
| --- | --- |
| D-pad / left stick | move menu focus (remapped to the arrow keys the PC menus expect) |
| B (bottom) | confirm (Xbox A) |
| A (right) | back (Xbox B) |
| Y (left) / X (top) | Xbox X / Xbox Y |
| + / − | Start / Back |
| ZL / ZR | triggers (digital: fully released or fully pulled) |

---

## 2. How it is put together

### The fastfile problem, and KBZ

`.ff` fastfiles serialise structs with **four-byte pointers and x86 struct
sizes**. A 64-bit build cannot read them at all. This is the fundamental
obstacle of the whole port, and it is about word size, not about ARM.

The answer is to transcode offline:

- **`tools/ffconv/convert.cpp`** (PC host tool) reads a shipped `.ff` and
  writes a **KBZ1 "prelinked zone"**: every asset rewritten in native LP64
  layout, with every internal pointer flattened into a `(block, offset)`
  relocation table. Each transcoder mirrors the game's own `Load_<T>`
  (`src/database/db_load.cpp`) but writes LP64 fields and records relocations
  instead of fixing up in place. It covers 22 asset types — rawfile,
  stringtable, localize, techset, image, material, physpreset, physconstraints,
  lightdef, xglobals, xanim, xmodel, menufile, fx, weapon, impactfx,
  snddriverglobals, font, ddl, sound, sound patch, emblemset.
  - The hard part is **de-duplicated pointers**: a shipped string can be
    emitted once and referenced again by a 32-bit stream offset. The converter
    emulates the game's append-only block-4 memory cursor in lockstep and
    resolves those in a second pass. It self-checks: the final emulated cursor
    must equal `XFileHeader.blockSize[4]` exactly.
- **`tools/ffconv/kbzdump.cpp`** is the reference implementation of the
  on-device relocator, and the way to prove a zone before putting it on the SD
  card.
- **`src/nx/nx_kbz.cpp`** loads it on device in four steps: allocate blocks and
  copy their images, apply relocations, register each asset, then build the
  runtime objects the `.ff` loader would have built.
- **Script strings (KBZ version 2).** Bone names, notetracks, pathnode and
  dynent names, weapon hide tags are `u16` indices into the zone's own string
  list, which `Load_ScriptStringCustom` swaps for engine string ids. The KBZ
  carries that list and every slot holding one (`markScrStr` in the
  converter, one call per `Load_ScriptString` in `db_load.cpp`), and the
  loader remaps them before registration. Before this every such field named
  an arbitrary string (a pathnode asked for `animscripts/traverse/ground`).
  A new `Load_ScriptString` site in a transcoder needs a `markScrStr` too.

The `.ff` files must still be on the SD card: the zone loader opens the file
before the KBZ path takes over, and a zone with no `.kbz` is skipped
(`nx: no .kbz for zone '...'; skipping raw .ff load`), never loaded raw.

Two parts of `nx_kbz.cpp` are load-bearing and easy to get wrong again:

- **`AssetSlots`.** `DB_AddXAsset` *clones* the struct into a pool entry, so
  the header it returns is never the one it was given, and every `Load_<T>Ptr`
  on PC writes the returned pointer back through the slot it loaded from.
  Here those slots are relocations, so after each asset is registered every
  relocation that pointed at its block copy is rewritten to the pool entry.
  This cannot be deferred to one sweep at the end: a pointer inside an asset is
  frozen the moment that asset is cloned.
- **Step 4, the builders.** `db_load.cpp` does more than fill structs; it calls
  back into the subsystems to turn load defs into live device objects, and the
  KBZ path has to do the same. `kBuildSteps` is the table — one row per asset
  type — and it walks the **pool entries**, not the block copies, because a
  `GfxImage` keeps its texture inside the struct and the clone is a different
  copy of it. Currently:
  - `TECHNIQUE_SET` → `Load_CreateMaterialVertexShader`,
    `Load_CreateMaterialPixelShader`, `Load_BuildVertexDecl`;
  - `IMAGE` → `Load_Texture`, which turns the `GfxImageLoadDef` sitting in
    `GfxImage::texture` into a real texture;
  - `GFXWORLD` → `Load_VertexBuffer` for the world and layer vertex buffers.

  Still missing: `MATERIAL` → `Load_PicmipWater` (`db_load.cpp:2334`).

- **Map assets and `layout_gen.h`.** The six types a map adds (`gfx_map`,
  `col_map_mp`, `com_map`, `game_map_mp`, `destructibledef`, `glasses`) are
  too big to lay out by hand, so their offsets come from the compiler.
  `tools/ffconv/layout/layout.sh` (UCRT64 shell) parses the structs listed in
  `structs.txt` from the engine headers, compiles a probe of `offsetof` /
  `sizeof` twice with the Switch compiler — LP64, and `-mabi=ilp32`, which
  lays structs out as MSVC x86 did — and writes `tools/ffconv/layout_gen.h`:
  `X_`/`L_` offsets for every member and the pointer-free spans that copy
  across. A member that is not a pointer but changes size stops it (a union
  of pointers, say: list it as `opaque`). Regenerate after changing any
  listed struct. Two KBZ details came with it: output block 1 is the
  fastfile's runtime block (sized, never stored, zeroed on load), and offset
  pointers into the middle of an array resolve record by record.

Device resources are created **inline**, not queued.
`Sys_CanCreateDeviceResourcesInline()` returns true for every thread here,
because `code_pre_gfx_mp` is loaded before `R_InitThreads` has spawned the
thread that drains the `RB_Resource` queue — so anything that queued would
block in `RB_Resource_Flush` on a signal nobody can send.

See also [`docs/fastfile-stub-assets.md`](docs/fastfile-stub-assets.md) — an
asset whose name begins with `,` is a **stub**, a reference to an asset another
zone really owns, and only `DB_LinkXAssetEntry` knows to strip that comma.

### The renderer: Direct3D 9 over OpenGL

The engine talks Direct3D 9. `src/nx/nx_d3d9_null.cpp` implements the subset
it uses over EGL and an OpenGL 4.3 core context — the Switch does not
translate anything itself; homebrew cannot use NVN, so Mesa it is.

| D3D9 | how |
| --- | --- |
| render targets | every surface the engine draws into — render-target and depth textures, `CreateRenderTarget` / `CreateDepthStencilSurface` surfaces, **the back buffer too** — is a GL texture behind a cached framebuffer object, allocated empty once and never uploaded over |
| swap chain `Present` | blits the back buffer to the window, then `eglSwapBuffers` |
| `SetRenderTarget`, `SetDepthStencilSurface`, `StretchRect`, `ColorFill`, `GetRenderTargetData` | real (framebuffer binds, `glBlitFramebuffer`, scissored clears, `glReadPixels`) |
| `SetViewport`, `SetScissorRect`, scissor state | applied per draw |
| `Clear` | honours the viewport, its rectangles and the scissor, as D3D9 does |
| vertex / index buffers | `Lock`/`Unlock` write into real memory; the draw path uploads what changed |
| `CreateVertexShader` / `CreatePixelShader` | translated to GLSL ES 3.00 (below), compiled on first use, linked per pair |
| `Set{Vertex,Pixel}ShaderConstantF` | uploaded as the `vsc[]` / `psc[]` uniform arrays |
| `SetTexture` / `SetSamplerState` | textures on the unit of the same number, sampler state on one GL sampler object per slot; vertex texture slots (257+) on units 16+ |
| textures | DXT1/3/5 through `EXT_texture_compression_s3tc`, plus the uncompressed formats, with a swizzle that gives `A8`, `L8` and `A8L8` their D3D9 meaning |
| vertex declarations | every element feeds the attribute its usage maps to (`NX_ShaderAttribLocation`) |
| blend state, colour write mask, alpha test | applied (alpha test inside the pixel shader, `uAlphaTestFunc` / `uAlphaRef`) |
| `DrawIndexedPrimitive` | `glDrawElementsBaseVertex` |

**Orientation.** D3D9 numbers a target's rows from the top, GL from the
bottom. Every target is stored top row first — the vertex stage negates clip
y — so viewport and scissor rectangles, `StretchRect` and render targets read
back as textures all use D3D9's numbers unchanged. The one place the order is
undone is the blit to the window. The flip also reverses every triangle's
winding, which culling will have to account for.

**Shader translation.** `src/nx/nx_d3d9_shader.cpp` turns the engine's
shader-model 3 bytecode into GLSL ES 3.00 (which the GL 4.3 core context
accepts through ARB_ES3_compatibility). It is adapted from
[riicchhaarrd/KisakBlack](https://github.com/riicchhaarrd/KisakBlack/tree/web-port)'s
web port, `src/gfx_gl/gl_shader.cpp`, which already runs this engine's shaders
into maps through WebGL2; the changes for this backend are listed at the top of
the file. Among them: the vertex epilogue adds, after the D3D9 → GL depth fix
(`z = 2z − w`), **D3D9's half-pixel offset** (`nxHalfPixel`) and the **y
flip**. Anything that cannot run — no shader bound, no translation, a compile
or link failure — falls back to a single built-in program (vertex colour times
the pixel shader's first 2D sampler), so a translator gap degrades one draw,
never the frame.

**What is not applied yet:** depth test, stencil and culling are recorded but
deliberately ignored (nothing fills a depth buffer correctly until they all
land together); `DrawPrimitive` and `DrawPrimitiveUP` draw nothing; only
render target 0 of an MRT set is bound; sRGB reads and writes are ignored.

**The report.** Every 60 frames the driver prints a census to the log:
`[d3d census]` (call counts), `[nx-gl] geometry`, `textures` (formats, what
each frame sampled), `targets` (framebuffers, where the frame's draws went,
the current target, viewport and scissor) and `shaders` (translated,
compiled, linked, and how many draws ran the engine's shaders versus the
built-in program and why). Read it first.

### The `src/nx/` layer

The scaffolding is [NaGa](https://github.com/)'s: `cmake/switch.cmake` drives
the devkitPro build without touching the Windows path, and `src/nx/compat/`
supplies fake `windows.h`, `d3d9.h`, `d3dx9.h` and friends that are found ahead
of the real headers, so ~647,000 lines of engine compile nearly unmodified.
On top of that sit the platform pieces: `nx_main.cpp` (entry point, logging,
crash handler), `nx_wincompat.cpp` (Win32 API surface, threads, files),
`nx_winsock.cpp`, `nx_winuser.cpp`, `nx_xinput.cpp` (HID), `nx_snd_null.cpp`,
`nx_platform_stubs.cpp`, and the big ones, `nx_kbz.cpp`, `nx_d3d9_null.cpp`
and `nx_d3d9_shader.cpp`.

Input: the engine's own gamepad layer (`src/win32/win_gamepad.cpp`) reads the
controller through `nx_xinput.cpp`. The PC menus only move focus with the
keyboard arrows and the mouse wheel, so `UI_KeyEvent` (`src/ui/ui_main.cpp`)
hands the menus the arrow key each D-pad or stick direction means.

### The LP64 ratchet

The decompiled tree casts pointers through 32-bit ints in thousands of places.
On x86 that was lossless; on LP64 every one truncates — and `-w` was hiding all
of them. Measured across the Switch build: **3654 sites in 460 files**.

So `cmake/switch.cmake` keeps `NX_SANITIZED_SOURCES`. Everything *not* in that
list gets `-w -fpermissive -Wno-narrowing` applied **per source file**; a file
graduates by being added to the list, and from then on a pointer truncation is
a hard error. CMake prints the count at configure time. **Keep the list
additive — removing a file is a regression.**

Graduated so far: all of `src/nx/`, `com_expressions.cpp`,
`com_expressions_eval.cpp`, `r_material.cpp`, `r_rendercmds.cpp`,
`rb_backend.cpp`, `threads.cpp`.

Two lessons from doing it, both written up in the sweeps doc: the **error** is
usually the harmless one (narrowing a pointer to an `int` is ill-formed, so it
is fatal) and the **warning** is usually the crash (widening an `int` back to a
pointer is legal, so it only warns). And a wrong struct offset produces no
diagnostic at all.

The crashes found on device since have almost all been one of three shapes —
worth grepping for before the next one finds you:

| shape | example | fix |
| --- | --- | --- |
| a pointer read as a 32-bit word at a 4-byte stride | `*((unsigned int *)&rgp.poisonFXMaterial + n)` (the blur material) | index the array it meant |
| a pointer read through an overlapping `int` of a union | `*(const char **)(dvar->domain.integer.max + 4 * i)` (enum dvar strings, 5 sites) | use the union member that is the pointer |
| x86 struct sizes as literals | `Expression_Alloc(.., 16)`, `12 * numRpn` (menu expressions) | `sizeof` |

[`docs/lp64-sweeps/`](docs/lp64-sweeps/) is the standing census for the classes
the compiler *cannot* see, with the tooling that produces it: the same headers
compiled twice, once `-mabi=lp64` and once `-mabi=ilp32` as a stand-in for x86,
then compared field by field. Read
[`docs/lp64-sweeps/README.md`](docs/lp64-sweeps/README.md) before trusting a
clean sweep — it also documents what the sweeps are structurally blind to.

---

## 3. Debugging

### Logs

`stdout` and `stderr` go to **nxlink** if the title was launched from it, and
otherwise to `sdmc:/switch/kisakblack/kisakblack.log`. Both are unbuffered and
open the file in append mode — they used to share it through `dup2` with
separate write positions and overwrote each other, which is where stray binary
in older logs came from. `nxlink -s KisakBlack.nro` is the better loop while
debugging.

Assertions print an `ASSERTBEGIN` / `ASSERTEND` block with file and line before
the trap.

### Crashes

`nx_main.cpp` installs a CPU exception handler. On a fault it writes an
`[nx-crash]` block to the log — the exception kind, `pc`, `lr`, the fault
address, all registers, and the code addresses found on the stack, as offsets
into `KisakBlack.elf` — and exits the process. (It exits rather than handing
the exception back: `svcReturnFromException` is not allowed for this process,
and calling it looped the handler until the console froze.) Resolve the
offsets against the ELF from the **same** build:

```sh
$DEVKITPRO/devkitA64/bin/aarch64-none-elf-addr2line -f -C -i -e build-nx/KisakBlack.elf 0x52b344 0x533250 ...
```

The stack list is a scan, not a frame walk (`-O2` omits frame pointers): every
live return address is in it, with some stale ones mixed in. Read it top down.
An Atmosphère report in `sdmc:/atmosphere/crash_reports/` (a text `.log`)
resolves the same way: subtract nothing, the `KisakBlack + 0x...` offsets are
ELF addresses already.

A trap with an `ASSERTBEGIN` block right before it is an engine assert, not a
fault: `__debugbreak` is `__builtin_trap`, so read the assert first.

### Workflow

- **Boot straight into a map:** `sdmc:/switch/kisakblack/cmdline.txt` is
  appended to the command line; `+devmap mp_nuked` in it skips the menus.
  Remove it to get the menus back.
- **Zones:** `GAME=/f/pluto_t5_full_game sh tools/nx/convert-zones.sh` (UCRT64
  shell, after `tools/ffconv/build.sh`) converts every zone the port uses,
  `mp_nuked` included, into `<game>/kbz/` and validates each. The whole game
  folder then goes to the SD card as is.
- **LP64 checks:** `sh tools/nx/lp64check.sh <file.cpp>` and
  `sh tools/nx/lp64scan.sh <src subdirs>` (devkitPro shell, configured
  `build-nx/`) list pointer truncations the way a sanitised build would.
  A file that comes out clean can join `NX_SANITIZED_SOURCES`.
- **Push:** Git Credential Manager holds the GitHub login on the dev machine,
  so `git push origin switch-port` works non-interactively there.

---

## 4. Findings worth remembering

- **Menus are mouse-driven.** The D-pad only focuses items with focus scripts
  and a button only takes focus under the cursor, so the right stick drives a
  cursor in menus (ZR clicks, touchscreen points and taps; `NX_VirtualMouse`,
  `win_input.cpp`). The right stick is untouched in game.
- **Play / Theater / Operations open `error_netconnect_popmenu`**, the correct
  answer with no online services. OpenBLOPS (below) fakes those checks with
  `OPENBLOPS_OFFLINE_MENUS`; that is the way to Combat Training offline.
- **Popups drew as an empty blur** because the converter wrote
  `menuDef_t::visibleExp.rpn` 8 bytes late, over `showBits`: every menu with a
  `visible when` failed `Menu_IsVisible`. Converter offset bugs look like
  this -- a field that holds a pointer where a bitmask belongs.
- **`vid_restart` crashed in malloc** until level surfaces shared their
  texture's reference count, as in D3D9. Resets also restore device state.
- **One image, `tv_lookup`, still holds a load def** after a picmip reload;
  `Image_Release` names it and skips it. Not yet explained.
- **Diagnostics in the log:** `[nx-trace]` (one frame's draws in order, with
  target and state), `[nx-paint]` (why an open menu was not painted),
  `[nx-kbz] audit` (materials whose images are not textures),
  `R_NxCreateShaderLate` (a shader the KBZ build step missed, created at
  first use -- so far only `loadscreen_mp_nuked`'s, not yet explained),
  the frame report's `depth-tested / stencil / culled` counts, `[nx-crash]`.
- **Culling:** with the y flip, GL window coordinates equal D3D9 screen
  coordinates, but D3D9 (y down) calls positive area clockwise and GL
  calls it counter-clockwise. So `D3DCULL_CCW` is `GL_BACK`. Getting this
  backwards culled every menu quad (black screen with the menu "drawing").
- **Offline sign-in:** the Steam stub reports signed in while there is no
  DemonWare user, so XUID is 0 -- anything that looks the local player up
  in the player cache must check for that (`Dvar_InfoString` did not; devmap
  died building the connect string).

### LP64 bug classes met on the map path

Beyond the ones in "The LP64 ratchet", in the order the device found them.
The compiler flags the first three kinds (`tools/nx/lp64check.sh`); the rest
are silent and need reading:

- **Pointer arithmetic through `unsigned int`**, e.g. the hunk allocator's
  `(unsigned int)ptr & 0xFFFFF000` (`HUNK_PAGE_DOWN` now), or a function
  returning an address as `unsigned int` (`Hunk_AllocateTempMemoryHigh`).
- **Pointers stored in 32-bit slots**: script bytecode operands, the
  builtin function table (`func_table`), static-model draw streams
  (`R_PRIM_PTR_WORDS`), a VM stack buffer's entries.
- **Structs filled at x86 offsets** (`*((_DWORD *)p + 3)`, `stackValue + 13`,
  `(int)localFs.top + 12`): rewrite by field name.
- **Pointer arrays read as 32-bit words**: `*((unsigned int *)w->worldModel
  + 1)` is the *high half of element 0* on LP64, not element 1.
- **Whole-value copies spelled through a 4-byte member**:
  `a.u.intValue = b.u.intValue` copied a script value on x86; on LP64 the
  union holds 8-byte pointers. Silent -- grep for them.
- **Literal x86 sizes**: `MT_Alloc(112, ...)` for a struct with pointers,
  `Hunk_UserAlloc(..., 4 * size, ...)` for sval_u nodes, `2048` for 512
  pointers, `alloc(9)` / `alloc(strlen(name) + 10)` for a `fileData_s` header
  (corrupted the hunk's file list; crashed at the next hunk clear),
  `SV_LocateGameData(..., 760, ..., 10720)` (entity lookups by number landed
  mid-entity), `0x1DD8` for `vehicle_info_t`, `24 * i` over
  `XAnimClientNotify`, `5 * numModels` for a DObj's model list, `objBuf[3072][31]` (124-byte DObjs,
  so neighbours overlapped). Use
  `sizeof` / `offsetof`. Allocation sites were swept with
  `grep -rnE "(MT_Alloc|MT_Free|Hunk_[A-Za-z]*Alloc|Z_Malloc)([^;]*[0-9]{2,}"`
  and every struct-sized literal replaced (cgame, FX and server client memory,
  `XAnim_s`, the MT caches); their `*_SizeRequired` totals follow the same terms.
- **Script field tables with x86 offsets** (`{ "classname", 356, ... }`).
  Map each literal to its member with the layoutgen loose dump:
  `LAYOUTGEN_LOOSE=1 STRUCTS=tools/nx/fields_structs.txt OUT=fields_gen.h
  sh tools/ffconv/layout/layout.sh`, then `perl tools/nx/fieldmap.pl
  fields_gen.h gentity_s src/game_mp/g_spawn_mp.cpp`. Where a whole tail of
  a struct shifts by a constant (gclient_s +256 from `sess`, centity_s +4
  after `pose.actor`) a rebase macro is enough; otherwise `offsetof`.
- **Reads past the end of an array** that were harmless on x86 because of
  what happened to follow it (`itemTable[256]` in bg_unlockable_items) and
  are not on LP64.
- **Decompiler types**: ints typed as pointers (`EmitObject`'s classnum),
  unions whose meaning depends on another field (`cLeafBrushNode_s.data`).
- **Constants decompiled as addresses**: `(int)&objBuf[1758][2]` is
  `FL_OBSTACLE` (0x4000000) -- the value happened to fall inside objBuf in the
  PC binary. Wrong on every build; breaks outright once the array resizes.
- **Pointer arrays sized as `4 * n`**: `memset(cornerEntry, 0, 4 * n)` cleared
  half of each light-grid entry pointer (crash on `0xd00000000`), the file list
  was `unsigned int[]` copied back as `4 * n`, and `FS_ListFilteredFiles`
  allocated 16384 four-byte pointers. The compiler cannot see these.
- **Job queue command sizes**: every `jqWorkerCmd` carried its data size as an
  x86 literal (`r_dpvs_staticWorkerCmd = { ..., 12u, ...}`), so a command
  holding a pointer was queued with half of it. `sizeof` of the command the
  builder passes (r_workercmds.cpp, r_stream, r_foliage, r_water, fx_marks).
- **Fields reached through a neighbouring array**: the decompiler writes
  `scene.glassBrush[i].bmodel` as `*(GfxBrushModel **)&scene.glassBrushVisData[40 * i - 40932]`,
  the x86 distance back from the next member. Wrong once the element grows.
  Find with `grep -rnE "[[0-9]+ * w+ - [0-9]{3,}]" src`.
- **Pointers written through the wrong union member**: the decompiler picks
  any member at the right x86 offset, so `pose.fx.triggerTime = (int)&ci->control`
  is `pose.player.control`, `ent_update.handle = (int)playback` is
  `playback_free.playback`. Rewrite with the member that has the pointer type.
- **Pointers kept inside byte streams or int fields**: a chat icon's
  `Material *` inside the text (now a 4-byte slot, `R_TextIconHandle`), a map
  level shot in `int timeToBeat[31]` (side table).
- **Stack buffers sized for x86 structs**: `unsigned __int8 dst[9896]` then
  `memset(dst, 0, sizeof(playerState_s))` wrote 256 bytes past the array into
  the frame (first snapshot: the reader read garbage). `playerState_s` grew
  because `objective_t` holds a pointer. Candidates: a byte array cast to a
  struct or cleared with `sizeof(Struct)`.
- **Absolute x86 addresses**: `*(_BYTE *)(strlen(info6) + 67341897) = 0` is an
  inlined strcat into `info6`; `*(unsigned int *)(v * 16 + 172779900) = hi` is
  the high half of the element `LODWORD(...)` set on the line before. Find them
  with `grep -rnE "+ [0-9]{8,9})" src`; the online-only ones (live_meetplayer,
  sessions, ticker, fileshare, DW) are left.

**Physics is off on the Switch** (`nx_physics 0`): `Phys_ObjCreateAxis`
returns NULL, which every caller treats as "no physics" (dynents stay put).
The handles around it are LP64 now (`physObjId`, `physUserBody`,
`constraintHandle` are `intptr_t`, the generic AVL map keys `uintptr_t`), but
the solver in `physics_system_internal.cpp` still walks its free lists by x86
word index (`m_ptr_list[87]`, `+296`) and needs porting against the structs.

Still open, known: `actor_fields.cpp` and
`sentient_fields.cpp` still hold x86 offsets (mostly SP); word-stride
reads in `rb_backend.cpp` (render cmd), `fx_convert.cpp` / `fx_system.cpp`
(`anonymous + 59`), physics (~490 flagged sites); the script debugger
(`cscr_evaluate`, `cscr_debugger`).

## 5. Plan

**Renderer stages.** 1, render targets: done. 2, the engine's shaders
translated to GLSL: done (every menu draw). 3, depth, stencil, culling, depth
bias, fill mode: done (`nxGlApplyDepthStencil`; the y flip makes GL's winding
equal D3D9's, so `D3DCULL_CCW` is `GL_FRONT`). Not needed: MRT (only target 0
is ever set) and `DrawPrimitive(UP)` (only the RESZ hack, skipped without
INTZ). Still open: sRGB reads and writes. 4, performance: a program cache on
the SD card, per-draw constant uploads trimmed, batching (the web port solved
the same problems).

**Order.** Offline menus (done) -> map-zone converters (done: `mp_nuked`
converts and validates) -> renderer stage 3 (done) -> first 3D map via
`devmap`, fixing the LP64 crashes on that path -> SP / Zombies.

**Zombies goes through OpenBLOPS.** OpenBLOPS (GPL-3.0, no history available)
is the same KisakBlack tree with SP and Zombies built from it under `KISAK_SP`:
1344 of its 1372 engine/game files map one-to-one onto `src/<module>`, the SP
delta is ~700 `#ifdef KISAK_SP` lines in 145 files plus 28 new files, and it
is 32-bit x86 only. Bringing it in means a second Switch target with
`KISAK_SP`, an SP mode in the converter (SP numbers its asset types
differently and has `col_map_sp` / `game_map_sp`), and LP64 work on the SP-only
code. It waits for 3D: everything a Zombies map needs is shared with MP, and
the SP front end (`frontend.ff`) is itself a 3D scene.

## 6. Known problems

- **Settings only persist on a clean quit or an explicit apply.** Closing the
  title from the Home menu kills the process before the config is written.
- **Crash on forced exit.** The worker threads are never stopped, so tearing
  the title down from the home menu takes the process apart underneath them.
  Harmless in practice.
- **Renderer gaps** (section 2): no depth, stencil or culling yet; no
  `DrawPrimitive` / `DrawPrimitiveUP`; one render target of an MRT set; no
  sRGB.
- **`ui_viewer_mp` cannot be converted**: it contains a `ComWorld` (asset type
  13), map data the converter does not handle yet. The engine carries on
  without it.

---

## 7. What is missing to reach a map

1. ~~Convert the map zones~~ — done: `mp_nuked` and `en_mp_nuked` convert
   and validate, and `kBuildSteps` builds the world vertex buffers.
2. ~~Depth, stencil and culling in the renderer~~ — done (stage 3).
3. **Finish the LP64 work on the map path.** `rb_postfx.cpp`,
   `r_draw_staticmodel.cpp`, `r_add_staticmodel.cpp`, `r_light.cpp`, the hunk
   allocator (`com_memory.cpp`) and the script compiler and VM are done and
   graduated. The script port changed the bytecode: code positions are
   pointer-sized (`src/clientscript/cscr_codepos.h`, 12-byte switch-table
   entries) and a waiting thread's saved stack stores 1 + 8-byte entries
   (`SCR_STACKBUF_*` in `cscr_variable.h`); anything that walks either has to
   use those constants. A scan of `gfx_d3d`, `EffectsCore`, `qcommon`,
   `DynEntity`, `physics`, `glass` and `xanim` (the build's flags without
   `-w`, plus `-Wint-to-pointer-cast`, one file at a time) still flags ~490
   sites outside the `*_load_obj.cpp` BSP loaders, which fastfile maps never
   run. Most are in `src/physics/` (`physics_system_internal.cpp` 89,
   `phys_contact_manifold.cpp` 53, `phys_broad_phase.cpp` 37, `phys_main.cpp`
   36), where the decompiler addressed structs as arrays of free-list nodes at
   x86 offsets; fixing those means rewriting functions against the real
   structs. Physics runs when dynamic entities move, ragdolls spawn or
   destructibles break, not to load a map, so it waits for a crash to point at
   it. Separately, `src/gfx_d3d/r_water_sim.cpp:3336` and
   `src/EffectsCore/fx_beam.cpp:378` and `:1008` index past the end of a
   four-element `unitVec[0].array` to reach `unitVec[1]` — undefined
   everywhere, and on the map path.
4. ~~Boot straight into a map~~ — `cmdline.txt` (section 3, Workflow).
5. **Get through `G_InitGame` and into the first frame.** The loop is: run
   `+devmap mp_nuked`, read the crash or assert, fix it and its whole class
   (section 4, "LP64 bug classes"), rebuild. Fixed on this path so far, in
   order: the connect string's XUID lookup, a shader the KBZ build missed,
   hunk page arithmetic, the script compiler and VM, weapon model arrays,
   the unlockables table overrun, the script field tables and entity links,
   script strings in the KBZ (the map's traverse scripts), the hunk's
   `fileData_s` headers, the game entity and client sizes, DObj creation and storage, IK state buffers, struct-sized allocations (the client now reaches `CG_Init`), physics handles (physics off), the glass allocators. `CL_InitCGame` now completes; the main loop runs, parses snapshots, draws the first HUD and starts the first 3D frame. Then the 3D renderer meets its first world frame.
   After any converter change, re-convert **and re-copy the `kbz/` folder**.

---

## 8. Scope: this is the multiplayer executable

KisakBlack reimplements **only `BlackOpsMP.exe`** — as its own README and its
author's blog say. Campaign and Zombies live in `BlackOps.exe`, which is not
decompiled, and **no amount of work on this tree reaches them**. That is a
property of the upstream project, not of this port.

The nearest thing to a single-player experience is therefore **offline
multiplayer against bots** — Combat Training, `src/server_mp/sv_bot_mp.cpp`,
which is present in the tree but has never been exercised here.

Everything this port adds that is not Switch-specific — the fastfile converter,
the LP64 fixes, the asset-loading work — lives in the shared engine and would
apply just as well to a future single-player port, if `BlackOps.exe` is ever
decompiled.

The web port referenced above is a different trade-off: it builds 32-bit
(`-m32` on Linux, wasm32 in the browser), so it loads `.ff` files directly and
never meets the LP64 problems. The Switch runs 64-bit homebrew only, so its
renderer is what transfers here, not its engine-side work.

---

## 9. Credits

- [KisakBlack](https://github.com/SwagSoftware/KisakBlack) — the decompiled
  engine this is a port of.
- [NaGa](https://github.com/) — the original devkitPro scaffolding.
- [mesa-switch](https://github.com/danfromtico/mesa-switch) — EGL, OpenGL and
  Vulkan on the Tegra X1.
- [riicchhaarrd/KisakBlack](https://github.com/riicchhaarrd/KisakBlack/tree/web-port)
  — the web port, whose D3D9 → GLSL shader translator `nx_d3d9_shader.cpp` is
  adapted from (GPL-3.0).

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
- **The match runs.** The client connects, the world renders with the
  engine's shaders, the HUD and the team select menu show and the pre-match
  countdown runs, and class select leads to the first spawn. It is slow — most of the frame is spent in the
  Direct3D-over-GL layer per draw (section 2, "The report"), which is being
  profiled down.

Hardware and driver, as the log reports them: Mesa 26.2.1, OpenGL 4.3 core,
renderer `NV12B` — Mesa's native nvc0 driver on the Tegra X1, not Zink.

**Mesa 20.1 build.** `-DNX_MESA20_DIR=<package>/portlibs/switch` builds
against switch-mesa 20.1 (EGL + glapi + GLESv2 + libdrm_nouveau) instead,
leaving devkitPro alone; use a separate build directory (`build-nx-mesa20`).
20.1 has no libGL: GL comes from libGLESv2 through glapi, and the three
desktop-only calls (`glClearDepth`, `glDrawBuffer`, `glPolygonMode`) are
fetched with `eglGetProcAddress` (`src/nx/nx_gl_mesa20.cpp`). The NRO is
14 MB against 32 MB.

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
- **Effect references by name (KBZ version 3).** `FxEffectDefRef` (an
  element's `effectOnImpact`/`OnDeath`/`Emitted`/`Attached`, and effect-type
  visuals) is loaded as the effect's name and `Load_FxEffectDefFromName`
  swaps it for the `FxEffectDef`. The KBZ path skipped that swap, so the first
  impact effect followed a string as an effect (crash when firing). The
  converter records those slots (`markFxRef`) and the loader resolves them
  after registering the zone (`resolveFxRefs`). The stock loader resolves
  nothing else by name.

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
| vertex / index buffers | `Lock`/`Unlock` write into real memory and record the locked range; the draw path sends only the changed range (`glBufferSubData`), the whole buffer on first use or after `D3DLOCK_DISCARD` |
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

**The report.** With `+set nx_glreport 1` in `cmdline.txt`, every 60 frames the
driver prints a census to the log:
`[d3d census]` (call counts), `[nx-gl] geometry`, `textures` (formats, what
each frame sampled), `targets` (framebuffers, where the frame's draws went,
the current target, viewport and scissor) and `shaders` (translated,
compiled, linked, and how many draws ran the engine's shaders versus the
built-in program and why). It costs a visible hitch each time (about 60 SD
writes, and a `glGetError` around every draw of the frame before), which is
why it is off by default: it made the game stutter once a second.

Without it, a **summary** goes out every 600 frames: `[nx-gl] frames A..B`
with the average and the slowest frame (a hitch shows as a max far above the
average), MB of buffer data sent whole and partial, MB of buffer data sent whole and partial, then ms and calls per
present for each part of the GL work (draw total, `glDrawElements`, buffer
uploads, constants, texture uploads, pipeline, program lookup, target, blend
and depth state, `glGetError`, swap), and the buffers most often re-sent
whole with the reason. Time not in "draw total" or "swap" is the engine.

Performance findings so far, all in this layer:

- **Log volume is frame time.** stdout is unbuffered, so every line is an SD
  write. The report above, the heap report (every 30 s: `mallinfo` walks the
  heap) and `NET_GetPacket: WSAENETDOWN`, printed every frame with the console
  offline (now once), all showed up as hitches.

- **Whole-buffer re-uploads.** Every `Unlock` used to mark the whole buffer
  dirty and the next draw re-sent it with `glBufferData`. The engine appends
  to multi-MB dynamic buffers thousands of times a frame: ~2.5 s per frame in
  game. Fixed by tracking the locked range.
- **Diagnostics on every draw.** The geometry statistics (`nxFrameAccumulate`)
  transformed every index of every draw on the CPU: ~460 ms of a 566 ms frame.
  Off unless `NX_GL_GEOMETRY_STATS` is set; the geometry report's per-vertex
  numbers read zero without it.
- **`glGetError` on every draw.** Two per draw (to catch a failed draw) made
  the driver wait for its queue each time: ~630 ms of a 690 ms frame. Only
  the frame the report describes checks now. Keep `glGetError`, `glFinish`
  and any read-back off the per-draw path.
- **Constants re-sent every draw.** `vsc[]`/`psc[]` went up with every draw
  (~100 ms a frame after the above). Uniforms are per program, so each
  program now remembers the constant-file version it last received and skips
  the upload when nothing was written since. The program lookup keeps the
  last pair. The engine changes some constant before nearly every draw, so
  the bigger saving is size: the whole array up to the highest register a
  shader reads went up each time. The translator now records which registers
  each shader reads (`NxShaderInfo::constMask`) and only those go up, as up
  to 16 contiguous runs per stage.
- **Partial buffer uploads waited on the GPU.** `glBufferSubData` into a
  buffer a pending draw reads makes the driver wait or stage a copy (~60 µs
  each). A dynamic buffer whose locks since the last upload all carried
  `D3DLOCK_NOOVERWRITE` now goes up through an unsynchronized
  `glMapBufferRange` — the promise D3D9 makes, which GL can express.

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

A fault address that looks like a real one with its top bits missing
(`0x34034790` when the zones load at `0xa3…`) is a pointer that passed
through 32 bits somewhere: look for an int in its path (section 4).

### Memory

`[nx-mem]` lines in the log (`src/nx/nx_wincompat.cpp`):

- **every 5 s** (`Com_Frame`): free heap, malloc's arena and in-use bytes,
  its top chunk, what the `VirtualAlloc` regions hold, and the heap break
  against the heap's size. A steady rise is a leak; a step is one allocation.
- **every heap growth of 8 MB or more** and **every refused one**
  (`--wrap=_sbrk_r`), with the caller.
- **every `VirtualAlloc` of 16 MB or more**, and a report when one fails.

**Guard pages.** `malloc`/`calloc`/`realloc`/`free` are wrapped: blocks of
256 KB or more, and every `VirtualAlloc` region, end against a page with no
access (`svcSetMemoryPermission`). An overrun then faults at the writing
instruction, with the fault address exactly on a page boundary just past the
block, instead of corrupting malloc's bookkeeping and failing later somewhere
unrelated (malloc refusing small requests with gigabytes free, `free()`
faulting inside Mesa). At most 1500 guards live at once — each splits the
kernel's memory map. Smaller blocks are not guarded.

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
- **Byte offsets between records in zone data**: `GfxAabbTree::childrenOffset`
  is the distance in bytes from a node to its first child, counted in 40-byte
  x86 nodes. The native node is 48 bytes, so the offset landed inside the
  wrong node and the tree walk recursed until the stack ran out. The converter
  rescales it (`tGfxCellRefs`). Other `*Offset` fields in zone structs are
  either same-size strides (`mnode_t`, `DObjSkelMat`) or not links.
- **Surface ids stepped by x86 record sizes**: a draw surf names its record
  in `surfsBuffer` as a count of 4-byte units, and the builders advanced it by
  the x86 size (`surfId += 5` for `BModelSurface`, `+= 14` for
  `GfxModelRigidSurface`). Every surface after the first pointed into the
  middle of the previous one. The backend also read `gfxEntIndex` at its x86
  offset (`surfsBuffer[v + 14]`). Now `sizeof(T) / 4` and the field.
  The skinned walkers in `r_scene.cpp` had it as `surfSize = 56` / `24`
  (rigid / skinned): the first spawned player drew from garbage.
- **Heap overruns from literal allocation sizes**: `debug_brush_info` was
  allocated at the x86 490012 bytes and cleared with `sizeof`, 40 KB past the
  block, which broke malloc's bookkeeping. malloc then refused small requests
  with 1.7 GB free and `free()` faulted inside Mesa. Found with guard pages:
  blocks of 256 KB or more (malloc and VirtualAlloc) end against a no-access
  page, so an overrun faults at the writer (`nx_wincompat.cpp`,
  `--wrap=malloc`). Candidates: `grep -rnE "Alloc[A-Za-z]*\((0x[0-9A-F]{3,}|[0-9]{4,})"`.
- **`N * count` with an x86 element size**: `R_ClearScene` cleared the scene
  model/DObj/brush/glass arrays with 76/132/44/40 per element, so stale `obj`
  pointers survived into the next frame. The same form sized the client state
  hunk (`clientActive_t`, `clientConnection_t`), the script parser lookups and
  collmap geoms. Sweep: `grep -rnE "mem(set|cpy|move)\([^;]*, [0-9]{2,} \* "`,
  then compare each literal with the native size by compiling a probe
  (`template <size_t N, int X> struct Show; Show<sizeof(T), X> s;` with
  `-fsyntax-only` and the project's flags; the error prints both).
- **Tables of x86 field offsets**: `g_animRateOffsets` (cg_weapons.cpp) held
  the x86 byte offsets of each weapon anim's time field in `WeaponDef` /
  `WeaponVariantDef` (948 = `iRechamberTime`…), so the first weapon anim read
  a float as its duration (assert `time >= 0`, time -1073741824). Now
  `offsetof`. To name x86 offsets, run the layout generator on the structs
  alone: `STRUCTS=<file with "include bgame/bg_weapons_def.h", "WeaponDef">
  LAYOUTGEN_LOOSE=1 OUT=<tmp.h> sh tools/ffconv/layout/layout.sh`, then read
  the `X_Struct__field = N` lines. Script field tables (`g_client_fields`,
  vehicles) go through `GCLIENT_X86` / `VEHICLE_X86`; `hudelem_s` holds no
  pointers; the flame table's floats precede its pointers.
- **Allocators doing pointer arithmetic in `int`**: the physics transient
  allocator aligned, bumped and returned its pointers as `int`
  (`~(align - 1) & (int)&cur[align - 1]`), so every allocation came back cut
  to 32 bits — the first player spawn crashed in the GJK query that player
  movement runs. Physics being off does not keep `src/physics/` off the
  map path: `Pmove` collides through it. The slot pool under it
  (`phys_mem_new.cpp`) reserved the x86 8 bytes for each slot's owner record
  (16 here, so it overlapped the caller's data), compared owners with 32-bit
  compare-and-swaps and packed a 32-bit pointer and a tag into one 64-bit
  free-list head; it now sizes the record by `sizeof`, uses full-width
  atomics and keeps the free list under a spinlock. Pointers used only as hash keys
  (`bpei_database_id`, `get_ent_info((unsigned int)ent)`) are left at 32 bits.
- **Pointers passed to varargs as ints**: `DDL_MoveTo(&s, &s, 2,
  op0.internals.intVal, op1.internals.intVal)` handed the low halves of two
  string pointers to a function that reads them with `va_arg(args, const
  char *)`. Varargs take anything, so the compiler says nothing. Look at
  every `.intVal` (or other int) in a call to a variadic function whose
  receiver expects a pointer.
- **Unions copied through their `int` member**: menu expression operands
  hold an int, float or string in `operandInternalDataUnion`, which has an
  `operator int()`. The comma operator copied each argument as
  `v.intVal = (int)op.internals`, so every string argument to a multi-argument
  menu function kept only its low 32 bits (crash selecting a team). Copy the
  union whole; its constructors now clear all 8 bytes and the conversion
  operators are `explicit`. Search: `grep -rn "(int)[^;]*\.internals;"`.
- **Pool indices from x86 entry sizes**: freeing an FX element computed its
  slot as `offset / 48`; the pool entry is 64 bytes here (`FxElem` holds a
  pointer), so the wrong slot went on the free list and the next alloc read
  garbage. Now `sizeof`. The effect handle helpers also disagreed with
  `FX_EffectFromHandle` (48 vs the container's 52 per index, an x86 bug too).
- **Size functions returning x86 sizes**: `XAnimTreeSize()` returned 8, so
  every anim tree was allocated and cleared at half its size; `children` was
  whatever followed. Now `sizeof(XAnimTree_s)`. The decompiler also reused
  that function wherever a constant 8 was needed (the QoS payload size),
  which keeps its literal.
- **Int arrays handled as `void *`**: `importance_merge_sort` sorted the
  `int` image-index list as `void **`, walking twice the array. Typed as
  `int *`. The other hand-written merge sorts hold real pointers.
- **Pointers written through the wrong union member**: the decompiler picks
  any member at the right x86 offset, so `pose.fx.triggerTime = (int)&ci->control`
  is `pose.player.control`, `ent_update.handle = (int)playback` is
  `playback_free.playback`. Rewrite with the member that has the pointer type.
  The destructible event queue (`destructible_event_t`) wrote its damage
  event's `self`/`attacker` as `ehe.localClientNum`/`ehe.event` ints and its
  radius event through `ed` at `erd`'s x86 offsets: crash shooting a car.
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

**The OpenBLOPS merge (branch `sp-merge`).** OpenBLOPS forked upstream
KisakBlack at `2a778526` (2 Aug 2026; closest on every sample, later commits
drift away), and this repository carries that commit, so the merge is a real
three-way one: base `2a778526`, theirs OpenBLOPS, ours `switch-port`. 190
files changed there (22k lines, 140 of them with `KISAK_SP` code); 174 merged
cleanly and 27 hunks in 16 files were resolved by hand -- keeping our LP64 fixes
(`sizeof` instead of x86 literals, `GCLIENT_X86`, `clientFlags` by field) with
their additions, and taking theirs where they fixed the same thing properly
(`mapInfo.levelShot`, `displayServers`, `deathContents`, offline stats). New
files: `g_sp_crosshair`, `g_sp_lookat_nodes`, `bg_actor_constants`,
`bg_sp_anim_snapshot` (+ `cg_sp_anim_snapshot.inl`), `msg_origin_quantization`,
`live_stats_layout`, `sv_offline_stats`, `ui_custom_bots_mp`. Their x86 layout
`static_assert`s are fenced with `#ifndef KISAK_NX`; the sentient field table
now uses `offsetof` (it had raw x86 offsets into a struct with pointers).

**The SP build.** `cmake -S . -B build-nx-sp -DNX_SP=ON` builds the same
sources with `KISAK_SP` for `KISAK_MP` (NRO title "KisakBlack SP").

**SP zones.** No separate converter mode was needed: SP and MP share one
`XAssetType` numbering and the same asset struct layouts. What SP zones add is
three asset types MP maps never carry, all on existing transcoders --
`GAMEWORLD_SP` (14, the same `{name; PathData}` as `GAMEWORLD_MP`) and
`CLIPMAP` (11, the same `clipMap_t` loader as `CLIPMAP_PVS`) -- and string tail
sharing: a localize entry can point into the middle of another string, so
inline strings are recorded as byte ranges (`emitInlineStr`). `convert-zones.sh`
converts the Zombies set after the MP one (`code_pre_gfx`, `code_post_gfx`,
`common`, `patch`, `frontend` + `_patch`, `common_zombie` + `_patch`,
`zombie_theater` + `_patch`, and their `en_` zones); all validate. Boot with
`+devmap zombie_theater`, skipping the frontend.

**Running SP.** Copy `build-nx-sp/KisakBlack.nro` as `KisakBlack-SP.nro`; both
NROs use the same game folder, `sdmc:/switch/kisakblack/`, and the same
`cmdline.txt` -- the one on the SD card, which the log's first lines echo
(`command line: ...`). For Zombies: `+devmap zombie_theater`. The engine
loads `code_pre_gfx`, `code_post_gfx`, `patch` (+ `en_`) at boot, then
`common_zombie_patch`, `en_common_zombie`, `common_zombie`,
`en_zombie_theater`, `zombie_theater` for the map (not `common`).

**SP findings so far:**

- **Load cinematics.** SP opens `<map>_load` with Bink for every map. The
  Bink stub (`nx_bink_stubs.cpp`) must report no error: the engine checks
  `BinkGetError` before each Bink call and asserts on anything non-empty.
  `BinkOpen` failing is what makes it skip the video (black load screen with
  "NOT USING CINEMATIC_SUBTITLES").
- **Commands before the client exists.** SP's map-start scripts send
  `cmd mlvl ...` (menu open/close) before the client state is allocated;
  `CL_ForwardToServer_f` now reports "not connected" instead of asserting.
- **Clip map pool.** `Load_ClipMapAsset` registers every clip map as
  `CLIPMAP_PVS` whatever the zone numbers it; SP zones number `col_map_sp`
  `CLIPMAP` (11). The KBZ loader now does the same remap, or
  `CM_LoadMapData_FastFile` waits 33 s and errors "Couldn't find the bsp for
  this map" (the stuck popup). The game worlds need nothing: pools 14 and 15
  alias the same storage.
- The SP `ui/menus.txt` and some lobby materials are not in the zones loaded
  without the frontend: harmless for `+devmap`.

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
- **In game it is slow** (section 2, "The report"); being profiled down.
- **Physics is off** (`nx_physics 0`): the solver is still at x86 offsets.
- **`r_water_sim.cpp` is not LP64-clean** (dozens of pointer/int casts); maps
  with dynamic water will break. `mp_nuked` has none.
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
   Since then: the AABB tree child offsets (converter), surface ids and
   record strides in the scene buffer, anim tree size, the FX element pool,
   the scene clears, the client state hunk, the IK DObj reads, a heap overrun
   by the physics debug buffers (found with guard pages), and the menu
   expression operands. **The match now runs** to the team select and
   countdown; next is frame time, then whatever the match itself hits.
6. **Zombies (branch `sp-merge`).** OpenBLOPS merged (section 5), the SP
   build (`-DNX_SP=ON`) boots, the Zombies zones convert and load, and
   `+devmap zombie_theater` reaches `CM_LoadMap`. Next: the SP game code on
   the map path (actors, pathnodes, zombie scripts), the same LP64 loop as MP.
   After any converter change, re-convert **and re-copy the `kbz/` folder**.

---

## 8. Scope: this is the multiplayer executable

KisakBlack itself reimplements only `BlackOpsMP.exe`. Campaign and Zombies live
in `BlackOps.exe`; they come from **OpenBLOPS**, which builds SP and Zombies
from the same tree under `KISAK_SP`, merged here (below).

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

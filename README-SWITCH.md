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

**Zombies (SP build, `KisakBlack.nro`) -- state as of 2026-10-02.** Kino der
Toten (`zombie_theater`) boots every time, loads, and is playable through the
rounds: menus with the controller (A/B in the Nintendo layout), the coop load
screen with a progress bar, sound (PCM, ADPCM, WMA through FFmpeg, streams),
power, perks, power-ups, the mystery box. Handheld at 460.8 MHz and 540p it
runs at roughly 19-30 ms a frame depending on the area (~33-50 fps), with the
GPU at 12-19 ms; the multiplayer build is `KisakBlack-MP.nro`.

**Defaults that matter**, all overridable in `cmdline.txt` (a map command in
it is moved to the end of the line, so `+set` lines always apply first):

| Setting | Default | Why |
|---|---|---|
| `r_mode` | 960x540 handheld, 1280x720 docked (chosen at boot) | GPU-bound in handheld; the present blit upscales |
| `nx_gpuclock` | 460 (MHz, handheld only; 0 = leave the system's) | GPU-bound; only replaces the 307.2 MHz default, so sys-clk wins |
| `r_picmip` / `_bump` / `_spec` | 2 | 3 too blurry with no streaming; 1 brought driver stalls |
| `r_stream` | 0 | high-mip streaming was the stutter |
| `nx_bloom` | 0 | bloom drew glows offset from their lights |
| `nx_vsync` | 0 | the refresh wait rounded frames up to 33/50 ms |
| `nx_glflush` | 64 | flush every N draws so CPU and GPU overlap |
| `nx_splog` | 0 | SP bring-up traces off the log |
| `nx_switchglyphs` | 1 | button prompts show A/B and X/Y as the Switch labels them (visual only) |
| `nx_aimassist` | 1 | console gamepad aim assist: slowdown over targets, lock-on (`aim_lockon_enabled` 1), ADS snap (`aim_autoaim_enabled` 1); 0 = off as on PC |
| shadows, depth prepass, DoF, distortion, flame, marks, brass | off | the low preset (`nx_main.cpp`) |

**Shader warm-up.** Every shader pair ever built is listed in
`shadercache/pairs.txt` and built while the loading screen is up; a session
after the first builds none in game. Binaries go to `shadercache/*.bin` (Mesa
still runs its back end on load, ~18 ms each, which is why the warm-up
matters). `pairs.txt` is build-independent and can ship in the NRO.

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
Xbox A. On-screen prompts keep their Xbox glyphs. Menus swap the pair back to the Nintendo
convention (`NX_UI_GamepadDirectionToArrow`, `ui_main.cpp`): A confirms, B goes
back.

| Switch | Game |
| --- | --- |
| D-pad / left stick | move menu focus (remapped to the arrow keys the PC menus expect) |
| A (right) | menus: confirm. In game: the Xbox B binding |
| B (bottom) | menus: back. In game: the Xbox A binding (jump) |
| Y (left) / X (top) | Xbox X / Xbox Y |
| + / − | Start / Back |
| ZL / ZR | triggers (digital: fully released or fully pulled) |

The gamepad is always on (`gpad_enabled` is forced true in `Dvar_SetVariant`
and registered on): with it off the engine drops every pad key in
`CL_KeyEvent` and binds nothing. The pad bindings (the profile's button and
stick configs, else `buttons_default` / `thumbstick_default`) are executed at
every `CL_InitCGame`, since a map started from the command line never passes
through the profile sign-in or options menu that normally runs them.

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
| `Set{Vertex,Pixel}ShaderConstantF` | kept as register files; a draw whose stage file changed copies the rows its shader reads into a persistent-mapped uniform buffer, bound to the `NxVsConst` / `NxPsConst` std140 blocks |
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
`nx_winsock.cpp`, `nx_winuser.cpp`, `nx_xinput.cpp` (HID), `nx_snd.cpp`,
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
otherwise to `sdmc:/switch/kisakblack/kisakblack.log`. They used to share the
file through `dup2` with separate write positions and overwrote each other,
which is where stray binary in older logs came from. `nxlink -s
KisakBlack.nro` is the better loop while debugging.

The log is **asynchronous** (`nx_main.cpp`, "Asynchronous log"): descriptors 1
and 2 get a device (`devoptab_list[STD_OUT/STD_ERR]`, the way libnx routes
them to nxlink) whose writes go into a 4 MB ring, and a low-priority thread
writes the ring to the file. Writing on the printing thread made every
600-frame summary a 140-190 ms hitch. A full ring makes the printer wait; the
crash handler writes out what the ring still holds before its report, after
stopping the thread and letting a write it has in progress finish (otherwise
both wrote the same bytes and the thread ran past the end, rewriting the ring
until the process died).

Assertions print an `ASSERTBEGIN` / `ASSERTEND` block with file and line before
the trap.

**CPU profile.** `PROF_SCOPED` (`universal/profile.h`) was Tracy-only; on NX
each of its ~600 sites now adds ticks and calls to a per-thread slot
(`src/nx/nx_prof.cpp`), and every 600 main-thread frames the log gets
`[nx-prof]` lines: the 40 costliest scopes per frame, with thread and calls
per frame. Times are inclusive (a scope contains the scopes nested in it), so
read them as a tree, not a sum. Measure at stock clocks.

**SP traces.** The SP bring-up prints (lines starting `SP `: anim commands,
sound notifies, actor/mover/view probes) are dropped unless `nx_splog 1`;
each was a write through to the SD card. Warnings and errors still print.

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
- **NRO names and icons:** SP/Zombies (`build-nx-sp/`) builds
  `KisakBlack.nro`, multiplayer (`build-nx/`) builds `KisakBlack-MP.nro`; the
  same string is the title the homebrew menu shows. Both live in
  `cmake/switch.cmake`, in the `if(NX_SP)` block: `NX_NRO_NAME` (file name and
  title) and `NX_NRO_ICON`. The author and version are in the
  `nx_generate_nacp` call further down. The icons are
  `switch-meta/icon-zm.jpg` and `switch-meta/icon-mp.jpg` (libnx's default
  icon for now): replace them with **256x256 baseline JPEGs** (not
  progressive; hbmenu shows nothing for a progressive one) and rebuild.
  After changing a name, re-run `cmake build-nx-sp` / `cmake build-nx`. To
  restamp an NRO without rebuilding, devkitPro's tools work on the ELF:
  `nacptool --create "<title>" "<author>" "<version>" out.nacp`, then
  `elf2nro KisakBlack.elf out.nro --icon=icon.jpg --nacp=out.nacp`.

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

What that costs until it is ported, and the stand-ins:

- **No ragdolls.** `Ragdoll_CreateRagdollForDObj` creates none while
  `nx_physics` is off (a ragdoll is solver rigid bodies and joints).
- **HACK: Zombies corpses vanish when their death animation ends.** SP turns
  the dead actor into a corpse whose client anim tree starts empty; in retail
  the ragdoll takes the bones over at that moment, and without one the corpse
  stood in its bind pose (T-pose). `CG_ActorCorpse` (`cg_actors_mp.cpp`)
  therefore does not draw actor corpses while `nx_physics` is off, and frees
  their client DObj (left alive, it shared its actor slot's anim tree with the
  next zombie in that slot and doubled its animation speed). The server
  entity stays (scripts keep their reference, the corpse limit clears it).
  Copying the server's corpse tree into the client one was tried and crashed:
  the tree carries server script strings into client notetrack notifies.
  Remove the hack once ragdolls work.

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

**Running SP.** SP/Zombies is `build-nx-sp/KisakBlack.nro`, multiplayer
`build-nx/KisakBlack-MP.nro` (until 2026-10-01 they were `KisakBlack-SP.nro`
and `KisakBlack.nro`); both NROs use the same game folder, `sdmc:/switch/kisakblack/`, and the same
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
- **`devmap` turns on developer, and developer turns on the script
  debugger.** `Scr_EndLoadScripts` runs `Scr_InitDebugger` and archives the
  canonical strings whenever `gScrVarPub.developer` is set, which `+map` in
  MP never did. That code used x86 sizes: `ArchivedCanonicalStringInfo` (a
  `ushort` plus a pointer, 16 bytes here) allocated and sorted at 8 bytes a
  record, so `qsort` handed `strcmp` half-records (crash in `strcmp`, far
  0x120). Fixed with `sizeof` there and on every debugger allocation reached
  without the debugger window (the 294910-pointer breakpoint table, assignment
  list, script windows, watch nodes, and the child sort, which stored
  pointers in ints and read fields at x86 offsets 48/72/76). The
  watch-expression evaluator in `cscr_evaluate.cpp` still has int pointer
  casts; only the PC debugger window reaches it.
- **Script developer mode is always on in SP**, `map` or `devmap`: the merge
  forces `gScrVarPub.developer` in `Scr_Settings` (`cscr_vm.cpp`) so compile
  errors name the file and line. That makes SP the first real user of the
  debug hunk (`g_DebugHunkUser`, first-fit), which exposed an allocator bug:
  the owner back-pointer sits in the 8 bytes before the user pointer, and
  with a 16-byte header it landed on the block node's `size`. Every block's
  size became address bits, frees merged garbage-sized blocks, and later
  allocations overlapped (crash in `Hunk_FirstFitFree` growing the client
  script source table). The header is now node + pointer (24), sizes are
  rounded to 8, the hunk is 32 MB on NX (23 on x86; the tables hold
  pointers), and running out is a named fatal error instead of a write to 0.
- **Ragdoll handles were pointers in ints** (`cpose_t::ragdollHandle`,
  `killcamRagdollHandle`, tested with `> 0`). The first zombie kill that
  reached ragdoll creation crashed on the truncated pointer. On NX a handle is
  now a slot in a 32-entry table (`Ragdoll_BodyHandle` / `Ragdoll_HandleBody`),
  and no ragdoll is created while `nx_physics` is off: a ragdoll is solver
  rigid bodies and joints, which are not LP64-ported. Corpses keep their
  death-animation pose.
- **In-game performance (Zombies, first look):** about 112 ms per frame.
  Per frame: ~1330 partial buffer uploads (41 ms; ~450 bytes each, nearly one
  per draw), 1560 draws (30 ms), program binds (14 ms), whole uploads (13 ms),
  constants (11 ms). The next step is persistent-mapped dynamic buffers, so
  locks write into GPU memory and no draw uploads anything.
- **Low graphics preset (both builds).** `nx_main.cpp` puts `+set` lines on
  the built-in command line: picmip 3 for color, normal and specular maps, no
  anisotropy or AA, no shadow maps, no depth prepass, no DOF, distortion,
  flame effect, bullet marks or brass. `Com_StartupVariable` applies them after
  the saved config and before the renderer starts; `cmdline.txt` comes after
  and can override any of them. Second Zombies run before it: 136-141 ms per
  frame, of which partial uploads 71 ms (2344 per frame), draws 39 ms (2883),
  whole uploads 17, programs 16, constants 13 -- nearly all of it in the GL
  layer.
- **With the preset:** 48-72 ms per frame in Zombies (from ~140), 370-930
  draws. Uploads were then two thirds of the frame: partial 35 ms (712 per
  frame, ~50 us each), whole 11 ms (the 8 MB vertex and 2 MB index rings
  re-sent whole on every `D3DLOCK_DISCARD`).
- **Persistent-mapped dynamic buffers** (`nxGlSyncRing`, needs
  `ARB_buffer_storage`, which Mesa 26 has on the 4.3 core context). A dynamic
  buffer is up to 3 GL buffers with immutable storage mapped write +
  persistent + coherent; an unlock `memcpy`s its bytes into the mapping and
  makes no GL call. `NOOVERWRITE` writes into the slot in use; `DISCARD`
  moves to the next slot after waiting on its fence and copies only the bytes
  written since; a lock with neither flag waits for the GPU. The frame
  summary counts fence waits that found the GPU busy ("ring waits"). Without
  the extension it falls back to the upload path.
- **First result: no gain.** The coherent mapping is uncached on this driver:
  a 4 KB `memcpy` into it took ~53 us (~80 MB/s), the same as the
  `glBufferSubData` it replaced, so the frame stayed at 48-63 ms even though
  whole-buffer traffic fell from ~7.5 GB to ~1.3 GB per 600 frames. The mapping
  flags are now chosen at startup (`nxGlPickRingMode`): 4 MB written in 4 KB
  appends into a buffer made each way (coherent, explicit flush, each with and
  without `GL_CLIENT_STORAGE_BIT`), timed against `glBufferSubData`, fastest
  kept, all printed as `[nx-gl] buffer write speed`. Buffers first uploaded
  before the pick move onto the ring.
- **Second result:** the pick chose explicit flush with client storage (4 KB
  append: glBufferSubData 20.1 us, coherent 4.0, explicit flush 1.8). Partial
  uploads fell from 35 ms to ~2 ms a frame. The frame (42-55 ms) then spent
  21-27 ms in constants: uniform data goes into the command stream, and every
  program re-sent every register it reads whenever any constant changed.
- **Constants now go up by change.** Each register keeps the version at which
  it last took a different value (an equal write changes nothing); a program
  sends, per run, only the span changed since it last drew.
- **Shader builds are in the frame summary** ("N programs built, X ms,
  slowest Y") so hitches can be matched to first-use compiles, and startup
  prints the number of program binary formats: a shader cache shipped with the
  NRO would rest on `glGetProgramBinary`, and the Mesa in the NRO is the same
  for every user.
- **Third result:** program binary formats: 1. The stutters are shader builds:
  179 programs in the first gameplay minute took 5.5 s (slowest 759 ms), then
  34 (0.9 s), then 9. Constants stayed ~27 ms per ~1000 draws after the
  diffing while `glDrawElements` itself showed 0.3 ms a frame: the driver
  queues the real work and the call that finds the queue full waits, so this is
  the driver's per-draw cost showing up in the uniform uploads. `glUseProgram`
  now has its own profile row to check that.
- **Program binary cache** (`shadercache/<key>.bin` in the game folder). Every
  program linked is saved with `glGetProgramBinary`; later runs load it with
  `glProgramBinary` and skip compile and link. Key: FNV-1a of both GLSL sources
  and the attribute bindings. A binary the driver refuses is deleted and
  rebuilt. The frame summary says how many programs came from the cache and
  how many were saved. Shipping a cache: play the maps once, then the folder
  can go into the NRO's romfs (loader still to add).
- **Fourth result:** second run, first gameplay minute: 170 of 171 programs
  from the cache, 2.7 s in all (~16 ms each, slowest 47 ms, was 759). Later
  windows settle at 42-45 ms per frame. `glUseProgram` alone is 0.24 ms, so the
  ~22 ms under "constants" (per ~900 draws) is the driver doing the queued
  draw work when the next uniform update arrives. Next step for it: constants
  in uniform buffers (vsc/psc as std140 blocks, written into the
  persistent-mapped ring and bound with `glBindBufferRange`), so a draw binds a
  buffer instead of pushing constant data through the command stream.
- **Constants in uniform buffers** (done). The profile from `PROF_SCOPED`
  (section 3) showed the main thread waiting on the render thread for most
  of the frame, and the render thread spending 16-29 ms of 21-42 ms under
  "constants". The translator now declares `vsc`/`psc` as std140 blocks
  (`NxVsConst`, `NxPsConst`); `nxGlBindConstants` copies a stage's rows
  into a 4 x 2 MB persistent-mapped buffer (segments fenced like the vertex
  ring) only when its file version changed, and binds the range at binding
  0 / 1. Block bindings are set after every link and binary load. The alpha
  test and half-pixel uniforms are sent only when their value changes. The
  old "constants" timer also covered sampler and attribute setup; those
  are now their own rows (`samplers`, `attributes`).
- **Result, and the per-draw state behind it.** Constants fell to ~1 ms a
  frame, but the frame barely moved (36-44 ms): the time reappeared under
  samplers (~10 ms) and attributes (~11 ms). Mesa re-validates whatever a call
  touches, even to the same value, and bills it to the next call -- the real
  cost is ~37 us of driver work per draw. So every texture, sampler and
  attribute binding now goes through tracked wrappers (`nxBindTexture`,
  `nxBindSampler`, `nxActiveTexture`, `nxVertexAttribPointer`,
  `nxVertexAttribValue`) that skip unchanged calls; deletes forget the name
  first, since GL reuses names. The report also names the textures behind
  "texture upload" (4-6 ms a frame in game, cause unknown yet).
- **Program warm-up.** Cached binaries still cost ~18 ms each (Mesa runs its
  back end on `glProgramBinary`), so the first use of a program is a hitch
  with or without the cache. Every pair built is now listed in
  `shadercache/pairs.txt` by the GLSL hashes of its shaders, and after each
  present while the client is not active (loading screen, menus) the GL
  thread builds, for up to 120 ms, the listed pairs whose shaders are loaded
  and that have no program (`nxGlWarmPrograms`). The summary prints
  `warm-up: N programs built ahead of use`. The list fills while playing, so
  warm-up starts from the second session. It is small and build-independent:
  shipping it in the NRO gives new players the same warm-up (their binaries
  are then made on that first load). Next: spread the warm-up over more cores
  (a second GL context), and build the stragglers in game in the background.
- **Warm-up, first result.** The list had 236 pairs, but 718 programs were
  built during the boot load and 97 more at the first draws in game: programs
  were keyed by shader object, and the zones create the same shader many
  times. Programs are now keyed by the GLSL hashes of the pair (`s_programs`),
  so every copy shares one, and a program goes only when the last live shader
  with that GLSL does (the registry keeps all of them per hash).
  Skipping unchanged binds took attributes from ~11 to ~2-4 ms a frame.
- **`$model_lighting` re-sent whole every frame.** The texture report named
  it: a 256x256x4 volume the engine patches a few 4x4x4 blocks of each frame
  (`RB_PatchModelLighting`, `LockBox` over the whole volume), uploaded whole
  with `glTexImage3D` -- ~1 MB and 3-12 ms a frame, counted under samplers.
  An uncompressed texture uploaded again now keeps a copy of what GL holds and
  sends only the rows that differ, with `glTexSubImage*` into the existing
  storage (`nxGlUpdateTexture`). The constant buffer ring grew to 8 x 2 MB:
  at ~2.5 MB of constants a busy frame, 4 segments made it wait on the GPU.
- **Second result.** Warm-up: 250 of 251 listed pairs built during the boot
  load, and only 2-13 programs per 600 frames in game (the new ones from a
  room opened late). `$model_lighting` went from ~1 MB to ~16 KB a frame but
  still cost ~5.6 ms: the write itself stalls, because the driver waits for
  the GPU to finish reading the texture before changing it. A texture
  rewritten in place now rotates through three GL copies (`glRing`), each
  with its own shadow, so an update goes into a copy the GPU finished with
  frames ago and still sends only the rows changed since that copy was last
  current. In-game frames were ~32-45 ms, the render thread ~24 ms drawing
  plus ~9 ms in the swap.
- **Third result.** The three copies did not help either (~5.2 ms): the cost
  is per upload call -- a write into a tiled texture makes the driver stage
  it -- and a frame's patches came out as dozens of small row runs. Now the
  changed rows of all slices go up as one band per level, from the constant
  buffer's mapped storage bound as a pixel unpack buffer
  (`nxGlTexSubImageRows`).
- **The freeze entering the map** was the first frame uploading the map's
  textures: 4732 levels in that window. Textures are now uploaded during the
  loading screen once the loader has left them alone for 300 ms
  (`nxGlPreuploadTextures`, 60 ms a present; `s_texPending` fed by
  `nxTexMarkDirty`). The summary prints `pre-upload: N textures`.
- **Fourth result, and vsync.** `$model_lighting` fell to ~0.7 ms a frame and
  samplers to ~4 ms; the first frame no longer uploads the map (1000+
  textures went up during loading). The render thread was then ~27 ms drawing
  plus ~11 ms in `eglSwapBuffers`: EGL's default swap interval of 1 waits for
  the display refresh, so a 38 ms frame showed at 50 (or 33). `nx_vsync`
  (default 0) sets the interval; 1 restores the wait. A `GL_TIME_ELAPSED`
  query around each frame now prints `GPU time: N ms per frame` in the
  summary, to tell a GPU limit from a CPU one.
- **Crash on Restart Map (SP)**, assert `startLocalId == localId` in
  `VM_TrimStack` during `map_restart` -> `G_ShutdownGame`. Three walkers of a
  waiting thread's saved stack (`VM_TrimStack`, the one at cscr_vm.cpp:849,
  `Scr_GetThreadUsage`) still stepped 5-byte x86 entries with 4-byte values,
  while `VM_ArchiveStack` writes `SCR_STACKBUF_ENTRY` (1 + 8) bytes; they now
  match it. Shutdown is the first time live waiting threads get walked that
  way. `Scr_DumpScriptThreads` / `Scr_DumpScriptVariables` (debug dumps) had
  the same entries and x86 struct sizes (0x8C, 0x10) and are fixed too.
- **Fifth result: GPU 20-24 ms, no overlap.** With vsync off frames were still
  34-40 ms and the swap still ~9 ms: the driver kept the frame's commands
  until the swap, so the GPU (20-24 ms a frame by the new timer) started only
  when the render thread (~27 ms) had finished. A `glFlush` every
  `nx_glflush` draws (default 64, 0 off) hands the GPU each part as it is
  issued, so the two can overlap; the summary prints the flushes per present.
- **Restart Map crawled at ~1 fps (SP).** The in-place restart (MP's:
  `SV_RestartGameProgs` + reconnecting the clients) left SP's per-player
  script state unbuilt -- `players[p].solo_powerup_hud` undefined -- and the
  power-up HUD threads errored every pass, each error a stack written to the
  SD card. SP now restarts by queueing `devmap`/`map` for the current map, the
  path the first load takes. Script runtime error headers (channel 6) are
  now logged too; the stacks used to appear without their message.
- **Sixth result: GPU-bound.** With the mid-frame flushes the render thread's
  drawing fell to ~14-24 ms a frame and the swap rose to ~10-14 ms: it now
  waits on the GPU (~21-26 ms a frame at 1280x720). Further gains are GPU
  work: fewer pixels shaded (a lower game resolution is upscaled by the
  present blit already, `GL_LINEAR` when the sizes differ) or cheaper passes.
- **Crash reloading the level (Restart Map)**, `UI_Shutdown` ->
  `DevGui_FreeMenu_r` -> `UILocalVar_Shutdown` -> `FreeString(0x200000000)`.
  `UILocalVar_Find` / `UILocalVar_FindOrCreate` returned `context + 12 * hash`,
  the x86 stride of `UILocalVar` (24 bytes on LP64), so every UI local
  variable was read and written in the wrong place; shutdown freed a garbage
  name. They return `&context->table[hash]` now.
- **Handheld GPU clock.** The frame is GPU-bound, and handheld runs the GPU at
  307.2 MHz by default while the console offers 384.0 and 460.8 MHz there.
  `nx_clock.cpp` sets it through `clkrst` (8.0.0+; `pcv` before), from
  `nx_gpuclock` (MHz: 307 / 384 / 460, default 460; 0 leaves the system's),
  only in handheld mode, re-checked every ~2 s since docking resets it, and
  restores the clock it found on exit.
- **The clock yields to sys-clk.** `NX_ClockUpdate` only replaces the
  system's handheld default (307.2 MHz); any other clock was set by someone
  else (sys-clk, an overclock tool) and stays. On exit it puts the original
  back only if the clock is still the one it set.
- **540p handheld, 720p docked.** `nx_main.cpp` adds `+set r_mode 960x540`
  when the title starts in handheld mode, `1280x720` docked (960x540 is now
  in both mode lists, `s_modes` and `EnumDisplaySettingsA`). The window stays
  1280x720 and the present blit scales the back buffer up (`GL_LINEAR`); the
  touchscreen point is scaled to the game window (`NX_GameWindowSize`). It is
  chosen at boot only: docking later keeps it, since changing it live would
  be a `vid_restart` with every render target remade.
- **GPU time by part.** Render scopes listed in `nx_prof.cpp`
  (`s_gpuScopes`: the whole command execution, the 3D frame, depth prepass,
  Lit, LitPostResolve, Dynamic Lights, Emissive, post effects) also put a
  `GL_TIMESTAMP` pair around themselves (`NxProf_GpuBegin/End`), read back
  four presents later without waiting; the summary prints
  `GPU time by part, ms per frame`. What the list leaves out (the HUD, the
  2D) is the first entry less the rest.
- **540p did not apply at first.** The log said `'960x540' is not a valid
  value for dvar 'r_mode'`: `R_AddValidResolution` drops modes under 800x600,
  so the enum never had it. On NX the floor is 800x540. The startup `+set`
  runs before `R_Init` registers `r_mode`, and the registration keeps a
  startup value that is in the enum, so the window is created at 960x540.
  (The in-game video menu showing 720 was right: it had not changed.)
- **GPU by part, 1280x720 handheld at 460.8 MHz (first reading).** Whole 3D
  frame (`RB_StandardDrawCommands`) 12-28 ms; Lit 7-18 ms -- the bulk, and
  the part that scales with pixels; post effects 2.4-3.3 ms; Emissive
  0.4-1.5 ms; the 3D frame less those (clears, sky, decals, resolves) 5-9 ms.
  Frames 31-36 ms with GPU time 19-22 ms.
- **Stutters are not shader builds.** Every in-game window showed 0 programs
  built, yet slowest frames of 165-510 ms and once 1.57 s (with a 1 s GPU
  frame). A frame over 100 ms outside loading now prints two `[nx-hitch]`
  lines: from `nx_prof.cpp`, that main-thread frame's costliest scopes on
  every thread (each frame snapshots every site to diff against the next);
  from the GL layer, that present's GL work by kind and programs built.
- **540p, first reading (handheld, 460.8 MHz).** `Attempting 960 x 540
  window`; in-game frames 26-28 ms (from 31-36 at 720p), GPU 15.7-16.7 ms:
  Lit 9.2-9.9, post effects ~1.5, Emissive ~0.5.
- **Stutters: texture streaming, not shaders.** The `[nx-hitch]` lines showed
  `programs built 0` in every hitch but one (2 programs, ~62 ms). They came in
  bursts -- ~30 frames of ~200 ms right after the map started (about 6 s at
  5 fps), shorter ones entering new areas -- and every one had the Stream
  thread in `R_StreamUpdate_ReadTextures` for 130-220 ms of the frame, the
  render thread re-uploading textures (15-45 ms) and binding them (35-90 ms
  under samplers). The 2.1 s freeze was `Snd_StreamUpdate` on the same
  thread sitting 1.9 s, the sound streams' reads queued behind the texture
  reads. `r_stream` (high mip streaming) now defaults to 0 on NX: with
  `r_picmip 3` the streamed high mips are dropped anyway.
- **`r_stream 0`, result.** Stutter bursts gone; frames 17-31 ms. But the
  textures were visibly worse than with streaming: the streamed high mips
  were not all dropped by `r_picmip 3`. `r_stream` is a bitmask of what
  streams its high mips -- 1 world surfaces, 2 xmodels, 4 brush models --
  and 7, all three, is the PC default. Raising texture quality without the
  streaming hitches: a lower `r_picmip` (and `_bump` / `_spec`), all loaded
  with the level.
- **The hitches left after that:** (1) at every 600th frame, 140-190 ms --
  the log summaries written to the SD card on the printing thread, fixed by
  the asynchronous log (section 3, Logs); (2) runs of slow frames with the
  render thread 85-120 ms in the swap, i.e. the GPU itself, likely heavy
  effects (to be read from `GPU time by part`); (3) once, entering the map:
  `Com_EventLoop` 500-600 ms and the first `SV_SendClientMessages` 800 ms.
- **`+set` lines after the map command came too late.** `r_picmip 2` in
  `cmdline.txt` showed `Using picmip 2` and then `Using picmip 3` for the map:
  the file's own `+devmap zombie_theater` came before the added lines, and the
  startup commands run in order -- the map loaded with the preset's picmip
  before the user's value was set. `nxAppendCmdlineFile` now moves a
  `+devmap` / `+map` / `+spdevmap` / `+spmap` command and its map name to the
  end of the line. Not cheat protection.
- **FFmpeg on stderr.** The WMA decoder logged `Could not update timestamps for
  skipped samples` ~670 times a session (we do not use its timestamps);
  `av_log_set_level(AV_LOG_ERROR)` at the first decoder open. ~20 `nb_frames
  is 0` errors a session remain visible.
- **Hitches still open (log of 2026-10-02 12:53):** frames 1409-1411, 0.4-0.8 s,
  the server thread 338 ms in one `G_RunFrameForEntity` (script/AI); frame
  1926, 1.07 s with the swap, the sound worker (`SND_CommandPump`) and the
  server all stalled the same ~1.06 s, then ~60 frames of ~100 ms with
  `RB_StandardDrawCommands` ~100 ms but almost no GL draws -- what the player
  did then decides where to look.
- **picmip 2, `r_stream 0` (log of 2026-10-02 13:00).** Applied for the map
  this time. In-game frames 19-30 ms by area, GPU 12-19 ms, slowest frame of
  each 600 44-85 ms. The hitches left in game are the main thread in
  `SV_AllowPackets` for 0.5-1.1 s waiting on the server thread, which sat in
  `SERVER: msg recv` (`Com_ServerPacketEvent`) with no scope under it; the
  worst also had the swap waiting 1.1 s. `Com_ServerPacketEvent` now scopes
  the socket read (`NET_GetClientPacket`), the loopback read and each
  `SV_PacketEvent`, and `SV_ExecuteClientMessage`, `SV_ExecuteClientCommand`,
  `ClientThink_real`, `ClientCommand` and `Pmove` got scopes, so the next
  `[nx-hitch]` line names the part.
- **The packet-path hitches were the network socket.** With the new scopes
  the 0.3-1 s server stalls were all `NET_GetClientPacket`: the server polls
  its UDP socket every frame, and a recvfrom through the system's network
  service now and then took 0.3-1 s to return (once alongside a 1 s stall
  of several threads at once). SP's only client is on the loopback, so the
  SP build no longer reads the socket (`Com_ServerPacketEvent`). The client
  side already skips it when a local server runs.
- **picmip.** 3 with `r_stream 0` looked too blurry. 1 (`_bump`/`_spec` 2)
  brought runs of ~200 ms frames where the wait landed in whichever GL call
  came next (swap 177-199 ms, constants 176 ms, `glUseProgram` 39 ms) while
  the GPU's own time stayed under 35 ms: the driver stalling, most likely
  over the ~4x texture memory. 2 ran clean (slowest in-game frames 44-85 ms),
  so the preset is now picmip 2 for colour, normal and specular maps.
- **Double-speed zombies again, after killing several at once.** The corpse
  fix freed a hidden corpse's DObj only inside `CG_ActorCorpse`'s
  `!(eFlags & EF_NODRAW)` branch; a corpse flagged no-draw (several deaths at
  once) kept its DObj on the actor slot's tree, and the next zombie in that
  slot shared it again. The free now comes before that check, and
  `CG_UpdateActorDObj` keeps one live DObj per actor tree: creating one frees
  any other entity's DObj still on the same tree, printing `[nx-anim] ent N
  still had a DObj on the anim tree ent M now uses; freed`.
- **Long match (log of 2026-10-02 13:31): the stutters were shaders.** 57 of
  60 hitches built at least one program, all first-time compiles (`0 from
  cache, N saved`) at 140-200 ms each, one 996 ms -- turning the power on and
  opening new areas brought materials and shader combinations never drawn
  before. They are in `pairs.txt` now, so later sessions warm them while
  loading. The other 3 were the first frames entering the map. Covering
  first-time compiles in game needs the background builds (second context).
- **Button prompts in the Switch's labels.** The game draws gamepad buttons
  as font glyphs at codes 1 (A), 2 (B), 3 (X), 4 (Y) (`Key_KeynumToString`
  returns the code when translating). `R_GetCharacterGlyph` swaps 1<->2 and
  3<->4 while `nx_switchglyphs` is set (default 1), so a prompt for the
  bottom button reads B and the right one A, as on the Switch. Bindings are
  untouched.
- **KBZ rebuilt from an untranslated game set (2026-10-02).** All zones the
  port uses converted and validated (`tools/nx/convert-zones.sh`);
  `frontend_patch.ff` is not in this set (the engine runs without it). Most
  `.kbz` came out byte-for-byte the size of the previous conversion.
- **Crash: aim assist found no DObj (log of 2026-10-02 15:28).** Assert
  `dobj` at `aim_target.cpp:333`. Two zombies, ents 571 and 714, were both in
  the snapshot as actors on the same actor slot, so on the same client anim
  tree. The one-DObj-per-tree guard freed the other's DObj each time it
  created its own, they alternated every frame, and aim assist read one in
  the frame it had none. The guard now leaves the other DObj alone when that
  entity is itself in the snapshot as an actor on the same slot (logs
  `[nx-anim] ents N and M are both live on actor slot S; tree shared`, those
  two share the tree and may play at double speed), and
  `AimTarget_GetTagPos` aims at the origin of an entity with no DObj instead
  of asserting. Why the server sends two actors on one slot is still open.
  When it happens again the log has three lines: the pair, then for each
  entity its client state (eType, slot, eFlags, in the snapshot, time its
  DObj was created) and the local server's (inuse, eType, slot, health,
  whether its `actor_s` is live or freed and which entity owns it).
- **Aim assist on (2026-10-02).** The console aim assist code runs on PC too
  (`AimTarget` collects targets every frame), but
  `AimAssist_PlayerDisabledAutoAim()` returns 1 on PC, which sets
  `ps.targetAssistDisabled` and turns off slowdown and lock-on; ADS snap and
  lock-on are also off by their dvar defaults. On NX the function follows
  `nx_aimassist` (default 1) and `aim_lockon_enabled` / `aim_autoaim_enabled`
  default to 1. The tuning dvars keep their registered values, which are the
  console ones (shared code). Applies to the MP build too.
- **28 MB log after a crash.** Not the cause of the crash: the crash handler
  wrote out the ring while the log thread was in the middle of writing the
  same bytes; the thread then moved its read position past the write
  position and wrote the whole 4 MB ring again and again (the log held the
  same 23,000 lines four times plus zeroed ring memory). The first crash
  drain now stops the thread and waits up to a second for its write in
  progress.
- **Streaming without stutters (not done).** The streaming hitches had three
  parts: the Stream thread's reads (off the main thread, harmless alone), the
  render thread uploading every streamed texture whole at its next bind
  (15-45 ms upload, 35-90 ms under samplers), and the sound streams sharing
  the Stream thread and its SD reads. Making it smooth means spreading the
  uploads over frames with a per-frame budget (keeping the old mips bound
  until the new ones are in), and keeping the sound reads ahead of the
  texture reads.
- **Restart Map, still broken (SP).** Reloading the level now gets past the
  UI shutdown but crashes unloading the map's zones: `DB_FreeUnusedResources`
  -> `Mark_WeaponVariantDef` -> `Mark_XModelPtr` follows a weapon (still
  loaded, in `common_zombie`) into an XModel of the zone being unloaded.
  Zone unloading has never run on NX -- quitting to the menu takes the same
  path. The in-place restart (MP's) does not rebuild SP's per-player script
  state; retail SP's own `map_restart` / `fast_restart` are not in this
  decomp yet. Either needs reconstructing.
- **Vision sets (SP).** `player VisionSetNaked()` was a no-op stub; it now does
  what the global `VisionSetNaked` does (configstring 1550, which the client
  lerps to). `VisionSetLastStand` stays a stub: this client has no last-stand
  vision channel (`VISIONSETMODE_*` stops at EXTRACAM), so the last-stand
  look needs that part of the retail SP client reconstructed.
- **Zombie corpses in T-pose:** SP converts the actor into a corpse in place
  (`Actor_BecomeCorpse`), and the client draws the corpse through
  `actorCorpseInfo[slot]`, whose tree starts empty; retail hides that with the
  ragdoll. Copying the server's corpse tree across crashed (server script
  strings in client notifies), so corpses are hidden instead while physics is
  off -- see the physics note in section 3.
- `Could not load rawfile "maps/gametypes/zom.gsc"` is expected: retail ships
  no gametype script for Zombies and the load is optional under `KISAK_SP`.
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
- **Frame rate is area-dependent, ~33-50 fps handheld (SP).** Both the GPU
  (12-19 ms at 540p, 460.8 MHz; Lit is ~60% of it) and the render thread's
  per-draw driver cost (~25-30 us a draw, 500-900 draws) are close to the
  frame time. Further gains: fewer draws, cheaper Lit, or the GPU clock.
- **Resolution is chosen at boot.** Docking or undocking mid-game keeps the
  boot resolution until the title restarts (changing it live would be a
  `vid_restart`, never exercised here).
- **Restart Map / quitting to the menu (SP).** The level reload crashes
  unloading the map's zones (`DB_FreeUnusedResources` -> `Mark_XModelPtr`
  following a still-loaded weapon into the zone being freed); zone unloading
  has never run on NX. The in-place restart (MP's) does not rebuild SP's
  per-player script state. Retail SP's `map_restart` / `fast_restart` are not
  in the decomp. Section 5, "Restart Map".
- **Texture streaming is off** (`r_stream 0`), so textures are the level's
  own mips at `r_picmip 2`. Smooth streaming needs per-frame upload budgets
  and the sound reads ahead of the texture reads (section 5).
- **Occasional hitches remain**: a first-time shader (~140 ms, once, then it
  is in `pairs.txt`), heavy-effect GPU frames, and one-off spikes entering
  the map. `[nx-hitch]` lines in the log name the scopes of any frame over
  100 ms.
- **Vision sets (SP).** `player VisionSetNaked()` works; `VisionSetLastStand`
  is a stub (no last-stand vision channel in this client) and
  `GetVisionSetNaked` returns nothing.
- **SP script stubs.** Builtins the decomp has not reconstructed report once
  in the log (`TODO(SP-STUB) script builtin '...'`): e.g. `setblur`,
  `allowlean`, `allowmelee`, `weaponisgasweapon`.
- **Physics is off** (`nx_physics 0`): the solver is still at x86 offsets.
  No ragdolls; Zombies corpses are hidden when their death animation ends
  (section 3, physics).
- **No light coronas**: occlusion queries report 0 visible pixels, so
  `RB_DrawCorona` never draws one.
- **`r_water_sim.cpp` is not LP64-clean** (dozens of pointer/int casts); maps
  with dynamic water will break. `mp_nuked` has none.
- **`ui_viewer_mp` cannot be converted**: it contains a `ComWorld` (asset type
  13), map data the converter does not handle yet. The engine carries on
  without it.
- **Bloom is off** (`nx_bloom 0`) -- the light glows drawn away from their
  lights (Zombies: the Quick Revive machine, the lamps outside), more
  visibly from afar, were **the bloom**:
  `nx_bloom 0` makes it go, so the low preset (`nx_main.cpp`) now sets it --
  which also saves the bloom's ~10 small passes. The fault inside
  `RB_BloomLDR` (one of its downsample / blur / streak / smooth passes reading
  or writing offset) is not found yet; it matters beyond bloom, since depth of
  field, blur and the revive effect use the same targets. Ruled out: light coronas (`IDirect3DQuery9::GetData` reports 0 visible pixels, so
  `RB_DrawCorona` never draws one), and the `*_eyeoffset` FX vertex shaders
  (translated, they pull the sprite towards the eye along x, y, z and w
  together, which leaves its screen position alone). Left: the FX sprites
  themselves and the bloom. To split them on hardware, `cmdline.txt`:
  `+set fx_draw 0` hides the effects, `+set nx_bloom 0` adds no bloom
  (`RB_BloomLDR`). To read a material's shaders offline,
  `FFCONV_SHADERDUMP=<dir> convert.exe <zone.ff> <out.kbz>` writes each
  shader's bytecode and a material -> techset -> technique -> shader index.

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
   `+devmap zombie_theater` is playable: move, shoot, knife, kill, rounds
   advance, with the gamepad (always on) and the low graphics preset, at
   ~42-45 ms a frame in play. Open: sound, ragdolls (physics), a report of
   zombies animating about twice as fast in round 2 (unconfirmed), small
   visual bugs.
7. **Sound (first pass).** The whole sound engine runs; only the XAudio2
   driver layer (`snd_driver_xaudio2*.cpp`) is replaced, by `src/nx/nx_snd.cpp`
   (was the null driver `nx_snd_null.cpp`). It mirrors the PC driver call for
   call: `SD_StartAlias` creates a voice from the alias's `snd_asset`,
   `SD_UpdateVoice` takes the engine's speaker-map volumes times the dry level
   and the pitch, starts the voice once its start delay is over and stops it
   when its data runs out; streams take two windows at a time from
   `Snd_Stream*` and release them on the main thread once played. A software
   mixer thread (priority 0x2B) resamples each voice linearly (rate x pitch to
   48 kHz) and feeds libnx audout in 1024-frame buffers, four queued.
   Formats: PCM 16-bit and MS-ADPCM (262-byte blocks per channel, 512 frames,
   the standard seven coefficients). Streamed sounds are **not** WAV files
   despite the name: each `.wav` in the `.iwd` archives (`main/iw_100.iwd`,
   1.3 GB, and the `localized_English_iw1*` / some `iw_2x` ones) starts with
   the engine's own 2096-byte `snd_asset` header (version 1, 48 kHz, format 6
   = MS-ADPCM for the voice lines). Not done yet: WMA (refused and counted),
   the reverb bus, the per-voice low-pass and futz DSP, the master EQ and
   limiter. `[nx-snd]` prints every 10 s: voices started by format, refused
   (with the last alias), peak playing, starved stream reads, mixer time.
   **Reading the `.iwd` archives was broken on LP64**, which is what the
   streams hit first: `FS_FOpenFileRead` clones the iwd's unzip state for a
   second open file with `Com_Memcpy(zfi, handle, 128)` -- the x86 size of
   `unz_s` up to `tmpFile`, which on LP64 (8-byte `unsigned long`) stops short
   of `cur_file_info`, so the clone read another entry's size and offset
   ("Invalid file (incorrect length)", "could not read file"); now
   `offsetof(unz_s, tmpFile)`. Also in `unzip.cpp`: `unzlocal_getShort/Long`
   sign-extended into the 64-bit `uLong`, and `FS_Seek`'s skip-by-reading into
   a null buffer failed inside deflated entries (inflate refuses a null
   output); it now skips through a scratch buffer.
   **WMA is most of the loaded sounds**: the converter now prints loaded sounds
   by format per zone. `common_zombie`: 1444 WMA (18.0 MB) and 69 MS-ADPCM;
   `zombie_theater`: 302 WMA (4.4 MB) and 24 ADPCM; `en_zombie_theater` 43 WMA;
   `code_post_gfx` 30 WMA. As PCM that would be ~16x (hundreds of MB), as
   ADPCM ~4x; decoding at play time (devkitPro's `switch-ffmpeg`, wmav2) keeps
   the memory as it is.
   **WMA now decodes at play time**: `switch-ffmpeg` (install with devkitPro's
   pacman) is linked for its WMA v2 decoder alone, referenced by name
   (`ff_wmav2_decoder`; `avcodec_find_decoder` would pull in every codec). One
   decoder per voice slot, flushed and reused when rate and channels match.
   xWMA here is fixed packets (4096 bytes stereo, 2230 mono, the PC driver's
   nBlockAlign; the seek table counts them) of WMA v2 without extradata; the
   six bytes FFmpeg's xwma demuxer fills in (flags 31) are supplied the same
   way. It costs ~12 MB of NRO (the codec core comes with `avcodec_open2`).
   The 10-second report also lists the five aliases started most, to find the
   sounds reported as repeating endlessly.
   First WMA run: WMA plays (45-134 started per 10 s, none refused), mixer
   23-58 ms per second of audio. The alias list came out empty: for an
   in-memory sound `g_snd.voice[i].alias` is only set after the driver's start
   (`SND_SetVoiceStartInfo` follows it), so `createVoice` now takes the alias
   from the start info. Open: zombie grunts cut short, and some sounds (the
   chandeliers above spawn among them) repeating. The engine does not stop
   one-shots by time (`soundFileInfo.endtime` is never read), so a cut is the
   engine stopping the voice itself -- voice limits, stealing -- which voices
   that never end would cause too. The report now counts one-shots that
   played to their end against those stopped early, and lists the aliases
   cut most.
   Second run: the engine barely cuts one-shots (0-6 per 10 s against ~100
   played through). What restarts is two looping ambients, `amb_fire`
   (140-460 starts per 10 s) and `amb_chandelier_loop` (80-440): a loop is
   kept alive by the emitter calling `SND_ContinueLoopingSound` every frame,
   and a loop voice not continued is stopped by `SNDL_UpdateLoopingSounds`, so
   something stops these and the emitter starts them again. `SND_StopVoice`
   now records its caller (`g_nxSndStopCaller`, KISAK_NX) and the report
   lists who stopped looping voices, as elf offsets for addr2line.
   Asset check (`FFCONV_SNDDUMP=1`, `FFCONV_SNDDUMP_SEEK=1` on the converter):
   the fire loops are MS-ADPCM, looping, frames = blocks x 512; for all 1444
   WMA sounds in common_zombie the seek table's last entry equals frames x
   2 x channels, so `frame_count` is exact. Decoded bytes per 2230-byte packet
   vary (VBR-like), averaging ~6.6 KB/s mono -- consistent with the PC
   driver's 6000 x channels bytes/s. A WMA decode error no longer ends the
   sound (it went on to the next packet on PC); the report counts them.
   Third run: the loops are all stopped by `SNDL_UpdateLoopingSounds`, i.e.
   the continue from the emitter missed (`amb_fire` comes from the fire
   manager, `CG_SndUpdateFire`, replayed every frame with the player's handle);
   `SND_ContinueLoopingSound` now prints the first 20 misses with the handles.
   **Fast zombie vocals: a WMA decoded frame is a whole superframe** (~10000
   samples a packet for the vocals) and only its first 4096 samples were kept,
   so most of every packet was skipped; the frame is now handed out a stage at
   a time. **Streamed sounds still failed the length check** (10 files, then
   ~6000 "not found at load time" lines as the engine kept retrying): the
   first file opened from an iwd read through the iwd's own unzip handle,
   which every later open repositions (seeks its FILE to the central
   directory, swaps `cur_file_info`). On NX every iwd open is now a clone with
   its own FILE (`unzReOpen`).
   Fourth run: the fast vocals and the pistol are fixed; the spawn portal
   sound plays. Still wrong: **emitter loops restarting** -- the misses show
   `amb_fire` on loop emitters 3714-3722 (handle entIndex 3710 + slot), limit
   4 by priority. `SND_Frame` pumps every queued command then runs
   `SNDL_Update`, which continues the emitters; a late sound job pumps two
   client frames, two `UPDATE_LOOPS` with no emitter update between, and the
   second retired every emitter loop (then restarted from the top). Now only
   the first check after an emitter update retires (`g_nxLoopsRetiredThisSndFrame`).
   **Streams still failing the length check** (round-start music, theater
   underscore, announcer and player lines) although their headers match the
   iwd sizes exactly: `FS_FOpenFileReadForThread` has no lock, and the stream
   thread and the main thread both reposition the iwd's shared unzip handle
   before copying it into their clone. Opening from an iwd is now serialized
   (`s_nxIwdOpenLock`). The converter's `FFCONV_ALIASDUMP=<substring>` prints
   aliases' limit, distance, volume and pitch fields.
   Fifth run: loops fixed. **Streams still failed: `data 0` in every
   header** -- `Snd_StreamLoadHeader` copied the file's header (the x86
   `snd_asset`, 56 bytes: `data_size` at 48 after a 4-byte `seek_table`) straight
   into the LP64 struct, where `data_size` sits at 56, past the header. It now
   copies the 44 leading bytes and takes `data_size` from offset 48. Picking up
   a power-up crashed on a stream mutex with a null `ThisPtr`: `Snd_StreamInit`
   allocated `g_snd_streams` and `g_snd_buffers` at x86 sizes (0xFA0 = 10 x
   400, 0x15E0 = 20 x 280), so the last streams overlapped the buffers and
   `Snd_StreamBufferInit` zeroed their mutexes; both now use `sizeof`.
   **No Bink load movie on NX**: `SV_SpawnServer` starts it only for solo
   (not `onlinegame`/`systemlink`), and it has always been a black screen
   here; on NX it is never started, so SP shows the `loadscreen_<map>` image
   (`code_post_gfx`), as co-op does.
   Sixth run: the game crashed right after loading, on the `valid` assert in
   `Snd_StreamReleaseWindowWork`: `Snd_StreamReleaseWindow` handed windows
   back through `window_return[]` with a 32-bit compare-exchange, storing the
   low half of the address; now a pointer-sized `__atomic_compare_exchange_n`.
   (Streams only got that far once their headers parsed.) The loading screen
   still looked like the movie: SP's loading menu (`briefing`) draws the
   `cinematic` material, black with no movie; on NX `UI_DrawHandlePic` draws
   `sharedUiInfo.loadingScreen` in its place while loading, and cinematic
   subtitle items no longer print the decomp's "NOT USING
   CINEMATIC_SUBTITLES" placeholder.
   Seventh run: sounds right; loading image shows. **Turning on the power
   crashed**: `CG_UpdatePrimaryLight`'s cone assert (inner 0.764745 <= outer
   0.766044). The inner cone rides in `u.turret.heatVal`, sent as
   `MSG_FIELD_0TO1_P2` (quantized) while the outer is a full float, so Kino's
   power-on spotlights (outer cos 40 deg, inner a hair tighter) arrive out of
   order; NX restores the order before the assert. **No loading bar**: the bar
   is `DB_GetLoadedFraction` (`g_loadedSize / g_totalSize`, 256 KB units),
   counted by `DB_LoadXFile`, which the KBZ path bypasses; `slurp`
   (`nx_kbz.cpp`) now reads in 256 KB units and feeds the same counters.
   **Zombies with sped-up, stuck or broken-looking animations**, more of them
   the longer a game ran: `CG_ApplyPendingAnimCommandsForDObj_SP` replays
   every command still in the 1024-entry anim command ring for an entity
   number whenever that entity's DObj is made, and zombies reuse entity numbers
   constantly -- a new zombie got the previous occupants' commands, applied
   with lags of 100-290 s (the log's `lag` field) so the catch-up ran them to
   their ends, on top of its own. `G_FreeEntity` now calls
   `CG_ForgetAnimCommandsForEnt_SP`, dropping the freed number's commands.
   That removed the late replays but not the **double-speed zombies**: those
   came from the corpse hide (see the physics limitation). A dying actor's
   entity becomes the corpse but kept its client DObj, which points at
   `actorinfo[actorNum].pXAnimTree`; the next zombie given that actor slot got
   a DObj on the same tree, and `CG_UpdateEntInfo` advanced the tree once per
   DObj -- twice a frame. The hide path now frees the corpse's DObj.
   **Random boot failures** (a black screen for a few seconds, then back to the
   menu; relaunching eventually works), there since the SP merge: "Error
   during initialization: Could not load default asset '' for asset type
   'ddl'. Tried to load asset 'ddl/stats.ddl'." SP's stats DDL ships only in
   `patch.ff`, which `DB_LoadGraphicsAssetsForPC` queues without a sync, and
   `LiveStats_Init` asks for it during init. `DB_FindXAssetHeader` stops
   waiting for missing assets during init once the minimum fastfiles are in,
   so whether the boot survived depended on whether `patch` had finished.
   Under KISAK_SP, DDLs now wait for the load queue.
   Still to port from the PC driver, all as plumbing -- the algorithms are in
   the engine as plain C: the reverb bus (`SND_RvFrame`, `snd_radverb.cpp`, fed
   by each voice's wet level), the per-voice occlusion low-pass and futz
   (`snd_dsp.cpp`), the master EQ and limiter.
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

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
  and the settings and graphics menus open, and change values, without
  crashing.

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
    `GfxImage::texture` into a real texture.

  Still missing: `GFXWORLD` → `Load_VertexBuffer` (`db_load.cpp:7984`, `:8001`)
  and `MATERIAL` → `Load_PicmipWater` (`db_load.cpp:2334`).

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

---

## 4. Known problems

- **Some menu buttons are hard to reach with a controller.** The PC menus
  expect a mouse; focus moves between list items with the D-pad, but buttons
  such as the graphics menu's save/apply are not always reachable, so settings
  changes cannot always be applied.
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

## 5. What is missing to reach a map

1. **Convert the map zones.** `GfxWorld`, `clipMap`, `comWorld`, `gameWorld`
   and `mapEnts` have no transcoder in `tools/ffconv/convert.cpp` yet, and
   `Load_VertexBuffer` has no builder in `kBuildSteps`. Until both exist,
   nothing can load a level.
2. **Depth, stencil and culling in the renderer.** The state is recorded; it
   has to be applied together — with `glFrontFace` answering the y flip's
   reversed winding — before any 3D can draw correctly. The web port's
   `src/gfx_gl/gl_d3d9_draw.cpp` and `gl_state.cpp` (cloned next to this tree
   for reference) map the same states.
3. **Finish the LP64 work in the render path.** `rb_postfx.cpp` still has 42
   pointer-truncation sites — 3 hard errors and 39 `int-to-pointer` warnings —
   measured by compiling it with the build's own flags minus `-w`:

   ```sh
   aarch64-none-elf-g++ $(flags from build-nx/CMakeFiles/KisakBlack.dir/flags.make, without -w) \
       -fsyntax-only -fmax-errors=0 src/gfx_d3d/rb_postfx.cpp
   ```

   `r_draw_staticmodel.cpp` indexes its draw stream as 32-bit words
   (`*((unsigned int *)&drawStream + 10)`), the first shape in the table
   above, and it is in code a map will run. Separately,
   `src/gfx_d3d/r_water_sim.cpp:3336` and `src/EffectsCore/fx_beam.cpp:378` and
   `:1008` index past the end of a four-element `unitVec[0].array` to reach
   `unitVec[1]` — undefined everywhere, and on the map path.
4. **Boot straight into a map.** Put an `autoexec_dev_mp.cfg` in `main/` with a
   `devmap` line, so a test run does not have to go through the menus.

---

## 6. Scope: this is the multiplayer executable

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

## 7. Credits

- [KisakBlack](https://github.com/SwagSoftware/KisakBlack) — the decompiled
  engine this is a port of.
- [NaGa](https://github.com/) — the original devkitPro scaffolding.
- [mesa-switch](https://github.com/danfromtico/mesa-switch) — EGL, OpenGL and
  Vulkan on the Tegra X1.
- [riicchhaarrd/KisakBlack](https://github.com/riicchhaarrd/KisakBlack/tree/web-port)
  — the web port, whose D3D9 → GLSL shader translator `nx_d3d9_shader.cpp` is
  adapted from (GPL-3.0).

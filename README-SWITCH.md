# CBO1-NX — Call of Duty: Black Ops on Nintendo Switch

A port of [KisakBlack](https://github.com/SwagSoftware/KisakBlack), the
reverse-engineered Black Ops multiplayer executable, to Horizon — built with
devkitPro and libnx, rendering through [Mesa for Nintendo
Switch](https://github.com/danfromtico/mesa-switch).

**The multiplayer main menu is on screen and the game runs indefinitely.** No
map loads yet, so it is not playable. Work is paused at that point, and this
file is written to be enough to start again from cold — months later, or by
someone else.

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
  `ui_mp`, `patch_mp`, `dev_mp` and the localised variants — with their
  materials, technique sets, images, menus, fonts, sounds, models and effects
  registered into the asset pools.
- **The multiplayer main menu renders**: background, logo, text, cursor, and
  the animated fog (`bg_fogscrollthin`) — the original effect, scrolling, not a
  stand-in.

### The renderer

`src/nx/nx_d3d9_null.cpp` began as a null driver — accept every call, discard
every draw. It is now a partial Direct3D 9 backend over EGL and an OpenGL 4.3
core context on Mesa's Gallium **nvc0** driver (Tegra X1). What reaches the
screen:

| D3D9 | how |
| --- | --- |
| `Clear` | `glClearColor` + `glClear`, honouring the `D3DCOLOR` and the clear flags |
| swap chain `Present` | `eglSwapBuffers` |
| vertex / index buffers | `Lock`/`Unlock` write into real memory; the draw path uploads what changed |
| vertex declarations | `POSITION`, `COLOR0` and `TEXCOORD0` bound as attributes, with the D3D9 decl types mapped to GL (`D3DCOLOR` via `GL_BGRA`) |
| `DrawIndexedPrimitive` | `glDrawElementsBaseVertex` |
| `SetTexture` | textures upload on first use — DXT1/3/5 as blocks through `EXT_texture_compression_s3tc`, plus the uncompressed formats, with a swizzle that gives `A8`, `L8` and `A8L8` back their D3D9 meaning |
| render state | the whole `D3DRS_` file is recorded; the blend equation, its factors and the colour write mask are applied |
| `SetVertexShaderConstantF` | the constant file is kept, and the transform is picked out of it |

The transform is found rather than assumed: `R_CmdBufSet2D` builds the UI
projection by hand, and the driver matches that exact shape against every
four-register window in the constant file. Depth, stencil, culling, scissor and
alpha test are recorded and **deliberately ignored** — nothing here fills a
depth buffer or tracks a winding order yet, so honouring them could only
discard geometry for reasons unrelated to whether the geometry is right.

Every 60 frames the driver prints a census: draw counts and why draws were
skipped, the NDC bounding box of everything the frame drew, the vertex-colour
alpha histogram, which images the frame sampled (named by their `GfxImage`),
texture formats created/uploaded/unsupported, and what is bound to each of the
sixteen samplers. That log is the main debugging instrument — read it first.

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
    `GfxImage::texture` into a real texture. Without it every `SetTexture`
    hands the driver a pointer that was never a texture.

  Still missing: `GFXWORLD` → `Load_VertexBuffer` (`db_load.cpp:7984`, `:8001`)
  and `MATERIAL` → `Load_PicmipWater` (`db_load.cpp:2334`).

One more thing that bites on this port: device resources are created **inline**,
not queued. `Sys_CanCreateDeviceResourcesInline()` returns true for every
thread here, because `code_pre_gfx_mp` is loaded before `R_InitThreads` has
spawned the thread that drains the `RB_Resource` queue — so anything that
queued would block in `RB_Resource_Flush` on a signal nobody can send. Several
`Sys_IsRenderThread()` tests on the image path were changed to ask that
question instead.

See also [`docs/fastfile-stub-assets.md`](docs/fastfile-stub-assets.md) — an
asset whose name begins with `,` is a **stub**, a reference to an asset another
zone really owns, and only `DB_LinkXAssetEntry` knows to strip that comma.

### The `src/nx/` layer

The scaffolding is [NaGa](https://github.com/)'s: `cmake/switch.cmake` drives
the devkitPro build without touching the Windows path, and `src/nx/compat/`
supplies fake `windows.h`, `d3d9.h`, `d3dx9.h` and friends that are found ahead
of the real headers, so ~647,000 lines of engine compile nearly unmodified.
On top of that sit the platform pieces: `nx_main.cpp`, `nx_wincompat.cpp`
(Win32 API surface, threads, files), `nx_winsock.cpp`, `nx_winuser.cpp`,
`nx_xinput.cpp` (HID), `nx_snd_null.cpp`, `nx_platform_stubs.cpp`, and the two
big ones, `nx_kbz.cpp` and `nx_d3d9_null.cpp`.

### The LP64 ratchet

The decompiled tree casts pointers through 32-bit ints in thousands of places.
On x86 that was lossless; on LP64 every one truncates — and `-w` was hiding all
of them. Measured across the Switch build: **3654 sites in 460 files**.

So `cmake/switch.cmake` keeps `NX_SANITIZED_SOURCES`. Everything *not* in that
list gets `-w -fpermissive -Wno-narrowing` applied **per source file**; a file
graduates by being added to the list, and from then on a pointer truncation is
a hard error. CMake prints the count at configure time. **Keep the list
additive — removing a file is a regression.**

Graduated so far: all of `src/nx/`, `com_expressions_eval.cpp`,
`r_material.cpp`, `r_rendercmds.cpp`, `rb_backend.cpp`, `threads.cpp`.

Two lessons from doing it, both written up in the sweeps doc: the **error** is
usually the harmless one (narrowing a pointer to an `int` is ill-formed, so it
is fatal) and the **warning** is usually the crash (widening an `int` back to a
pointer is legal, so it only warns). And a wrong struct offset produces no
diagnostic at all.

[`docs/lp64-sweeps/`](docs/lp64-sweeps/) is the standing census for the classes
the compiler *cannot* see, with the tooling that produces it: the same headers
compiled twice, once `-mabi=lp64` and once `-mabi=ilp32` as a stand-in for x86,
then compared field by field. Read
[`docs/lp64-sweeps/README.md`](docs/lp64-sweeps/README.md) before trusting a
clean sweep — it also documents what the sweeps are structurally blind to.

---

## 3. Known problems

- **Crash when navigating the menu.** Leaving the main menu reaches
  `RB_UI3D_RenderToTexture` (`src/gfx_d3d/r_ui3d.cpp:484`) →
  `RB_GaussianFilterImage` (`src/gfx_d3d/rb_imagefilter.cpp:9`) →
  `RB_FilterImage` → `Material_GetTechniqueSet` (`src/gfx_d3d/rb_tess.cpp:67`)
  and dies there. The UI3D blur path asks for a material whose technique set is
  not what it expects. This is the first thing to fix.
- **Crash on forced exit.** The worker threads are never stopped, so tearing
  the title down from the home menu takes the process apart underneath them.
  Harmless in practice, ugly in a crash report.
- **One hardwired GLSL program.** The game's own vertex and pixel shaders are
  D3D9 SM3 bytecode and are still discarded. Everything on screen is drawn by a
  single built-in program — vertex colour times one sampler — so nothing is
  lit, and any material that depends on its shader doing something is wrong by
  construction.

---

## 4. What is missing to reach a map

1. **Convert the map zones.** `GfxWorld`, `clipMap`, `comWorld`, `gameWorld`
   and `mapEnts` have no transcoder in `tools/ffconv/convert.cpp` yet, and
   `Load_VertexBuffer` has no builder in `kBuildSteps`. Until both exist,
   nothing can load a level.
2. **Translate the D3D9 shaders to GLSL.** The largest single piece of work
   left. [MojoShader](https://icculus.org/mojoshader/) translates SM1–SM3
   bytecode to GLSL and is worth evaluating before writing anything by hand.
   The material system already hands the driver the bytecode and the constant
   registers; what is missing is the translation and a program cache.
3. **Finish the LP64 work in the render path.** `rb_postfx.cpp` still has 42
   pointer-truncation sites — 3 hard errors and 39 `int-to-pointer` warnings —
   measured by compiling it with the build's own flags minus `-w`:

   ```sh
   aarch64-none-elf-g++ $(flags from build-nx/CMakeFiles/KisakBlack.dir/flags.make, without -w) \
       -fsyntax-only -fmax-errors=0 src/gfx_d3d/rb_postfx.cpp
   ```

   Separately, `src/gfx_d3d/r_water_sim.cpp:3336` and
   `src/EffectsCore/fx_beam.cpp:378` and `:1008` index past the end of a
   four-element `unitVec[0].array` to reach `unitVec[1]` — the decompiler's way
   of spelling a swizzle across two vectors. It happened to work on x86, it is
   undefined everywhere, and it is in code a map will run.
4. **Boot straight into a map.** Put an `autoexec_dev_mp.cfg` in `main/` with a
   `devmap` line, so a test run does not have to go through the menu that
   currently crashes.

---

## 5. Scope: this is the multiplayer executable

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

---

## 6. Picking it up again

### Prerequisites

- **devkitPro** with devkitA64 and libnx. Everything for the game is built from
  the **devkitPro MSYS2 shell**.
- Two portlibs devkitPro does not pull in as dependencies:

  ```sh
  dkp-pacman -S switch-libexpat switch-libzstd
  ```

- **Mesa for Switch**, built and installed into `$DEVKITPRO/portlibs/switch`:
  <https://github.com/danfromtico/mesa-switch> (`build-opengl.sh`; see its
  `docs/switch-opengl.rst`). `cmake/switch.cmake` finds it through the
  `OpenGLConfig.cmake` that install drops, and links the archives inside a
  `--start-group` rescan because libGL, libEGL and the `mesa_util` set are
  mutually circular. The Mesa tree itself is **not** in this repository.
- A **UCRT64 MSYS2 shell** with `g++` and `zlib` for the host-side converter —
  it runs on the PC, not on the Switch, so it does not use devkitA64.
- Your own retail PC copy of Black Ops.

### Build the game

```sh
cmake -B build-nx -DCMAKE_TOOLCHAIN_FILE=$DEVKITPRO/cmake/Switch.cmake -G "Unix Makefiles"
cmake --build build-nx -j8
```

Produces `build-nx/KisakBlack.nro` (what you copy to the SD card) and
`build-nx/KisakBlack.elf` — **keep the ELF**, it is what turns a crash report
into line numbers.

### Build the converter and convert the zones

From the **UCRT64** shell:

```sh
cd tools/ffconv
sh build.sh                          # convert.exe, ffconv.exe, kbzdump.exe
./convert.exe <path>/common_mp.ff common_mp.kbz
./kbzdump.exe common_mp.kbz          # optional: prove the zone before copying it
```

Convert every zone the game asks for at boot. The device log names the ones it
could not find (`NX_TryLoadKbz: (not found) ...`), which is the quickest way to
get the list right.

### SD card layout

```
sdmc:/switch/kisakblack/
    KisakBlack.nro          <- build-nx/KisakBlack.nro
    kbz/                    <- the converted zones: common_mp.kbz, ui_mp.kbz, ...
    zone/Common/*.ff        <- the ORIGINAL fastfiles, still required
    zone/<language>/*.ff
    main/                   <- loose game files (iwd, configs)
    cmdline.txt             <- optional: extra startup arguments, one line
```

The `.ff` files must still be there even though nothing reads their contents:
the zone loader opens the file before the KBZ path takes over, and a zone it
cannot open is a hard error. The loader looks for `kbz/<zone>.kbz` first, then
for the `.ff` path with the extension swapped.

### Run it

Launch in **application mode** — open hbmenu by holding **R** while starting an
installed title, then run KisakBlack from there. Applet mode does not give the
engine enough heap; it says so at startup and then fails later in a much less
obvious way.

### Logs

`stdout` and `stderr` go to **nxlink** if the title was launched from it, and
otherwise to `sdmc:/switch/kisakblack/kisakblack.log`. Both are unbuffered, so
nothing is lost in a crash. `nxlink -s KisakBlack.nro` is by far the better
loop while debugging.

### Crash reports

Atmosphère writes one to `sdmc:/atmosphere/crash_reports/`. It lists the loaded
modules with their base addresses and the register set at the fault. Subtract
the KisakBlack module's base from a `PC` or `LR` value and give the difference
to addr2line, against the ELF from the **same** build:

```sh
/opt/devkitpro/devkitA64/bin/aarch64-none-elf-addr2line -f -C -e build-nx/KisakBlack.elf <offset>
```

In practice the engine's own log usually names the site first: assertions print
an `ASSERTBEGIN` / `ASSERTEND` block with file and line before the trap.

---

## 7. Credits

- [KisakBlack](https://github.com/SwagSoftware/KisakBlack) — the decompiled
  engine this is a port of.
- [NaGa](https://github.com/) — the original devkitPro scaffolding.
- [mesa-switch](https://github.com/danfromtico/mesa-switch) — EGL, OpenGL and
  Vulkan on the Tegra X1.

`README.md` is upstream KisakBlack's and is left untouched; everything about
the Switch port is in this file.

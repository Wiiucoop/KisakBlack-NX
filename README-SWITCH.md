# CBO1-NX — Call of Duty: Black Ops on Nintendo Switch

A port of [KisakBlack](https://github.com/SwagSoftware/KisakBlack) to
Horizon, built with devkitPro and libnx.

## Status

The engine **boots**. It reads its fastfiles, runs its configuration,
starts its worker threads, creates a Direct3D device and initialises
render targets, the static model cache and the particle buffers, then sits
in its main loop.

`src/nx/nx_d3d9_null.cpp` is still a null driver -- it accepts every call
and discards every draw -- but two calls out of the whole API now reach the
screen. `Clear` becomes `glClearColor` + `glClear` with the `D3DCOLOR` the
engine passed, and the swap chain's `Present` becomes `eglSwapBuffers`, over
Mesa's EGL and desktop OpenGL (`mesa-switch`, `docs/switch-opengl.rst`). So
the screen is the colour the game asked for, which is what proves the swap
chain, the context and the frame synchronisation. Everything else -- shaders,
vertices, textures -- is still the largest piece of work remaining.

The EGL context is created on first use and belongs to that thread. It lands
on `RB_RenderThread`, which is where `R_HandOffToBackend` puts the whole
`RB_*` sequence with `r_smp_backend` and `sys_smp_allowed` on, and also where
`R_BeginRegistrationInternal` brings the renderer up in the first place. The
driver logs which thread it got, rather than assuming.

Linking Mesa needs two portlibs devkitPro does not pull in as dependencies:

```sh
dkp-pacman -S switch-libexpat switch-libzstd
```

## What was needed to get here

The scaffolding is [NaGa](https://github.com/)'s: `cmake/switch.cmake`
drives the devkitPro build without touching the Windows path, and
`src/nx/compat/` supplies fake `windows.h`, `d3d9.h` and friends that are
found ahead of the real headers, so 647,000 lines of engine compile nearly
unmodified.

On top of that, nine distinct 64-bit portability bugs. Every one has the
same shape: decompiled code that stores a pointer in an `int`, or computes
an offset from a struct size that was measured on x86.

- **`mem_firstfit.cpp`** — `FIRSTFIT_HUNKUSER` was two `HunkUser` slots
  with pointers living in `unsigned int` fields. Rewritten with named
  fields; `Hunk_FirstFitAlloc` now returns `void *` instead of `int`.
- **`mem_userhunk.cpp`** — `Z_VirtualCommit(buffer, 44)` used the x86
  `sizeof(HunkUserDefault)`. It is 80 on LP64.
- **`phys_mem_new.cpp`** — the buffer pointer was truncated through an
  `unsigned int`, and `phys_memory_manager::allocate` returned `int` while
  performing its whole lock-free compare-and-swap in 32 bits.
- **`jobqueue_all.cpp`** — `jqAtomicHeap::Init` stepped five words per
  `LevelInfo` because that is its x86 size. On LP64 it is 32 bytes.
- **`cscr_variable.cpp`** — `Scr_InitVariables` sized its variable pool
  from a constant computed with the x86 `sizeof`, then overran it and
  corrupted the neighbouring allocation. This one took bisection probes to
  find: the damage surfaced hundreds of functions away from its cause.
- **`r_dpvs.cpp`** and friends — negative indices into
  `dynSModelVisBitsCamera` were really `scene.dpvs.entVisData[]` and
  `entInfo[]`; the decompiler expressed neighbouring struct fields that
  way.
- **`dvar.cpp`** — enum string tables were read through
  `domain.integer.max`, which aliased `enumeration.strings` only while
  pointers were four bytes.
- **`r_init.cpp`** — `R_EnumDisplayModes` and `R_ClosestRefreshRateForMode`
  reached `Height` and `RefreshRate` through negative indices into
  `resolutionNameTable`, which sits exactly 4096 bytes past
  `displayModes`.
- **`nx_main.cpp`** — `socketExit` now runs via `atexit`, so a
  `Com_Error` no longer leaves the bsdsocket session dangling and take
  Atmosphère down with it.

`nx_d3d9_null` also reports the capability bits `src/gfx_d3d/r_caps.cpp`
treats as mandatory, without which the engine refuses to start.

## The fastfile problem

`.ff` files serialise structs with **four-byte pointers and x86 struct
sizes**. A 64-bit build cannot read them at all — this is the fundamental
obstacle, and it is about word size, not about ARM.

`tools/ffconv/convert.cpp` transcodes them offline into KBZ1 "prelinked
zones": every asset is rewritten in native LP64 layout with its pointers
flattened into a relocation table. On device, `src/nx/nx_kbz.cpp` only
allocates, copies, relocates and registers — no per-asset parsing.

It currently handles rawfile, stringtable and localize, plus script string
lists. The next asset type needed is `techset`, which opens the
technique → pass → shader tree.

## Building

```sh
cmake -B build-nx -DCMAKE_TOOLCHAIN_FILE=$DEVKITPRO/cmake/Switch.cmake -G "Unix Makefiles"
cmake --build build-nx -j4
```

Requires the retail game files. Launch in **application mode** — via
hbmenu opened by holding R on an installed title — since applet mode does
not give the engine enough heap.

## Help wanted

A real graphics backend. The engine talks Direct3D 9; Horizon offers
deko3d, and Mesa now provides Vulkan. Translating one to the other is the
bulk of what stands between this and an image on screen, and it needs
someone who knows GPUs.
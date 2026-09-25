# KisakBlack-NX — Call of Duty: Black Ops multiplayer on Nintendo Switch

A Nintendo Switch port of [KisakBlack](https://github.com/SwagSoftware/KisakBlack),
the open-source, fully buildable reimplementation of Call of Duty: Black Ops'
multiplayer executable (`BlackOpsMP.exe`).

**Status:** the multiplayer menus boot, render with the game's own shaders and
can be navigated with a controller. **No map loads yet — it is not playable.**
What works, what does not, and how it is all put together is in
[README-SWITCH.md](README-SWITCH.md).

You need your **own copy of the retail PC game** (Steam). No game data is in
this repository, and none ever should be.

This guide is for building on **Windows**. The Switch side is built with
devkitPro; a small converter that runs on the PC is built with MSYS2.

---

## 1. What you need

| | what | why |
| --- | --- | --- |
| 1 | [devkitPro](https://devkitpro.org/wiki/Getting_Started) with **Switch Development** (devkitA64, libnx, switch-tools) | compiler, system library, `.nro` packaging |
| 2 | devkitPro packages `switch-libexpat`, `switch-libzstd`, `switch-zlib`, `cmake`, `make` | libraries Mesa links against, and the build tools |
| 3 | **Mesa for Switch**, the unified SDK (OpenGL + EGL + Vulkan) | the OpenGL driver the renderer runs on — [mesa-switch](https://github.com/danfromtico/mesa-switch) |
| 4 | [MSYS2](https://www.msys2.org/) with the **UCRT64** `gcc` and `zlib` | builds `tools/ffconv`, the zone converter that runs on the PC |
| 5 | Git | to clone this repository |
| 6 | Black Ops (PC, Steam) | the game data |
| 7 | A Switch running homebrew (Atmosphère) with an SD card | ~13 GB free for the game data |

### 1.1 devkitPro

1. Run the devkitPro graphical installer
   (`devkitProUpdater-*.exe` from [devkitPro/installer releases](https://github.com/devkitPro/installer/releases)).
2. Select **Switch Development** and keep the default location, **`C:\devkitPro`**.
3. Open the devkitPro shell (**Start → devkitPro → MSYS2**, or
   `C:\devkitPro\msys2\msys2_shell.cmd`) and bring it up to date, then install
   the extra packages:

   ```sh
   pacman -Syu          # run it again if it asks you to restart the shell
   pacman -S --needed switch-libexpat switch-libzstd switch-zlib cmake make
   ```

   A partial upgrade can leave `cmake` unable to start (`msys-jsoncpp-*.dll`
   missing); a full `pacman -Syu` fixes it.

### 1.2 Mesa for Switch

Build it with mesa-switch's `build-opengl.sh` (see its
`docs/switch-opengl.rst`), or use a prebuilt **unified SDK** archive. Either
way the result is laid out as `opt/devkitpro/portlibs/switch/...`; its
contents go into **`C:\devkitPro\portlibs\switch`**:

```
C:\devkitPro\portlibs\switch\
    include\EGL, include\GL, include\KHR, ...
    lib\libEGL.a, libGL.a, libvulkan.a, ...      (the Mesa archives)
    lib\cmake\OpenGL\OpenGLConfig.cmake          <- required: the build finds Mesa through it
```

`cmake/switch.cmake` needs `lib/cmake/OpenGL/OpenGLConfig.cmake` and
`lib/libvulkan.a` (the unified SDK's EGL embeds Zink, which links against it).

### 1.3 MSYS2 (for the converter)

Install MSYS2 (for example `winget install MSYS2.MSYS2`, which puts it in
`C:\msys64`), open the **MSYS2 UCRT64** shell and run:

```sh
pacman -Syu
pacman -S --needed mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-zlib
```

---

## 2. Build the game (`KisakBlack.nro`)

From the **devkitPro MSYS2** shell:

```sh
git clone --branch switch-port https://github.com/Wiiucoop/KisakBlack-NX.git
cd KisakBlack-NX
cmake -B build-nx -DCMAKE_TOOLCHAIN_FILE=$DEVKITPRO/cmake/Switch.cmake -G "Unix Makefiles"
cmake --build build-nx -j$(nproc)
```

This produces:

- `build-nx/KisakBlack.nro` — the program you copy to the SD card;
- `build-nx/KisakBlack.elf` — **keep it**: it is what turns a crash log into
  source lines, and only the ELF from the same build will do.

Re-run the `cmake -B ...` line whenever a source file is added or removed; a
plain `cmake --build` is enough otherwise.

---

## 3. Convert the zones

A 64-bit build cannot read the game's `.ff` fastfiles directly, so the zones
the menus need are converted on the PC into `.kbz` files. From the **MSYS2
UCRT64** shell:

```sh
cd KisakBlack-NX/tools/ffconv
sh build.sh                     # builds convert.exe, ffconv.exe, kbzdump.exe
```

Then convert the boot zones. Set `GAME` to your Black Ops folder (the one that
contains `zone/` and `main/`):

```sh
GAME="/c/Program Files (x86)/Steam/steamapps/common/Call of Duty Black Ops"
mkdir -p "$GAME/kbz"
for z in Common/code_pre_gfx_mp  English/en_code_pre_gfx_mp \
         Common/code_post_gfx_mp English/en_code_post_gfx_mp \
         Common/common_mp        English/en_common_mp \
         Common/ui_mp            English/en_ui_mp \
         Common/patch_mp         English/en_patch_mp
do
    name=$(basename "$z")
    ./convert.exe "$GAME/zone/$z.ff" "$GAME/kbz/$name.kbz" && ./kbzdump.exe "$GAME/kbz/$name.kbz" | tail -1
done
```

Each zone should end with `validation: ... => OK`. The converted zones sit
**next to** the originals — nothing in `zone/` is modified, and the `.ff`
files are still needed on the Switch. For another language, swap `English/en_`
for your language's folder and prefix.

Known and harmless: `ui_viewer_mp` cannot be converted yet (it contains map
data), and `dev_mp` does not exist in the retail game.

---

## 4. Copy to the SD card

Everything goes in **`sdmc:/switch/kisakblack/`** — copy the *contents* of
your game folder there, plus the `.nro`:

```
sdmc:/switch/kisakblack/
    KisakBlack.nro          <- build-nx/KisakBlack.nro
    localization.txt        <- from the game folder (required)
    kbz/                    <- the converted zones
    zone/Common/*.ff        <- the original fastfiles (still required)
    zone/English/*.ff
    main/                   <- iw_*.iwd, localized_*.iwd, video/
```

The Windows executables and DLLs, `redist/` and `imgui.ini` are not needed.
The largest file is about 1.3 GB, so FAT32 cards are fine.

---

## 5. Run it

Launch in **application mode**: hold **R** while starting any installed game
to open hbmenu, then start KisakBlack. The album applet does not give the game
enough memory (it warns at startup, then fails later).

In the menus the Switch's buttons act **by position**, like an Xbox pad:
D-pad or left stick moves, **B** confirms, **A** goes back. On-screen prompts
show Xbox letters.

The log is written to `sdmc:/switch/kisakblack/kisakblack.log`. If the game
crashes it ends with an `[nx-crash]` block of addresses — see
[README-SWITCH.md](README-SWITCH.md#3-debugging) for turning them into source
lines with `KisakBlack.elf`.

---

## Credits and thanks

- **All original Black Ops developers**, for creating one of the best games of
  all time.
- [KisakBlack](https://github.com/SwagSoftware/KisakBlack) (SwagSoftware) — the
  reimplementation this is a port of — and everyone it credits:
  [KisakCOD](https://github.com/SwagSoftware/KisakCOD),
  [jk3src](https://github.com/PJayB/jk3src),
  [CoD2rev_Server](https://github.com/voron00/CoD2rev_Server),
  [BO3Enhanced](https://github.com/shiversoftdev/BO3Enhanced) and
  [RAD Game Tools](https://www.radgametools.com/) for Bink.
  Development blog: <https://lwss.github.io/Kisak-Black/>.
- [NaGa](https://github.com/) — the original devkitPro scaffolding.
- [mesa-switch](https://github.com/danfromtico/mesa-switch) — OpenGL on the
  Tegra X1.
- [riicchhaarrd/KisakBlack](https://github.com/riicchhaarrd/KisakBlack/tree/web-port)
  — the WebGL port whose shader translator this port's renderer adapts.

Licensed under the GPL-3.0, like upstream KisakBlack.

From upstream:

```
Keep in Mind: This is a ~20 year old game with some known exploits. We will try to fix these as we become aware of them.
However, there is a non-zero chance of some type of binary exploitation when playing online.
```

Discord (upstream KisakCOD): <https://discord.gg/9uqntRWMA3>

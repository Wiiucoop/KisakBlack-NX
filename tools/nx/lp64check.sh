#!/bin/sh
# Compile sources with the Switch build's own flags minus -w, plus the
# pointer-width diagnostics, and print only errors and int<->pointer casts.
# Needs a configured build-nx/. Run from the devkitPro MSYS2 shell:
#   sh tools/nx/lp64check.sh src/game/g_items.cpp [...]
root=$(cd "$(dirname "$0")/../.." && pwd)
cd "$root" || exit 1
fm=build-nx/CMakeFiles/KisakBlack.dir/flags.make
defs=$(grep '^CXX_DEFINES' $fm | cut -d= -f2-)
incs=$(grep '^CXX_INCLUDES' $fm | cut -d= -f2-)
flags=$(grep '^CXX_FLAGS' $fm | cut -d= -f2- | sed 's/ -w / /')
for f in "$@"; do
  "$DEVKITPRO/devkitA64/bin/aarch64-none-elf-g++" $defs $incs $flags -fpermissive \
     -Wint-to-pointer-cast -Wno-narrowing -fsyntax-only -fmax-errors=0 "$f" 2>&1 |
     grep -E "error|int-to-pointer|cast from pointer|to pointer from integer|loses precision" |
     grep -v "^In file" | sort -u
done

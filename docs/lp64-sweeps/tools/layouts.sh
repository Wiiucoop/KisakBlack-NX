#!/bin/sh
# Stage 4: compile layouts.cpp twice from the same headers -- once LP64 (the
# real target) and once ILP32 (a stand-in for the original x86 ABI) -- with
# DWARF, so dwarf.py can read both field tables and diff them.
#
# Nothing is linked or run; this is a layout-only build.
set -e
HERE=$(cd "$(dirname "$0")" && pwd)
REPO=$(cd "$HERE/../../.." && pwd)
WORK=${LP64_WORK:-$REPO/build-nx/lp64-work}
G=${DEVKITA64:-/opt/devkitpro/devkitA64/bin}/aarch64-none-elf-g++
mkdir -p "$WORK"
cd "$WORK"

FM=$REPO/build-nx/CMakeFiles/KisakBlack.dir/flags.make
F=$(grep '^CXX_FLAGS' "$FM" | cut -d= -f2- | sed 's/ -g / /')
I=$(grep '^CXX_INCLUDES' "$FM" | cut -d= -f2-)
D=$(grep '^CXX_DEFINES' "$FM" | cut -d= -f2-)

# ILP32 copy of the compat headers: a few convenience overloads in the shim
# differ from their primary declaration only because a pointer is wider than an
# int. Under ILP32 they collapse onto it and redeclare it:
#   - the (volatile long *) Interlocked* overloads, since `long` == LONG there;
#   - GetProcessAffinityMask(DWORD *), since DWORD_PTR == DWORD there.
# Layout-only build, so drop them in the scratch copy rather than teach the shim
# about a second ABI.
rm -rf compat_ilp32 && cp -r "$REPO/src/nx/compat" compat_ilp32
python - <<'PY'
p = 'compat_ilp32/windows.h'
s = open(p).read()

def drop(first, last):
    global s
    a = s.index(first)
    b = s.index('}', s.index(last)) + 1
    s = s[:a] + '#if __SIZEOF_POINTER__ == 8\n' + s[a:b] + '\n#endif\n' + s[b:]

drop('static inline LONG InterlockedIncrement(volatile long *p)',
     'static inline LONG InterlockedCompareExchange(volatile long *p')
drop('static inline BOOL GetProcessAffinityMask(HANDLE process, DWORD *processMask',
     'static inline BOOL GetProcessAffinityMask(HANDLE process, DWORD *processMask')
open(p, 'w').write(s)
PY

C32=$WORK/compat_ilp32
I32=$(echo "$I" | sed "s#-I$REPO/src/nx/compat#-I$C32#")
F32=$(echo "$F" | sed "s#$REPO/src/nx/compat/nx_prefix.h#$C32/nx_prefix.h#")

$G $D $I   $F   -mabi=lp64  -g -gdwarf-4 -O0 -c "$HERE/layouts.cpp" -o layouts_lp64.o  2> layouts_lp64.err
echo "lp64  exit=$?"
$G $D $I32 $F32 -mabi=ilp32 -g -gdwarf-4 -O0 -c "$HERE/layouts.cpp" -o layouts_ilp32.o 2> layouts_ilp32.err
echo "ilp32 exit=$?"; grep -m5 error layouts_ilp32.err || true
echo "objects in $WORK"

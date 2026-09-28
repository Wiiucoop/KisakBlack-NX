#!/bin/sh
# Regenerate tools/ffconv/layout_gen.h: the x86 -> LP64 field maps convert.cpp
# uses for the map assets. Run from the msys2 UCRT64 shell (host g++), with
# devkitA64 installed:
#
#   sh tools/ffconv/layout/layout.sh
#
# layoutgen parses the structs in structs.txt out of the engine headers and
# writes a probe with an offsetof/sizeof constant per member. The Switch
# compiler builds the probe twice to assembly: LP64, the Switch build, and
# ILP32, standing in for the PC build the fastfiles were written by (4-byte
# pointers and longs, 8-byte alignment for 64-bit scalars, as MSVC lays out
# x86 structs). Nothing is linked. layoutgen then joins the two into the
# header.
set -e
here=$(cd "$(dirname "$0")" && pwd)
root=$(cd "$here/../../.." && pwd)
dkp=${DEVKITPRO:-/c/devkitPro}
case "$dkp" in /opt/*) dkp=/c/devkitPro ;; esac
tmp=${TMPDIR:-/tmp}/kb_layout
mkdir -p "$tmp"

g++ -O1 -std=c++17 -static "$here/layoutgen.cpp" -o "$tmp/layoutgen.exe"

find "$root/src" -name '*.h' -not -path '*/libs/*' -not -path '*/DemonWare/*' |
    cygpath -m -f - > "$tmp/headers.txt"
"$tmp/layoutgen.exe" probe "$(cygpath -m "${STRUCTS:-$here/structs.txt}")" "$(cygpath -m "$tmp/probe.cpp")" \
    "$(cygpath -m "$tmp/members.tsv")" "@$(cygpath -m "$tmp/headers.txt")"

for abi in lp64 ilp32; do
    "$dkp/devkitA64/bin/aarch64-none-elf-g++" -S -o "$tmp/$abi.s" -mabi=$abi \
        -march=armv8-a+crc+crypto -D__SWITCH__ -DKISAK_MP -DKISAK_NX -DNDEBUG \
        -std=gnu++20 -fPIE -fsigned-char -fms-extensions -fpermissive -w \
        -include "$root/src/nx/compat/nx_prefix.h" \
        -I"$root/src/nx/compat" -I"$root" -I"$root/src" -I"$root/src/libs" \
        -I"$dkp/portlibs/switch/include" -isystem "$dkp/libnx/include" \
        "$tmp/probe.cpp"
    # Each constant is emitted as "layout_<name>:" then ".word <value>", or
    # ".zero 4" when it is 0.
    awk '/^layout_[A-Za-z0-9_]+:/ { name = substr($1, 8, length($1) - 8); next }
         name != "" && $1 == ".word" { print name, $2; name = "" }
         name != "" && $1 == ".zero" { print name, 0; name = "" }' "$tmp/$abi.s" > "$tmp/$abi.txt"
done

"$tmp/layoutgen.exe" emit "$(cygpath -m "$tmp/members.tsv")" "$(cygpath -m "$tmp/lp64.txt")" \
    "$(cygpath -m "$tmp/ilp32.txt")" "$(cygpath -m "${OUT:-$root/tools/ffconv/layout_gen.h}")"
echo "wrote ${OUT:-tools/ffconv/layout_gen.h}"

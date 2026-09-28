#!/bin/sh
# lp64check.sh over whole directories of src/, 8 at a time. Writes one line per
# site to build-nx/lp64scan.txt and prints the count.
#   sh tools/nx/lp64scan.sh gfx_d3d physics
root=$(cd "$(dirname "$0")/../.." && pwd)
cd "$root" || exit 1
out=build-nx/lp64scan.txt
for d in "$@"; do find src/$d -name '*.cpp'; done |
  xargs -P 8 -n 4 sh tools/nx/lp64check.sh 2>/dev/null |
  sed "s|$(cygpath -m "$root" 2>/dev/null || echo "$root")/||" | sort -u > $out
wc -l < $out

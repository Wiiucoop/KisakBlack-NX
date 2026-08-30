#!/bin/sh
# Build the ffconv host tool (runs on the PC, not the Switch).
# Uses the msys2 ucrt64/mingw g++ + zlib. Run from an msys2 shell.
set -e
cd "$(dirname "$0")"
g++ -O2 -std=c++17 -Wall ffconv.cpp -o ffconv.exe -lz
echo "built ffconv.exe"

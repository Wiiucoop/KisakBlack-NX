#!/bin/sh
# Build the ffconv host tools (they run on the PC, not the Switch).
# Uses the msys2 UCRT64 g++ + zlib. Run from a UCRT64 shell.
#
#   convert.exe  .ff -> .kbz, the transcoder the port actually needs
#   ffconv.exe   .ff container inspector, for reading a zone by hand
#   kbzdump.exe  .kbz validator: the reference on-device relocator
set -e
cd "$(dirname "$0")"
g++ -O2 -std=c++17 -Wall convert.cpp -o convert.exe -lz
g++ -O2 -std=c++17 -Wall ffconv.cpp  -o ffconv.exe  -lz
g++ -O2 -std=c++17 -Wall kbzdump.cpp -o kbzdump.exe
echo "built convert.exe ffconv.exe kbzdump.exe"

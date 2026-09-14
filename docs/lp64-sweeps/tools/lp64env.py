"""Shared paths and toolchain locations for the LP64 sweep tools.

REPO is derived from this file's own location, so the sweep runs from any
checkout. Intermediates go to WORK (default build-nx/lp64-work) so that
nothing generated ever lands in the source tree; override with $LP64_WORK.
"""
import os

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.abspath(os.path.join(HERE, '..', '..', '..'))
WORK = os.environ.get('LP64_WORK', os.path.join(REPO, 'build-nx', 'lp64-work'))

DEVKITA64 = os.environ.get('DEVKITA64', '/opt/devkitpro/devkitA64/bin')
CXX = os.path.join(DEVKITA64, 'aarch64-none-elf-g++')
READELF = os.path.join(DEVKITA64, 'aarch64-none-elf-readelf')

# The CMake build writes the exact flags each TU is compiled with; the sweep
# reuses them verbatim so the probes see the same headers and defines the
# real build does.
FLAGS_MAKE = os.path.join(REPO, 'build-nx/CMakeFiles/KisakBlack.dir/flags.make')


def repo(*p):
    return os.path.join(REPO, *p)


def work(*p):
    os.makedirs(WORK, exist_ok=True)
    return os.path.join(WORK, *p)


def flag(name):
    """One variable out of flags.make, split into argv entries."""
    import re
    txt = open(FLAGS_MAKE).read()
    m = re.search(r'^' + name + r' = (.*)$', txt, re.M)
    return m.group(1).split() if m else []

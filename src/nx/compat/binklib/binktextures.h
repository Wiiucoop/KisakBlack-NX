// binktextures.h (compat shadow) -- pre-selects a RAD platform so the real
// binklib headers compile on AArch64 GCC, then chains to them.
//
// __RADLINUX__ wins the calling-convention branch (plain GCC attributes, and
// crucially does NOT define _WIN32 like the __RADNT__ branch would), while
// __RADWIN__/__RADNT__ unlock the D3D9 texture path, tex_draw, and the
// BinkOpenDirectSound declaration that r_cinematic.cpp uses. The Bink DLL
// itself does not exist on Switch: every Bink entry point is stubbed in
// src/nx/nx_bink_stubs.cpp (BinkOpen fails -> cinematics are skipped).
#ifndef NX_COMPAT_BINKTEXTURES_H
#define NX_COMPAT_BINKTEXTURES_H

#ifndef __RADLINUX__
#define __RADLINUX__
#endif
#ifndef __RADWIN__
#define __RADWIN__
#endif
#ifndef __RADNT__
#define __RADNT__
#endif
#ifndef __RAD32__
#define __RAD32__
#endif
#ifndef __RAD64__
#define __RAD64__
#endif
#ifndef __RADX64__
#define __RADX64__
#endif

#include <d3d9.h> // binklib's D3D9 texture path needs the types first

#include_next <binklib/binktextures.h>

#endif // NX_COMPAT_BINKTEXTURES_H

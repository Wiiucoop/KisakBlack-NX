// intrin.h -- MSVC intrinsics header shim; most mappings live in nx_prefix.h
// (force-included everywhere). This exists so `#include <intrin.h>` resolves.
#ifndef NX_COMPAT_INTRIN_H
#define NX_COMPAT_INTRIN_H
#include "nx_prefix.h"
#include "xmmintrin.h"
#endif

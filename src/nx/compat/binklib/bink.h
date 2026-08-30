// bink.h (compat shadow) -- same platform priming as binktextures.h for any
// TU that includes bink.h directly.
#ifndef NX_COMPAT_BINK_H
#define NX_COMPAT_BINK_H

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

#include_next <binklib/bink.h>

#endif // NX_COMPAT_BINK_H

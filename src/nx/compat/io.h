// io.h -- MSVC low-level io / _find* API over newlib dirent.
#ifndef NX_COMPAT_IO_H
#define NX_COMPAT_IO_H

#include <stdint.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

// MSVC _finddata definitions (32-bit time / 32-bit size variant used by the
// game via _findfirst64i32).
#define _A_NORMAL 0x00
#define _A_RDONLY 0x01
#define _A_HIDDEN 0x02
#define _A_SYSTEM 0x04
#define _A_SUBDIR 0x10
#define _A_ARCH   0x20

struct _finddata64i32_t {
    unsigned  attrib;
    long long time_create;
    long long time_access;
    long long time_write;
    unsigned int size;
    char      name[260];
};
struct _finddata_t {
    unsigned  attrib;
    long long time_create;
    long long time_access;
    long long time_write;
    unsigned int size;
    char      name[260];
};

intptr_t nx_findfirst64i32(const char *pattern, struct _finddata64i32_t *data);
int      nx_findnext64i32(intptr_t handle, struct _finddata64i32_t *data);
int      nx_findclose(intptr_t handle);
int      nx_access_compat(const char *path, int mode);
int      nx_unlink_compat(const char *path);

#ifdef __cplusplus
}
#endif

#define _findfirst64i32 nx_findfirst64i32
#define _findnext64i32  nx_findnext64i32
#define _findfirst(p, d) nx_findfirst64i32((p), (struct _finddata64i32_t *)(d))
#define _findnext(h, d)  nx_findnext64i32((h), (struct _finddata64i32_t *)(d))
#define _findclose      nx_findclose
#define _access         nx_access_compat
#define _unlink         nx_unlink_compat

#endif // NX_COMPAT_IO_H

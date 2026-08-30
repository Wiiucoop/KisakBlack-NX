// direct.h -- MSVC directory functions (_mkdir/_getcwd/...) over newlib.
#ifndef NX_COMPAT_DIRECT_H
#define NX_COMPAT_DIRECT_H

#ifdef __cplusplus
extern "C" {
#endif

int   nx_mkdir_compat(const char *path);
char *nx_getcwd_compat(char *buf, int size);
int   nx_chdir_compat(const char *path);
int   nx_rmdir_compat(const char *path);

#ifdef __cplusplus
}
#endif

#define _mkdir  nx_mkdir_compat
#define _getcwd nx_getcwd_compat
#define _chdir  nx_chdir_compat
#define _rmdir  nx_rmdir_compat

#endif // NX_COMPAT_DIRECT_H

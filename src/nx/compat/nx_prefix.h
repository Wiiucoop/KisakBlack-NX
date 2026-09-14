// nx_prefix.h -- force-included into every translation unit of the Switch
// build (see cmake/switch.cmake). Maps MSVC-specific keywords, fixed-width
// types, intrinsics and CRT extensions onto GCC/newlib equivalents so the
// (originally Windows/MSVC-only) sources compile unchanged on AArch64.
//
// Keep this header tiny and valid in both C and C++.
#ifndef NX_PREFIX_H
#define NX_PREFIX_H

#if defined(__SWITCH__) || defined(KISAK_NX)

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <strings.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <float.h>
#include <math.h>
#include <errno.h>
#include <setjmp.h>
#include <alloca.h>

// MSVC also accepts the single-underscore spelling
#define _declspec(x) __declspec(x)

// MSVC's FILE tag; newlib's is __sFILE
#define _iobuf __sFILE

// MSVC jmp_buf was int[16]; game code passes (int*) buffers around. The
// buffers themselves are enlarged to real jmp_bufs under KISAK_NX; this cast
// makes the (int*) call sites compile unchanged.
#ifdef __cplusplus
#define _setjmp(env) setjmp(*(jmp_buf *)(env))
// longjmp is called with (int*) in decompiled code; self-reference is safe
// (the inner token is not re-expanded).
#define longjmp(env, val) longjmp(*(jmp_buf *)(env), (val))
#endif

#define _putenv putenv

// SSE prefetch appears in TUs that never include xmmintrin.h
#ifndef _mm_prefetch
#define _mm_prefetch(p, hint) __builtin_prefetch((const void *)(p))
#endif

// ----- MSVC calling conventions / storage keywords ---------------------
#define __cdecl
#define __stdcall
#define __fastcall
#define __thiscall
#define __vectorcall

#ifndef __forceinline
#define __forceinline inline __attribute__((always_inline))
#endif

#define __unaligned
#define __w64
#define __single_inheritance
#define __multiple_inheritance

// __declspec(x) -> token-pasted per-argument mapping.
#define __nxdecl_align(x)   __attribute__((aligned(x)))
#define __nxdecl_noinline   __attribute__((noinline))
#define __nxdecl_noreturn   __attribute__((noreturn))
#define __nxdecl_deprecated __attribute__((deprecated))
#define __nxdecl_dllexport
#define __nxdecl_dllimport
#define __nxdecl_selectany  __attribute__((weak))
#define __nxdecl_novtable
#define __nxdecl_restrict
#define __nxdecl_naked
#define __nxdecl_thread     __thread
#define __declspec(x) __nxdecl_##x

// ----- MSVC sized integer keywords --------------------------------------
#define __int8  char
#define __int16 short
#define __int32 int
#define __int64 long long

// ----- MSVC intrinsics ---------------------------------------------------
#define __debugbreak() __builtin_trap()

#define _byteswap_ushort(x) __builtin_bswap16((uint16_t)(x))
#define _byteswap_ulong(x)  __builtin_bswap32((uint32_t)(x))
#define _byteswap_uint64(x) __builtin_bswap64((uint64_t)(x))

#ifndef _rotl
static inline unsigned int _rotl(unsigned int v, int s) { return (v << (s & 31)) | (v >> ((32 - s) & 31)); }
static inline unsigned int _rotr(unsigned int v, int s) { return (v >> (s & 31)) | (v << ((32 - s) & 31)); }
#endif

static inline unsigned char _BitScanReverse(unsigned int *index, unsigned int mask)
{
    if (mask == 0) return 0;
    *index = 31u - (unsigned int)__builtin_clz(mask);
    return 1;
}
static inline unsigned char _BitScanForward(unsigned int *index, unsigned int mask)
{
    if (mask == 0) return 0;
    *index = (unsigned int)__builtin_ctz(mask);
    return 1;
}
#ifdef __cplusplus
// MSVC prototypes take unsigned long*; some call sites use that spelling.
static inline unsigned char _BitScanReverse(unsigned long *index, unsigned long mask)
{
    if ((unsigned int)mask == 0) return 0;
    *index = 31ul - (unsigned long)__builtin_clz((unsigned int)mask);
    return 1;
}
static inline unsigned char _BitScanForward(unsigned long *index, unsigned long mask)
{
    if ((unsigned int)mask == 0) return 0;
    *index = (unsigned long)__builtin_ctz((unsigned int)mask);
    return 1;
}

// ----- MSVC _Interlocked* intrinsics ------------------------------------
// Win32 LONG semantics: always 32-bit, regardless of LP64 `long`.
// Overloads accept the int/long spellings that appear at call sites; all
// operate on 32 bits.
static inline int _InterlockedIncrement(volatile int *p)  { return __atomic_add_fetch(p, 1, __ATOMIC_SEQ_CST); }
static inline int _InterlockedDecrement(volatile int *p)  { return __atomic_sub_fetch(p, 1, __ATOMIC_SEQ_CST); }
static inline int _InterlockedExchange(volatile int *p, int v) { return __atomic_exchange_n(p, v, __ATOMIC_SEQ_CST); }
static inline int _InterlockedExchangeAdd(volatile int *p, int v) { return __atomic_fetch_add(p, v, __ATOMIC_SEQ_CST); }
static inline int _InterlockedCompareExchange(volatile int *p, int exchange, int comparand)
{
    int expected = comparand;
    __atomic_compare_exchange_n(p, &expected, exchange, 0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
    return expected;
}
static inline int _InterlockedIncrement(volatile unsigned int *p) { return (int)__atomic_add_fetch(p, 1u, __ATOMIC_SEQ_CST); }
static inline int _InterlockedDecrement(volatile unsigned int *p) { return (int)__atomic_sub_fetch(p, 1u, __ATOMIC_SEQ_CST); }
static inline int _InterlockedExchange(volatile unsigned int *p, unsigned int v) { return (int)__atomic_exchange_n(p, v, __ATOMIC_SEQ_CST); }
static inline int _InterlockedExchangeAdd(volatile unsigned int *p, unsigned int v) { return (int)__atomic_fetch_add(p, v, __ATOMIC_SEQ_CST); }
static inline int _InterlockedCompareExchange(volatile unsigned int *p, unsigned int exchange, unsigned int comparand)
{
    unsigned int expected = comparand;
    __atomic_compare_exchange_n(p, &expected, exchange, 0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
    return (int)expected;
}
// `volatile long*` spellings point at 32-bit objects in this codebase (Win32
// LONG); operate on the low 32 bits via cast.
static inline int _InterlockedIncrement(volatile long *p)  { return _InterlockedIncrement((volatile int *)p); }
static inline int _InterlockedDecrement(volatile long *p)  { return _InterlockedDecrement((volatile int *)p); }
static inline int _InterlockedExchange(volatile long *p, long v) { return _InterlockedExchange((volatile int *)p, (int)v); }
static inline int _InterlockedExchangeAdd(volatile long *p, long v) { return _InterlockedExchangeAdd((volatile int *)p, (int)v); }
static inline int _InterlockedCompareExchange(volatile long *p, long exchange, long comparand)
{
    return _InterlockedCompareExchange((volatile int *)p, (int)exchange, (int)comparand);
}
static inline long long _InterlockedCompareExchange64(volatile long long *p, long long exchange, long long comparand)
{
    long long expected = comparand;
    __atomic_compare_exchange_n(p, &expected, exchange, 0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
    return expected;
}
static inline long long _InterlockedExchange64(volatile long long *p, long long v) { return __atomic_exchange_n(p, v, __ATOMIC_SEQ_CST); }
static inline long long _InterlockedExchangeAdd64(volatile long long *p, long long v) { return __atomic_fetch_add(p, v, __ATOMIC_SEQ_CST); }
#endif // __cplusplus

static inline uint64_t __rdtsc(void)
{
    uint64_t cnt;
    __asm__ __volatile__("mrs %0, cntvct_el0" : "=r"(cnt));
    return cnt;
}

#define _ReturnAddress() __builtin_return_address(0)

// ----- CRT name differences ---------------------------------------------
#ifndef _stricmp
#define _stricmp   strcasecmp
#define stricmp    strcasecmp
#define _strnicmp  strncasecmp
#define strnicmp   strncasecmp
#define _strdup    strdup
#define _alloca    alloca
#define _snprintf  snprintf
#define _vsnprintf vsnprintf
#define _atoi64    atoll
#define _isnan(x)  __builtin_isnan(x)
#define _finite(x) __builtin_isfinite(x)
#endif

// ----- CRT rand() range --------------------------------------------------
// MSVC's RAND_MAX is 0x7FFF; newlib's is 0x7FFFFFFF (sys/config.h:261). The
// game was written against the MSVC contract and hardcodes it all over:
// com_math.cpp:213 divides by 32768, forty-odd call sites divide by 32767,
// cg_main.cpp:25 divides by 0x7FFF, RandWithSeed masks with 0x8000. Under
// newlib those expressions come out 65536x too large, and GaussianRandom
// (com_math.cpp:221) -- a Marsaglia polar rejection loop that retries until
// x*x + y*y <= 1 -- then exits with probability ~2e-10 per iteration, i.e.
// never: the engine hangs silently in R_CreateParticleCloudBuffer.
//
// Restore the MSVC contract once, here, instead of at every call site. The
// top 15 bits are taken rather than the low ones, so the result stays well
// mixed whichever generator newlib links in. RAND_MAX moves with it, which
// keeps the rand()/RAND_MAX idiom in third-party code correct as well
// (groupvoice/speex/misc.c:166).
static inline int nx_rand(void)
{
    return (int)(((unsigned int)rand() >> 16) & 0x7FFFu);
}
#define rand() nx_rand()
#ifdef __cplusplus
// libstdc++ calls std::rand() from inside template bodies (std::random_shuffle).
// The macro above rewrites that to std::nx_rand(), so make the name reachable
// there as well. Without this the error only appears once warnings are on, and
// only for a template nothing instantiates yet.
namespace std { using ::nx_rand; }
#endif
#undef RAND_MAX
#define RAND_MAX 0x7FFF

static inline char *_strlwr(char *s) { for (char *p = s; *p; ++p) *p = (char)tolower((unsigned char)*p); return s; }
static inline char *_strupr(char *s) { for (char *p = s; *p; ++p) *p = (char)toupper((unsigned char)*p); return s; }

static inline char *_itoa(int value, char *str, int radix)
{
    if (radix == 16)      sprintf(str, "%x", value);
    else if (radix == 8)  sprintf(str, "%o", value);
    else                  sprintf(str, "%d", value);
    return str;
}
#ifndef itoa
#define itoa _itoa
#endif

static inline char *_ui64toa(unsigned long long value, char *str, int radix)
{
    if (radix == 16)      sprintf(str, "%llx", value);
    else if (radix == 8)  sprintf(str, "%llo", value);
    else                  sprintf(str, "%llu", value);
    return str;
}
#define _strtoui64 strtoull
#define _strtoi64 strtoll
#define _errno() (&errno)

// MSVC qsort/bsearch comparator typedef
typedef int (*_CoreCrtNonSecureSearchSortCompareFunction)(const void *, const void *);

// MSVC aligned heap (tl_system.cpp)
#include <malloc.h> // memalign
static inline void *_aligned_malloc(size_t size, size_t alignment)
{
    if (alignment < sizeof(void *)) alignment = sizeof(void *);
    return memalign(alignment, size);
}
#define _aligned_free free
static inline void *_aligned_realloc(void *mem, size_t size, size_t alignment)
{
    // newlib realloc keeps malloc alignment (16 on AArch64); good enough for
    // the 16-byte requests tl_system makes.
    (void)alignment;
    return realloc(mem, size);
}

typedef int errno_t;

// ----- Secure-CRT (annex-K style, MSVC argument order) -------------------
#ifdef __cplusplus
extern "C++" {

static inline errno_t strcpy_s(char *dst, size_t size, const char *src)
{
    if (!dst || !src || size == 0) return 22 /*EINVAL*/;
    size_t n = strlen(src);
    if (n >= size) { dst[0] = 0; return 34 /*ERANGE*/; }
    memcpy(dst, src, n + 1);
    return 0;
}
template <size_t N> static inline errno_t strcpy_s(char (&dst)[N], const char *src) { return strcpy_s(dst, N, src); }

static inline errno_t strcat_s(char *dst, size_t size, const char *src)
{
    if (!dst || !src || size == 0) return 22;
    size_t d = strlen(dst), n = strlen(src);
    if (d + n >= size) return 34;
    memcpy(dst + d, src, n + 1);
    return 0;
}
template <size_t N> static inline errno_t strcat_s(char (&dst)[N], const char *src) { return strcat_s(dst, N, src); }

static inline errno_t strncpy_s(char *dst, size_t size, const char *src, size_t count)
{
    if (!dst || size == 0) return 22;
    if (!src) { dst[0] = 0; return 22; }
    size_t n = strnlen(src, count);
    if (n >= size) n = size - 1;
    memcpy(dst, src, n);
    dst[n] = 0;
    return 0;
}
template <size_t N> static inline errno_t strncpy_s(char (&dst)[N], const char *src, size_t count) { return strncpy_s(dst, N, src, count); }

static inline errno_t memcpy_s(void *dst, size_t dstSize, const void *src, size_t count)
{
    if (!dst || !src || count > dstSize) return 22;
    memcpy(dst, src, count);
    return 0;
}

static inline int vsnprintf_s(char *dst, size_t size, size_t count, const char *fmt, va_list args)
{
    (void)count;
    if (!dst || size == 0) return -1;
    int r = vsnprintf(dst, size, fmt, args);
    if (r < 0 || (size_t)r >= size) { dst[size - 1] = 0; return -1; }
    return r;
}

static inline int sprintf_s(char *dst, size_t size, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    int r = vsnprintf_s(dst, size, size ? size - 1 : 0, fmt, ap);
    va_end(ap);
    return r;
}
template <size_t N> static inline int sprintf_s(char (&dst)[N], const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    int r = vsnprintf_s(dst, N, N - 1, fmt, ap);
    va_end(ap);
    return r;
}

static inline errno_t fopen_s(FILE **f, const char *name, const char *mode)
{
    if (!f) return 22;
    *f = fopen(name, mode);
    return *f ? 0 : 2 /*ENOENT*/;
}

// wide-char fopen (CubeMapGenLib HDR export only); converts ASCII-range paths.
static inline errno_t _wfopen_s(FILE **f, const wchar_t *name, const wchar_t *mode)
{
    char nbuf[512], mbuf[16];
    unsigned i = 0, j = 0;
    if (!f) return 22;
    for (; name && name[i] && i < sizeof(nbuf) - 1; ++i) nbuf[i] = (char)name[i];
    nbuf[i] = 0;
    for (; mode && mode[j] && j < sizeof(mbuf) - 1; ++j) mbuf[j] = (char)mode[j];
    mbuf[j] = 0;
    *f = fopen(nbuf, mbuf);
    return *f ? 0 : 2;
}

static inline char *strtok_s(char *str, const char *delim, char **ctx) { return strtok_r(str, delim, ctx); }

// MSVC argument order: (tm*, const time_t*), returns errno_t.
static inline errno_t localtime_s(struct tm *out, const time_t *t)
{
    struct tm *r = localtime(t);
    if (!r) return 22;
    *out = *r;
    return 0;
}
static inline errno_t gmtime_s(struct tm *out, const time_t *t)
{
    struct tm *r = gmtime(t);
    if (!r) return 22;
    *out = *r;
    return 0;
}

typedef long long __time64_t;
static inline __time64_t _time64(__time64_t *t)
{
    time_t now = time(NULL);
    if (t) *t = (__time64_t)now;
    return (__time64_t)now;
}

static inline struct tm *_localtime64(const __time64_t *t)
{
    time_t tt = (time_t)*t;
    return localtime(&tt);
}
static inline struct tm *_gmtime64(const __time64_t *t)
{
    time_t tt = (time_t)*t;
    return gmtime(&tt);
}
static inline char *_ctime64(const __time64_t *t)
{
    time_t tt = (time_t)*t;
    return ctime(&tt);
}

// wide-char printf stubs (CubeMapGenLib error popups only)
static inline int _snwprintf_s(wchar_t *dst, size_t size, size_t, const wchar_t *, ...)
{
    if (dst && size) dst[0] = 0;
    return -1;
}
static inline int _vsnwprintf_s(wchar_t *dst, size_t size, size_t, const wchar_t *, va_list)
{
    if (dst && size) dst[0] = 0;
    return -1;
}

} // extern "C++"
#endif // __cplusplus

// ----- misc --------------------------------------------------------------
#ifndef _countof
#define _countof(a) (sizeof(a) / sizeof((a)[0]))
#endif

#endif // __SWITCH__ || KISAK_NX
#endif // NX_PREFIX_H

// xmmintrin.h -- SSE/MMX compatibility shim for AArch64.
//
// Implements exactly the intrinsics KisakBlack uses, on MSVC-style union
// types (the game accesses .m128_f32[] / .m64_u64 members directly).
// Scalar implementations; correctness over speed. The one semantic that
// matters for gameplay determinism: _mm_cvtss_si32 rounds to NEAREST-EVEN,
// which matches AArch64's default FP rounding via lrintf.
#ifndef NX_COMPAT_XMMINTRIN_H
#define NX_COMPAT_XMMINTRIN_H

#include <stdint.h>
#include <math.h>

typedef union __attribute__((aligned(16))) __m128 {
    float    m128_f32[4];
    uint64_t m128_u64[2];
    int8_t   m128_i8[16];
    int16_t  m128_i16[8];
    int32_t  m128_i32[4];
    int64_t  m128_i64[2];
    uint8_t  m128_u8[16];
    uint16_t m128_u16[8];
    uint32_t m128_u32[4];
} __m128;

typedef union __attribute__((aligned(8))) __m64 {
    uint64_t m64_u64;
    float    m64_f32[2];
    int8_t   m64_i8[8];
    int16_t  m64_i16[4];
    int32_t  m64_i32[2];
    int64_t  m64_i64;
    uint8_t  m64_u8[8];
    uint16_t m64_u16[4];
    uint32_t m64_u32[2];
} __m64;

#define _MM_SHUFFLE(fp3, fp2, fp1, fp0) \
    (((fp3) << 6) | ((fp2) << 4) | ((fp1) << 2) | (fp0))

#define _MM_HINT_NTA 0
#define _MM_HINT_T0  1
#define _MM_HINT_T1  2
#define _MM_HINT_T2  3

#ifndef _mm_prefetch // usually provided by nx_prefix.h
#define _mm_prefetch(p, hint) __builtin_prefetch((const void *)(p))
#endif

static inline __m128 _mm_set_ss(float w)
{
    __m128 r; r.m128_f32[0] = w; r.m128_f32[1] = 0; r.m128_f32[2] = 0; r.m128_f32[3] = 0; return r;
}
static inline __m128 _mm_set_ps(float z, float y, float x, float w)
{
    __m128 r; r.m128_f32[0] = w; r.m128_f32[1] = x; r.m128_f32[2] = y; r.m128_f32[3] = z; return r;
}
static inline __m128 _mm_set_ps1(float w)
{
    __m128 r; r.m128_f32[0] = w; r.m128_f32[1] = w; r.m128_f32[2] = w; r.m128_f32[3] = w; return r;
}
#define _mm_set1_ps _mm_set_ps1
static inline __m128 _mm_setzero_ps(void)
{
    __m128 r; r.m128_f32[0] = 0; r.m128_f32[1] = 0; r.m128_f32[2] = 0; r.m128_f32[3] = 0; return r;
}
static inline __m128 _mm_load_ps(const float *p)
{
    __m128 r; r.m128_f32[0] = p[0]; r.m128_f32[1] = p[1]; r.m128_f32[2] = p[2]; r.m128_f32[3] = p[3]; return r;
}
#define _mm_loadu_ps _mm_load_ps
static inline void _mm_store_ps(float *p, __m128 a)
{
    p[0] = a.m128_f32[0]; p[1] = a.m128_f32[1]; p[2] = a.m128_f32[2]; p[3] = a.m128_f32[3];
}
#define _mm_storeu_ps _mm_store_ps
static inline void _mm_stream_ps(float *p, __m128 a) { _mm_store_ps(p, a); }
static inline void _mm_stream_pi(__m64 *p, __m64 a)  { p->m64_u64 = a.m64_u64; }
static inline void _mm_empty(void) {}
#define _m_empty _mm_empty
static inline void _mm_sfence(void) { __atomic_thread_fence(__ATOMIC_SEQ_CST); }

static inline __m128 _mm_add_ps(__m128 a, __m128 b)
{
    __m128 r;
    for (int i = 0; i < 4; ++i) r.m128_f32[i] = a.m128_f32[i] + b.m128_f32[i];
    return r;
}
static inline __m128 _mm_sub_ps(__m128 a, __m128 b)
{
    __m128 r;
    for (int i = 0; i < 4; ++i) r.m128_f32[i] = a.m128_f32[i] - b.m128_f32[i];
    return r;
}
static inline __m128 _mm_mul_ps(__m128 a, __m128 b)
{
    __m128 r;
    for (int i = 0; i < 4; ++i) r.m128_f32[i] = a.m128_f32[i] * b.m128_f32[i];
    return r;
}
static inline __m128 _mm_div_ps(__m128 a, __m128 b)
{
    __m128 r;
    for (int i = 0; i < 4; ++i) r.m128_f32[i] = a.m128_f32[i] / b.m128_f32[i];
    return r;
}
static inline __m128 _mm_xor_ps(__m128 a, __m128 b)
{
    __m128 r;
    r.m128_u64[0] = a.m128_u64[0] ^ b.m128_u64[0];
    r.m128_u64[1] = a.m128_u64[1] ^ b.m128_u64[1];
    return r;
}
static inline __m128 _mm_min_ps(__m128 a, __m128 b)
{
    __m128 r;
    for (int i = 0; i < 4; ++i) r.m128_f32[i] = a.m128_f32[i] < b.m128_f32[i] ? a.m128_f32[i] : b.m128_f32[i];
    return r;
}
static inline __m128 _mm_max_ps(__m128 a, __m128 b)
{
    __m128 r;
    for (int i = 0; i < 4; ++i) r.m128_f32[i] = a.m128_f32[i] > b.m128_f32[i] ? a.m128_f32[i] : b.m128_f32[i];
    return r;
}

static inline __m128 _mm_movehl_ps(__m128 a, __m128 b)
{
    __m128 r;
    r.m128_f32[0] = b.m128_f32[2]; r.m128_f32[1] = b.m128_f32[3];
    r.m128_f32[2] = a.m128_f32[2]; r.m128_f32[3] = a.m128_f32[3];
    return r;
}
static inline __m128 _mm_movelh_ps(__m128 a, __m128 b)
{
    __m128 r;
    r.m128_f32[0] = a.m128_f32[0]; r.m128_f32[1] = a.m128_f32[1];
    r.m128_f32[2] = b.m128_f32[0]; r.m128_f32[3] = b.m128_f32[1];
    return r;
}
static inline __m128 _mm_unpackhi_ps(__m128 a, __m128 b)
{
    __m128 r;
    r.m128_f32[0] = a.m128_f32[2]; r.m128_f32[1] = b.m128_f32[2];
    r.m128_f32[2] = a.m128_f32[3]; r.m128_f32[3] = b.m128_f32[3];
    return r;
}
static inline __m128 _mm_unpacklo_ps(__m128 a, __m128 b)
{
    __m128 r;
    r.m128_f32[0] = a.m128_f32[0]; r.m128_f32[1] = b.m128_f32[0];
    r.m128_f32[2] = a.m128_f32[1]; r.m128_f32[3] = b.m128_f32[1];
    return r;
}
static inline __m128 _mm_shuffle_ps_impl(__m128 a, __m128 b, unsigned imm)
{
    __m128 r;
    r.m128_f32[0] = a.m128_f32[imm & 3];
    r.m128_f32[1] = a.m128_f32[(imm >> 2) & 3];
    r.m128_f32[2] = b.m128_f32[(imm >> 4) & 3];
    r.m128_f32[3] = b.m128_f32[(imm >> 6) & 3];
    return r;
}
#define _mm_shuffle_ps(a, b, imm) _mm_shuffle_ps_impl((a), (b), (unsigned)(imm))

// float -> int conversions; round-to-nearest-even (x86 default MXCSR mode,
// AArch64 default FPCR mode; lrintf honours it).
static inline int _mm_cvtss_si32(__m128 a) { return (int)lrintf(a.m128_f32[0]); }
static inline __m64 _mm_cvt_ps2pi(__m128 a)
{
    __m64 r;
    r.m64_i32[0] = (int)lrintf(a.m128_f32[0]);
    r.m64_i32[1] = (int)lrintf(a.m128_f32[1]);
    return r;
}
static inline __m128 _mm_cvtpu16_ps(__m64 a)
{
    __m128 r;
    for (int i = 0; i < 4; ++i) r.m128_f32[i] = (float)a.m64_u16[i];
    return r;
}
static inline __m128 _mm_cvtpu8_ps(__m64 a)
{
    __m128 r;
    for (int i = 0; i < 4; ++i) r.m128_f32[i] = (float)a.m64_u8[i];
    return r;
}
static inline __m128 _mm_cvtsi32_ss(__m128 a, int b)
{
    __m128 r = a; r.m128_f32[0] = (float)b; return r;
}
static inline float _mm_cvtss_f32(__m128 a) { return a.m128_f32[0]; }

static inline void _mm_pause(void) { __asm__ __volatile__("yield"); }

#endif // NX_COMPAT_XMMINTRIN_H

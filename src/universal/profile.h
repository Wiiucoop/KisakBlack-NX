#pragma once

#ifdef TRACY_ENABLE
#ifndef TRACY_ON_DEMAND
#error This should be left ON
#endif

#include <tracy/public/tracy/Tracy.hpp>
#include <tracy/public/tracy/TracyC.h>
//#include <tracy/Tracy.hpp>
//#include <tracy/TracyC.h>

#define PROF_SCOPED_RUNTIME_NAME(name) ZoneScoped; ZoneName(name, strlen(name));
#define PROF_SCOPED(name) ZoneScopedN(name)
#define PROFLOAD_SCOPED(name) PROF_SCOPED(name)
#define PROF_THREADNAME(threadname) tracy::SetThreadName(threadname)
#elif defined(KISAK_NX)

// Switch: no Tracy. Each PROF_SCOPED site keeps a per-thread tick total and call
// count (inclusive of nested scopes); the main thread's FrameMark prints the top
// sites every NX_PROF_REPORT_FRAMES frames (src/nx/nx_prof.cpp).
#include <stdint.h>

enum { NX_PROF_THREADS = 8 };

struct NxProfSite
{
    const char *name;
    NxProfSite *next;
    uint64_t ticks[NX_PROF_THREADS];
    uint32_t calls[NX_PROF_THREADS];
    int gpuScope;   // >= 0: also timed on the GPU (NxProf_GpuScopeName), -1 not
    explicit NxProfSite(const char *siteName);
};

int NxProf_ThreadSlot();
void NxProf_SetThreadName(const char *threadName);
void NxProf_FrameMark();
// GPU timestamps around the render scopes listed in nx_prof.cpp; implemented
// by the GL layer (nx_d3d9_null.cpp), which ignores calls off its thread.
void NxProf_GpuBegin(int scope);
void NxProf_GpuEnd(int scope);
int NxProf_GpuScopeCount();
const char *NxProf_GpuScopeName(int scope);

static inline uint64_t NxProf_Now()
{
    uint64_t t;
    __asm__ __volatile__("mrs %0, cntpct_el0" : "=r"(t));
    return t;
}

struct NxProfZone
{
    NxProfSite *site;
    uint64_t start;
    int slot;
    explicit NxProfZone(NxProfSite *s) : site(s), start(NxProf_Now()), slot(NxProf_ThreadSlot())
    {
        if ( site->gpuScope >= 0 )
            NxProf_GpuBegin(site->gpuScope);
    }
    ~NxProfZone()
    {
        if ( site->gpuScope >= 0 )
            NxProf_GpuEnd(site->gpuScope);
        if ( slot >= 0 )
        {
            site->ticks[slot] += NxProf_Now() - start;
            ++site->calls[slot];
        }
    }
};

#define NX_PROF_CAT2(a, b) a##b
#define NX_PROF_CAT(a, b) NX_PROF_CAT2(a, b)
#define PROF_SCOPED(name) \
    static NxProfSite NX_PROF_CAT(nxProfSite_, __LINE__)(name); \
    NxProfZone NX_PROF_CAT(nxProfZone_, __LINE__)(&NX_PROF_CAT(nxProfSite_, __LINE__))
#define PROF_SCOPED_RUNTIME_NAME(name)
#define PROFLOAD_SCOPED(name) PROF_SCOPED(name)
#define ZoneText(str, len)
#define ZoneTextF(fmt, ...)
#define ZoneName(str, len)
#define PROF_THREADNAME(threadname) NxProf_SetThreadName(threadname)
#define FrameMark NxProf_FrameMark()

#else

#define PROF_SCOPED(name) // Disable Profiling without Tracy
#define PROF_SCOPED_RUNTIME_NAME(name)
#define PROFLOAD_SCOPED(name) PROF_SCOPED(name)
#define ZoneText(str, len)
#define ZoneTextF(fmt, ...)
#define ZoneName(str, len)
#define PROF_THREADNAME(threadname)
#define FrameMark

#endif


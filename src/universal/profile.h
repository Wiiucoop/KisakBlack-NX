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
    explicit NxProfSite(const char *siteName);
};

int NxProf_ThreadSlot();
void NxProf_SetThreadName(const char *threadName);
void NxProf_FrameMark();

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
    explicit NxProfZone(NxProfSite *s) : site(s), start(NxProf_Now()), slot(NxProf_ThreadSlot()) {}
    ~NxProfZone()
    {
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


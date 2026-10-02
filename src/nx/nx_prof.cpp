// nx_prof.cpp -- the Switch side of PROF_SCOPED (universal/profile.h).
//
// Every PROF_SCOPED site the engine already carries (about 600, written for
// Tracy) adds its ticks and calls to a per-thread slot. Every
// NX_PROF_REPORT_FRAMES main-thread frames, FrameMark prints the sites that
// cost the most per frame since the last report, per thread. Times are
// inclusive: a scope's time contains the scopes nested in it.
#include <universal/profile.h>
#include <qcommon/threads.h>

#include <switch.h>

#include <algorithm>
#include <atomic>
#include <cstdio>
#include <cstring>
#include <mutex>
#include <unordered_map>
#include <vector>

namespace
{
constexpr int NX_PROF_REPORT_FRAMES = 600;
constexpr int NX_PROF_REPORT_ROWS = 40;

std::atomic<NxProfSite *> s_sites{nullptr};
std::atomic<int> s_nextSlot{0};
char s_threadNames[NX_PROF_THREADS][32];
thread_local int t_slot = -2;

struct Snapshot
{
    uint64_t ticks[NX_PROF_THREADS];
    uint32_t calls[NX_PROF_THREADS];
};
std::unordered_map<NxProfSite *, Snapshot> s_last;
uint64_t s_lastReportTick;
int s_frames;
uint64_t s_framesTotal;

struct Row
{
    const char *name;
    int slot;
    uint64_t ticks;
    uint32_t calls;
};
}

// The render scopes also timed on the GPU (the GL layer prints them with its
// summary). Whole frame first; anything not listed -- the HUD, the 2D --
// is the total less the parts.
static const char *const s_gpuScopes[] = {
    "RB_CallExecuteRenderCommands",
    "RB_StandardDrawCommands",
    "R_DepthPrepass",
    "Lit",
    "LitPostResolve",
    "Dynamic Lights",
    "Emissive",
    "postEmissiveBrightening",
    "RB_StandardPostEffects",
    "RB_ApplyLatePostEffects",
};

int NxProf_GpuScopeCount()
{
    return (int)(sizeof(s_gpuScopes) / sizeof(s_gpuScopes[0]));
}

const char *NxProf_GpuScopeName(int scope)
{
    return scope >= 0 && scope < NxProf_GpuScopeCount() ? s_gpuScopes[scope] : "?";
}

NxProfSite::NxProfSite(const char *siteName) : name(siteName), next(nullptr), ticks(), calls(), gpuScope(-1)
{
    for ( int i = 0; i < NxProf_GpuScopeCount(); ++i )
        if ( !strcmp(siteName, s_gpuScopes[i]) )
            gpuScope = i;
    NxProfSite *head = s_sites.load(std::memory_order_relaxed);
    do
        next = head;
    while ( !s_sites.compare_exchange_weak(head, this, std::memory_order_release, std::memory_order_relaxed) );
}

int NxProf_ThreadSlot()
{
    if ( t_slot == -2 )
    {
        const int slot = s_nextSlot.fetch_add(1);
        t_slot = slot < NX_PROF_THREADS ? slot : -1;
        if ( t_slot >= 0 && !s_threadNames[t_slot][0] )
            snprintf(s_threadNames[t_slot], sizeof(s_threadNames[t_slot]), "thread %d", t_slot);
    }
    return t_slot;
}

void NxProf_SetThreadName(const char *threadName)
{
    const int slot = NxProf_ThreadSlot();
    if ( slot >= 0 && threadName )
        snprintf(s_threadNames[slot], sizeof(s_threadNames[slot]), "%s", threadName);
}

static void NxProf_Report()
{
    const double ticksPerMs = (double)armGetSystemTickFreq() / 1000.0;
    const uint64_t now = armGetSystemTick();
    const double wallMs = (double)(now - s_lastReportTick) / ticksPerMs;

    std::vector<Row> rows;
    for ( NxProfSite *site = s_sites.load(std::memory_order_acquire); site; site = site->next )
    {
        Snapshot &last = s_last[site];
        for ( int slot = 0; slot < NX_PROF_THREADS; ++slot )
        {
            const uint64_t ticks = site->ticks[slot];
            const uint32_t calls = site->calls[slot];
            if ( ticks != last.ticks[slot] )
                rows.push_back({site->name, slot, ticks - last.ticks[slot], calls - last.calls[slot]});
            last.ticks[slot] = ticks;
            last.calls[slot] = calls;
        }
    }
    std::sort(rows.begin(), rows.end(), [](const Row &a, const Row &b) { return a.ticks > b.ticks; });

    printf("[nx-prof] main frames %llu..%llu: %.1f ms per frame; CPU per frame by scope, inclusive of nested scopes:\n",
           (unsigned long long)(s_framesTotal - s_frames + 1), (unsigned long long)s_framesTotal,
           wallMs / s_frames);
    const int n = (int)std::min<size_t>(rows.size(), NX_PROF_REPORT_ROWS);
    for ( int i = 0; i < n; ++i )
    {
        const Row &r = rows[i];
        printf("[nx-prof]   %7.2f ms  %7.1f calls  %-10s %s\n",
               (double)r.ticks / ticksPerMs / s_frames, (double)r.calls / s_frames, s_threadNames[r.slot], r.name);
    }
    s_lastReportTick = now;
    s_frames = 0;
}

void NxProf_FrameMark()
{
    const int slot = NxProf_ThreadSlot();
    if ( slot < 0 )
        return;
    if ( Sys_IsRenderThread() )
    {
        if ( !strncmp(s_threadNames[slot], "thread ", 7) )
            NxProf_SetThreadName("render");
        return;
    }
    if ( !Sys_IsMainThread() )
        return;
    if ( !strncmp(s_threadNames[slot], "thread ", 7) )
        NxProf_SetThreadName("main");
    if ( !s_lastReportTick )
    {
        s_lastReportTick = armGetSystemTick();
        return;
    }
    ++s_frames;
    ++s_framesTotal;
    if ( s_frames >= NX_PROF_REPORT_FRAMES )
        NxProf_Report();
}

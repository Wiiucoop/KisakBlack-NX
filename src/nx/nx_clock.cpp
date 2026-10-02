// nx_clock.cpp -- the GPU clock in handheld mode.
//
// Handheld runs the GPU at 307.2 MHz by default; the console also offers
// 384.0 and 460.8 MHz there, and games use them. The frame is GPU-bound in
// Zombies (~22 ms of GPU a frame at 307.2), so nx_gpuclock picks the handheld
// clock: 307, 384 or 460 (default), 0 to leave the system's. Docked, the
// system's 768 MHz is left alone. Docking and undocking reset the clock, so it
// is checked again every couple of seconds. Only the system default (307.2) is
// replaced: a clock set by sys-clk or another tool is left as it is. The clock
// found the first time it was changed goes back on exit.
#include "nx_clock.h"

#include <switch.h>

#include <cstdio>

namespace
{
constexpr u32 NX_HANDHELD_DEFAULT_HZ = 307200000;   // what the system sets undocked

bool s_useClkrst, s_usePcv;
ClkrstSession s_gpuSession;
u32 s_originalHz;        // the handheld clock before the first change, 0 before it
u64 s_lastCheckTick;
u32 s_lastReportedHz;

bool nxClockGet(u32 *hz)
{
    if ( s_useClkrst )
        return R_SUCCEEDED(clkrstGetClockRate(&s_gpuSession, hz));
    if ( s_usePcv )
        return R_SUCCEEDED(pcvGetClockRate(PcvModule_GPU, hz));
    return false;
}

bool nxClockSet(u32 hz)
{
    if ( s_useClkrst )
        return R_SUCCEEDED(clkrstSetClockRate(&s_gpuSession, hz));
    if ( s_usePcv )
        return R_SUCCEEDED(pcvSetClockRate(PcvModule_GPU, hz));
    return false;
}

u32 nxClockHandheldHz(int mhz)
{
    if ( mhz <= 0 )
        return 0;
    if ( mhz <= 307 )
        return 307200000;
    if ( mhz <= 384 )
        return 384000000;
    return 460800000;
}
}

void NX_ClockInit()
{
    if ( hosversionAtLeast(8, 0, 0) )
    {
        if ( R_SUCCEEDED(clkrstInitialize()) )
        {
            if ( R_SUCCEEDED(clkrstOpenSession(&s_gpuSession, PcvModuleId_GPU, 3)) )
                s_useClkrst = true;
            else
                clkrstExit();
        }
    }
    else if ( R_SUCCEEDED(pcvInitialize()) )
    {
        s_usePcv = true;
    }
    printf("[nx-clock] GPU clock control %s\n",
           s_useClkrst ? "through clkrst" : s_usePcv ? "through pcv" : "unavailable");
}

void NX_ClockUpdate(int handheldMhz)
{
    if ( !s_useClkrst && !s_usePcv )
        return;
    const u64 now = armGetSystemTick();
    if ( s_lastCheckTick && armTicksToNs(now - s_lastCheckTick) < 2000000000ull )
        return;
    s_lastCheckTick = now;
    if ( appletGetOperationMode() != AppletOperationMode_Handheld )
        return;
    const u32 want = nxClockHandheldHz(handheldMhz);
    if ( !want )
        return;
    u32 current = 0;
    if ( !nxClockGet(&current) || current == want )
        return;
    // Only the system's own handheld clock is replaced. Anything else was set
    // by someone -- sys-clk, an overclock tool -- and that choice stands.
    if ( current != NX_HANDHELD_DEFAULT_HZ )
        return;
    if ( !s_originalHz )
        s_originalHz = current;
    if ( nxClockSet(want) && want != s_lastReportedHz )
    {
        printf("[nx-clock] handheld GPU clock %.1f -> %.1f MHz\n", current / 1e6, want / 1e6);
        s_lastReportedHz = want;
    }
}

void NX_ClockExit()
{
    // Put back only our own change: a clock another tool set since stays.
    u32 current = 0;
    if ( s_originalHz && appletGetOperationMode() == AppletOperationMode_Handheld && nxClockGet(&current)
        && current == s_lastReportedHz )
        nxClockSet(s_originalHz);
    if ( s_useClkrst )
    {
        clkrstCloseSession(&s_gpuSession);
        clkrstExit();
    }
    if ( s_usePcv )
        pcvExit();
    s_useClkrst = s_usePcv = false;
}

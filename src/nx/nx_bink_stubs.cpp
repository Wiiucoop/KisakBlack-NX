// nx_bink_stubs.cpp -- Bink video stubs (binkw32.dll does not exist on
// Switch). BinkOpen always fails, so r_cinematic's error paths skip video
// playback cleanly. Signatures match src/binklib/bink.h exactly.
#include <binklib/bink.h>

#include <string.h>

RADDEFFUNC char PTR4 *RADEXPLINK BinkGetError(void)
{
    static char err[] = "Bink unavailable on Switch";
    return err;
}

RADDEFFUNC HBINK RADEXPLINK BinkOpen(const char PTR4 *, U32) { return 0; }
RADDEFFUNC void RADEXPLINK BinkClose(HBINK) {}
RADDEFFUNC S32 RADEXPLINK BinkDoFrame(HBINK) { return 0; }
RADDEFFUNC void RADEXPLINK BinkNextFrame(HBINK) {}
RADDEFFUNC S32 RADEXPLINK BinkWait(HBINK) { return 0; }
RADDEFFUNC S32 RADEXPLINK BinkShouldSkip(HBINK) { return 0; }
RADDEFFUNC S32 RADEXPLINK BinkPause(HBINK, S32) { return 0; }
RADDEFFUNC void RADEXPLINK BinkGetRealtime(HBINK, BINKREALTIME PTR4 *rt, U32)
{
    if (rt) memset(rt, 0, sizeof(*rt));
}
RADDEFFUNC S32 RADEXPLINK BinkControlBackgroundIO(HBINK, U32) { return 0; }
RADDEFFUNC void RADEXPLINK BinkSetMemory(BINKMEMALLOC, BINKMEMFREE) {}
RADDEFFUNC S32 RADEXPLINK BinkSetSoundSystem(BINKSNDSYSOPEN, UINTa) { return 0; }
RADDEFFUNC BINKSNDOPEN RADEXPLINK BinkOpenDirectSound(UINTa) { return 0; }
RADDEFFUNC void RADEXPLINK BinkSetSoundTrack(U32, U32 PTR4 *) {}
RADDEFFUNC void RADEXPLINK BinkSetVolume(HBINK, U32, S32) {}
RADDEFFUNC void RADEXPLINK BinkSetIOSize(U32) {}
RADDEFFUNC void RADEXPLINK BinkGetFrameBuffersInfo(HBINK, BINKFRAMEBUFFERS *fb)
{
    if (fb) memset(fb, 0, sizeof(*fb));
}
RADDEFFUNC void RADEXPLINK BinkRegisterFrameBuffers(HBINK, BINKFRAMEBUFFERS *) {}
RADDEFFUNC S32 RADEXPLINK BinkGetRects(HBINK, U32) { return 0; }

// nx_snd_null.cpp -- null sound driver for the Switch build, replacing
// snd_driver_xaudio2.cpp / snd_driver_xaudio2_dsp.cpp (excluded).
//
// IMPORTANT: SD_Xaudio2CanInit MUST return true and SD_Init MUST succeed --
// Sys_StreamSleep blocks the shared stream thread on sndInitializedEvent,
// which only gets set when the sound system initializes; killing sound init
// would also kill texture streaming.
#include <sound/snd_driver_xaudio2.h>
#include <sound/snd.h>

extern "C++" {

bool __cdecl SD_Xaudio2CanInit()
{
    return true;
}

char __cdecl SD_Init()
{
    return 1;
}

void SD_Shutdown() {}

void __cdecl SD_TruncateAudioDeviceNames(Font_s *, float, int) {}

// A started voice reports "not played"; the engine reclaims the slot itself.
int __cdecl SD_StartAlias(SndStartAliasInfo *, unsigned int voice)
{
    return SND_SetPlaybackIdNotPlayed(voice);
}

void __cdecl SD_StopVoice(int) {}
void __cdecl SD_UpdateVoice(unsigned int) {}
void __cdecl SD_PreUpdate() {}
void __cdecl SD_PauseVoice(int) {}
void __cdecl SD_UnpauseVoice(int) {}

} // extern "C++"

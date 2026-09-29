#pragma once
#include <d3d9.h>
#include "r_material.h"
#include <binklib/binktextures.h>

#define CINEMATIC_INVALID_IMAGE_FRAME -1
#define BINK_MISC_BUFFER_SIZE 1572864

enum CIN_IOSTATE : __int32
{                                       // XREF: CinematicGlob/r
    CIN_IOSTATE_RELINQUISHED = 0x0,
    CIN_IOSTATE_SEIZED       = 0x1,
    CIN_IOSTATE_COUNT        = 0x2,
};

enum CinematicEnum : __int32
{                                       // XREF: CinematicGlob/r
                                        // CinematicGlob/r ...
    CINEMATIC_NOT_PAUSED    = 0x0,
    CINEMATIC_PAUSED        = 0x1,
    CINEMATIC_SCRIPT_PAUSED = 0x2,
};

struct CINE_JQ_CMD // sizeof=0x4
{                                       // XREF: ?R_Cinematic_UpdateFrame@@YAX_N@Z/r
    int dummy;
};

struct __declspec(align(4)) CinematicHunk // sizeof=0x18
{                                       // XREF: CinematicGlob/r
                                        // CinematicGlob/r ...
    void *base;                         // XREF: R_Cinematic_Advance+2EE/r
                                        // R_Cinematic_InitBinkTextures+165/w ...
    void *atFront;                      // XREF: R_Cinematic_AreHunksOpen+3/o
                                        // R_Cinematic_HunksClose+3/o ...
    void *atBack;                       // XREF: R_Cinematic_Advance+30A/r
                                        // R_Cinematic_InitBinkTextures+227/w
    void *end;
    void *lastAllocPtr;                 // XREF: R_Cinematic_Advance+2D2/r
                                        // R_Cinematic_Advance+32F/r ...
    bool fragmented;
    // padding byte
    // padding byte
    // padding byte
};

struct CinematicTextureSet // sizeof=0x290
{
    GfxImage imageY[2];
    GfxImage imageCr[2];
    GfxImage imageCb[2];
    GfxImage imageA[2];
    GfxImage drawImageY;
    GfxImage drawImageCr;
    GfxImage drawImageCb;
    GfxImage drawImageA;
    void *Ytexture[2];
    void *cRtexture[2];
    void *cBtexture[2];
    void *Atexture[2];
};

void __cdecl R_CinematicInitSound(const _GUID *guid);
void __cdecl Unlock_Bink_textures2(IDirect3DDevice9 *d3d_device, BINKTEXTURESET *set_textures);
void __cdecl Unlock_Bink_textures_Callback();
void __cdecl R_Cinematic_BeginLostDevice();
void R_Cinematic_RelinquishIO();
const char *R_Cinematic_CheckBinkError();
void __cdecl R_Cinematic_EndLostDevice();
void R_Cinematic_MakeBinkDrawTextures();
IDirect3DTexture9 *__cdecl R_Cinematic_MakeBinkTexture_PC(
                GfxImage *image,
                unsigned int width,
                unsigned int height,
                int baseImageFlags);
void R_Cinematic_ClearBinkDrawTextures();
void __cdecl R_Cinematic_ClearTexture(IDirect3DTexture9 *texture, int width, int height, unsigned __int8 clearValue);
void __cdecl R_Cinematic_Init();
unsigned int *R_Cinematic_ReserveMemory();
void __cdecl R_Cinematic_Shutdown();
void __cdecl R_Cinematic_ReleaseImages(CinematicTextureSet *textureSet);
void __cdecl R_Cinematic_StartPlayback_Internal(const char *name, unsigned int playbackFlags, float volume);
void __cdecl R_Cinematic_StartPlayback(const char *name, unsigned int playbackFlags, float volume);
// SP queue producer, retail 0x006DA060. This writes the NEXT slot; the three-argument
// R_Cinematic_StartPlayback above writes the current target slot.
void __cdecl R_Cinematic_SetNextPlayback(const char *name, unsigned int playbackFlags);
void __cdecl R_Cinematic_StartNextPlayback();
bool __cdecl R_Cinematic_IsNextReady_Internal();
// SP-only addition (Ghidra 0x006da230) -- see r_cinematic.cpp for the full evidence note. Tests
// cinematicGlob.targetCinematicName[0], distinct from R_Cinematic_IsNextReady_Internal above
// (which tests nextCinematicName). Descriptive name, not a confirmed retail symbol.
bool __cdecl R_Cinematic_IsTargetSet();
#ifdef KISAK_SP
// Reconstruction names for reviewed SP behavior; not recovered retail symbols.
float R_Cinematic_GetRemainingSeconds_SP();
bool R_Cinematic_ConsumeFirstFrameNotify_SP();
bool R_Cinematic_ConsumeLastFrameNotify_SP();
#endif
// SP-only addition (Ghidra 0x006da140) -- see r_cinematic.cpp for the full evidence note. A
// one-line Sys_WaitWorkerCmdInternal(&_UpdateFrameWorkerCmd) bracket, used by SP's cinematic
// consumers around StartNextPlayback+UpdateFrame. Descriptive name, not a confirmed retail symbol.
void __cdecl R_Cinematic_WaitForUpdateFrame();
void __cdecl R_Cinematic_StopPlayback();
// SP-only additions (Ghidra 0x006d9f90 / 0x006d9fe0) -- see r_cinematic.cpp for the full
// evidence note. fromScript selects the stronger CINEMATIC_SCRIPT_PAUSED state, which only
// a fromScript resume (or an unconditional fromScript=true resume) can release.
void __cdecl R_Cinematic_Pause(bool fromScript);
void __cdecl R_Cinematic_Resume(bool fromScript);
int __cdecl _UpdateFrameCallback();
void __cdecl R_Cinematic_UpdateFrame_Core(bool force_wait);
char __cdecl R_Cinematic_AreHunksOpen();
char __cdecl CinematicHunk_IsOpen(CinematicHunk *hunk);
void R_Cinematic_HunksClose();
void __cdecl CinematicHunk_Close(CinematicHunk *hunk);
char __cdecl R_Cinematic_Advance(bool force_wait);
unsigned int __cdecl R_Cinematic_GetPercentageFull();
const char *R_Cinematic_SeizeIO();
void __cdecl R_Cinematic_UpdateTimeInMsec(const BINKREALTIME *binkRealtime);
int __cdecl R_Cinematic_BinkShouldSkip(bool useCustomLogic);
int __cdecl R_Cinematic_BinkShouldWait(bool useCustomLogic);
void R_Cinematic_StopPlayback_Now();
void __cdecl CinematicHunk_Reset(CinematicHunk *hunk);
char __cdecl R_Cinematic_StartPlayback_Now(const char *filename, unsigned int playbackFlags);
bool __cdecl CinematicHunk_IsEmpty(CinematicHunk *hunk);
char __cdecl R_Cinematic_HunksOpen(int activeTexture, char playbackFlags);
void __cdecl CinematicHunk_Open(CinematicHunk *hunk, char *memory, int size);
void __cdecl R_Cinematic_HunksAllocate(int activeTexture, char playbackFlags);
int __cdecl CinematicHunk_Alloc(CinematicHunk *hunk, int size);
void __cdecl R_Cinematic_HunksReset(int activeTexture, char playbackFlags);
void R_Cinematic_InitBinkTextures();
void __cdecl R_Cinematic_BlackRendererImages();
void *__stdcall R_Cinematic_Bink_Alloc(unsigned int bytes);
void __stdcall R_Cinematic_Bink_Free(void *ptr);
bool __cdecl R_Cinematic_BinkOpen(const char *filename, char playbackFlags, char *errText, unsigned int errTextSize);
char __cdecl R_Cinematic_BinkOpenPath(const char *filepath, char playbackFlags, char *errText, unsigned int errTextSize);
int __cdecl CinematicHunk_GetFreeSpace(CinematicHunk *hunk);
void __cdecl R_Cinematic_InitBinkVolumes();
void __cdecl R_Cinematic_UpdateFrame(bool force_wait);
void R_Cinematic_UpdateRendererImages();
void __cdecl R_Cinematic_SetRendererImagesToFrame(int frameToSetTo);
char __cdecl R_Cinematic_GetFilenameAndTimeInMsec(char *outName, int outNameSize, unsigned int *outTimeInMsec);
bool __cdecl R_Cinematic_IsFinished();
bool __cdecl R_Cinematic_IsStarted();
bool __cdecl R_Cinematic_IsNextReady();
char __cdecl R_Cinematic_IsUnderrun();
void __cdecl R_Cinematic_ForceRelinquishIO();

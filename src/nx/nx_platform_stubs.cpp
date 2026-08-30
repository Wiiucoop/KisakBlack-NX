// nx_platform_stubs.cpp -- Switch replacements for excluded Windows-only
// modules: system console window (win_syscon.cpp), splash window
// (win_splash.cpp), minidumper (win_mini_dumper.cpp), voice/mixer transport
// (win_voice.cpp + groupvoice DSound backends), plus small library stubs
// (dxerr, d3dx, ddraw, dsound, nvapi, vpx wrapper).
#include <windows.h>

#include <stdio.h>
#include <string.h>

// ===========================================================================
// win_syscon.cpp replacements -- external console; print to stdout instead
// ===========================================================================
static char s_errorText[512];

extern "C++" {

void __cdecl Sys_CreateConsole(HINSTANCE__ *) {}
void __cdecl Sys_DestroyConsole() {}
void __cdecl Sys_ShowConsole() {}

char *__cdecl Sys_ConsoleInput()
{
    return NULL;
}

char *__cdecl Conbuf_CleanText(const char *source, char *target, int sizeofTarget)
{
    if (!source || !target || sizeofTarget <= 0) return target;
    int j = 0;
    for (int i = 0; source[i] && j < sizeofTarget - 1; ++i) {
        char c = source[i];
        if (c == '\r') continue;
        target[j++] = c;
    }
    target[j] = 0;
    return target;
}

void __cdecl Conbuf_AppendText(char *pMsg)
{
    if (pMsg) fputs(pMsg, stdout);
}

void __cdecl Conbuf_AppendTextInMainThread(char *msg)
{
    Conbuf_AppendText(msg);
}

void __cdecl Sys_SetErrorText(const char *buf)
{
    if (buf) {
        strncpy(s_errorText, buf, sizeof(s_errorText) - 1);
        fprintf(stderr, "SYS ERROR: %s\n", buf);
    }
}

// ===========================================================================
// win_splash.cpp replacements
// ===========================================================================
void __cdecl Sys_DestroySplashWindow() {}
void __cdecl Sys_HideSplashWindow() {}

// ===========================================================================
// win_mini_dumper.cpp replacements
// ===========================================================================
void __cdecl Sys_StartMiniDump(bool) {}
bool __cdecl Sys_IsMiniDumpStarted() { return false; }

// ===========================================================================
// groupvoice DSound backends (play_dsound.cpp / record_dsound.cpp excluded)
// ===========================================================================
struct dsound_sample_t;

int __cdecl DSound_GetBytesLeft(dsound_sample_t *) { return 0; }
unsigned int __cdecl DSound_UpdateSample(dsound_sample_t *, char *, signed int) { return 0; }
void __cdecl DSound_AdjustSamplePlayback(dsound_sample_t *, int) {}
bool __cdecl DSound_BufferUnderrunOccurred(dsound_sample_t *) { return false; }
void __cdecl DSound_HandleBufferUnderrun(dsound_sample_t *) {}
void __cdecl DSound_SampleFrame(dsound_sample_t *) {}
dsound_sample_t *__cdecl DSound_NewSample() { return NULL; }
char __cdecl DSound_StopSample(dsound_sample_t *) { return 0; }
int __cdecl DSound_Init(bool, HWND__ *) { return 0; }
void __cdecl DSound_Shutdown() {}
void __cdecl DSOUNDRecord_UpdateSample(dsound_sample_t *) {}
void __cdecl DSOUNDRecord_Frame() {}
dsound_sample_t *__cdecl DSOUNDRecord_NewSample() { return NULL; }
int __cdecl DSOUNDRecord_DestroySample(dsound_sample_t *) { return 0; }
HRESULT __cdecl DSOUNDRecord_Start(dsound_sample_t *) { return E_FAIL; }
HRESULT __cdecl DSOUNDRecord_Stop(dsound_sample_t *) { return E_FAIL; }
int __cdecl DSOUNDRecord_Init(bool) { return 0; }
void __cdecl DSOUNDRecord_Shutdown() {}

// ===========================================================================
// win_voice.cpp replacements (voice chat disabled on Switch)
// ===========================================================================
float voice_current_scaler = 1.0f;

bool __cdecl Voice_Init() { return false; }
void __cdecl Voice_StopClientSamples() {}
void __cdecl Voice_Shutdown() {}
double __cdecl Voice_GetVoiceLevel() { return 0.0; }
void __cdecl Voice_Playback() {}
int __cdecl Voice_GetLocalVoiceData() { return 0; }
void __cdecl Voice_IncomingVoiceData(unsigned char, unsigned char *, int) {}
bool __cdecl Voice_IsClientTalking(unsigned int) { return false; }
char __cdecl Voice_StartRecording() { return 0; }
char __cdecl Voice_StopRecording() { return 0; }
unsigned int __cdecl mixerGetRecordLevel(char *) { return 0; }
int __cdecl mixerSetRecordLevel(char *, unsigned short) { return 0; }
int __cdecl mixerGetRecordSource(char *) { return 0; }
int __cdecl mixerSetRecordSource(char *) { return 0; }
int __cdecl mixerSetMicrophoneMute(unsigned char) { return 0; }

// ===========================================================================
// dxerr
// ===========================================================================
const char *DXGetErrorDescriptionA(HRESULT hr)
{
    static char buf[48];
    snprintf(buf, sizeof(buf), "HRESULT 0x%08X", (unsigned)hr);
    return buf;
}

const char *DXGetErrorStringA(HRESULT hr)
{
    return DXGetErrorDescriptionA(hr);
}

// ===========================================================================
// ddraw (video-memory probe; only reachable through LoadLibrary, which fails)
// ===========================================================================
extern const GUID IID_IDirectDraw7; const GUID IID_IDirectDraw7 = { 0x15e65ec0, 0x3b9c, 0x11d2, { 0xb9, 0x2f, 0x00, 0x60, 0x97, 0x97, 0xea, 0x5b } };

#include "compat/ddraw.h"

HRESULT IDirectDraw7::SetCooperativeLevel(HWND, DWORD) { return DD_OK; }
HRESULT IDirectDraw7::GetAvailableVidMem(LPDDSCAPS2, LPDWORD total, LPDWORD free_)
{
    if (total) *total = 256 * 1024 * 1024;
    if (free_) *free_ = 256 * 1024 * 1024;
    return DD_OK;
}
ULONG IDirectDraw7::AddRef() { return 1; }
ULONG IDirectDraw7::Release() { return 0; }

HRESULT DirectDrawCreateEx(GUID *, LPVOID *dd, REFIID, void *)
{
    if (dd) *dd = NULL;
    return E_FAIL;
}

// ===========================================================================
// dsound (cinematic audio + voice transport; always fails cleanly)
// ===========================================================================
#include "compat/dsound.h"

HRESULT DirectSoundCreate8(const GUID *, LPDIRECTSOUND8 *ds, void *)
{
    if (ds) *ds = NULL;
    return DSERR_GENERIC;
}

HRESULT DirectSoundCaptureCreate(const GUID *, LPDIRECTSOUNDCAPTURE *dsc, void *)
{
    if (dsc) *dsc = NULL;
    return DSERR_GENERIC;
}

HRESULT IDirectSound8::SetCooperativeLevel(HWND, DWORD) { return DS_OK; }
HRESULT IDirectSound8::CreateSoundBuffer(LPCDSBUFFERDESC, LPDIRECTSOUNDBUFFER *buffer, void *)
{
    if (buffer) *buffer = NULL;
    return DSERR_GENERIC;
}
ULONG IDirectSound8::AddRef() { return 1; }
ULONG IDirectSound8::Release() { return 0; }

HRESULT IDirectSoundBuffer::Lock(DWORD, DWORD, LPVOID *p1, LPDWORD n1, LPVOID *p2, LPDWORD n2, DWORD)
{
    if (p1) *p1 = NULL;
    if (n1) *n1 = 0;
    if (p2) *p2 = NULL;
    if (n2) *n2 = 0;
    return DSERR_GENERIC;
}
HRESULT IDirectSoundBuffer::Unlock(LPVOID, DWORD, LPVOID, DWORD) { return DS_OK; }
HRESULT IDirectSoundBuffer::Play(DWORD, DWORD, DWORD) { return DSERR_GENERIC; }
HRESULT IDirectSoundBuffer::Stop() { return DS_OK; }
HRESULT IDirectSoundBuffer::GetCurrentPosition(LPDWORD play, LPDWORD write)
{
    if (play) *play = 0;
    if (write) *write = 0;
    return DSERR_GENERIC;
}
HRESULT IDirectSoundBuffer::SetFrequency(DWORD) { return DS_OK; }
HRESULT IDirectSoundBuffer::GetStatus(LPDWORD status)
{
    if (status) *status = 0;
    return DS_OK;
}
HRESULT IDirectSoundBuffer::SetVolume(LONG) { return DS_OK; }
ULONG IDirectSoundBuffer::AddRef() { return 1; }
ULONG IDirectSoundBuffer::Release() { return 0; }

HRESULT IDirectSoundCapture::CreateCaptureBuffer(LPCDSCBUFFERDESC, LPDIRECTSOUNDCAPTUREBUFFER *buffer, void *)
{
    if (buffer) *buffer = NULL;
    return DSERR_GENERIC;
}
ULONG IDirectSoundCapture::AddRef() { return 1; }
ULONG IDirectSoundCapture::Release() { return 0; }

HRESULT IDirectSoundCaptureBuffer::Start(DWORD) { return DSERR_GENERIC; }
HRESULT IDirectSoundCaptureBuffer::Stop() { return DS_OK; }
HRESULT IDirectSoundCaptureBuffer::Lock(DWORD, DWORD, LPVOID *p1, LPDWORD n1, LPVOID *p2, LPDWORD n2, DWORD)
{
    if (p1) *p1 = NULL;
    if (n1) *n1 = 0;
    if (p2) *p2 = NULL;
    if (n2) *n2 = 0;
    return DSERR_GENERIC;
}
HRESULT IDirectSoundCaptureBuffer::Unlock(LPVOID, DWORD, LPVOID, DWORD) { return DS_OK; }
HRESULT IDirectSoundCaptureBuffer::GetCurrentPosition(LPDWORD cap, LPDWORD rd)
{
    if (cap) *cap = 0;
    if (rd) *rd = 0;
    return DSERR_GENERIC;
}
ULONG IDirectSoundCaptureBuffer::AddRef() { return 1; }
ULONG IDirectSoundCaptureBuffer::Release() { return 0; }

// ===========================================================================
// nvapi
// ===========================================================================
#include "compat/nvapi/nvapi.h"

NvAPI_Status NvAPI_Initialize(void) { return NVAPI_ERROR; }
NvAPI_Status NvAPI_Unload(void) { return NVAPI_OK; }
NvAPI_Status NvAPI_GetDisplayDriverVersion(void *, NV_DISPLAY_DRIVER_VERSION *) { return NVAPI_ERROR; }
NvAPI_Status NvAPI_Stereo_CreateHandleFromIUnknown(void *, StereoHandle *handle)
{
    if (handle) *handle = NULL;
    return NVAPI_ERROR;
}
NvAPI_Status NvAPI_Stereo_DestroyHandle(StereoHandle) { return NVAPI_OK; }
NvAPI_Status NvAPI_Stereo_IsActivated(StereoHandle, NvU8 *activated)
{
    if (activated) *activated = 0;
    return NVAPI_ERROR;
}
NvAPI_Status NvAPI_Stereo_IsEnabled(NvU8 *enabled)
{
    if (enabled) *enabled = 0;
    return NVAPI_ERROR;
}
NvAPI_Status NvAPI_Stereo_SetConvergence(StereoHandle, float) { return NVAPI_ERROR; }
NvAPI_Status NvAPI_D3D9_GetTextureHandle(void *, NVDX_ObjectHandle *handle)
{
    if (handle) *handle = NVDX_OBJECT_NONE;
    return NVAPI_ERROR;
}
NvAPI_Status NvAPI_D3D9_GetCurrentZBufferHandle(void *, NVDX_ObjectHandle *handle)
{
    if (handle) *handle = NVDX_OBJECT_NONE;
    return NVAPI_ERROR;
}
NvAPI_Status NvAPI_D3D9_StretchRect(void *, NVDX_ObjectHandle, const RECT *, NVDX_ObjectHandle, const RECT *, int)
{
    return NVAPI_ERROR;
}
NvAPI_Status NvAPI_D3D_GetCurrentSLIState(void *, NV_GET_CURRENT_SLI_STATE *) { return NVAPI_ERROR; }

// ===========================================================================
// vpx wrapper (src/vpx/vpx.cpp excluded; VP8 clip recording disabled)
// ===========================================================================
void __cdecl vpx_init(const char *, int, int) {}
void __cdecl vpx_encode_frame(unsigned char *, unsigned char *, unsigned char *, bool) {}
void __cdecl vpx_shutdown() {}

// win_voice.h declares this; the rest of win_voice.cpp is excluded
struct SessionData_s;
int __cdecl Live_GetClientNumForXuid(const SessionData_s *, unsigned long long) { return -1; }

} // extern "C++"

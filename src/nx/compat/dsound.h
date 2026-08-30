// dsound.h -- DirectSound subset. Real playback/capture backends
// (play_dsound.cpp / record_dsound.cpp) are excluded from the Switch build;
// this exists so directsound.h / r_cinematic.cpp compile. DirectSoundCreate8
// fails, so all runtime paths bail out cleanly.
#ifndef NX_COMPAT_DSOUND_H
#define NX_COMPAT_DSOUND_H

#include "windows.h"

#ifndef NX_WAVEFORMATEX_DEFINED
#define NX_WAVEFORMATEX_DEFINED
#pragma pack(push, 1)
typedef struct tWAVEFORMATEX {
    WORD  wFormatTag;
    WORD  nChannels;
    DWORD nSamplesPerSec;
    DWORD nAvgBytesPerSec;
    WORD  nBlockAlign;
    WORD  wBitsPerSample;
    WORD  cbSize;
} WAVEFORMATEX, *PWAVEFORMATEX, *LPWAVEFORMATEX;
#pragma pack(pop)
#define WAVE_FORMAT_PCM 1
#endif

#define DS_OK 0
#define DSERR_GENERIC ((HRESULT)0x80004005)
#define DSSCL_NORMAL   1
#define DSSCL_PRIORITY 2
#define DSBPLAY_LOOPING 1
#define DSBSTATUS_PLAYING 1
#define DSBCAPS_CTRLFREQUENCY 0x20
#define DSBCAPS_GLOBALFOCUS 0x8000
#define DSBCAPS_GETCURRENTPOSITION2 0x10000
#define DSCBSTART_LOOPING 1

typedef struct _DSBUFFERDESC {
    DWORD dwSize;
    DWORD dwFlags;
    DWORD dwBufferBytes;
    DWORD dwReserved;
    LPWAVEFORMATEX lpwfxFormat;
    GUID  guid3DAlgorithm;
} DSBUFFERDESC, *LPDSBUFFERDESC;
typedef const DSBUFFERDESC *LPCDSBUFFERDESC;

typedef struct _DSCBUFFERDESC {
    DWORD dwSize;
    DWORD dwFlags;
    DWORD dwBufferBytes;
    DWORD dwReserved;
    LPWAVEFORMATEX lpwfxFormat;
    DWORD dwFXCount;
    void *lpDSCFXDesc;
} DSCBUFFERDESC, *LPDSCBUFFERDESC;
typedef const DSCBUFFERDESC *LPCDSCBUFFERDESC;

class IDirectSoundBuffer {
public:
    HRESULT Lock(DWORD offset, DWORD bytes, LPVOID *audioPtr1, LPDWORD audioBytes1,
                 LPVOID *audioPtr2, LPDWORD audioBytes2, DWORD flags);
    HRESULT Unlock(LPVOID audioPtr1, DWORD audioBytes1, LPVOID audioPtr2, DWORD audioBytes2);
    HRESULT Play(DWORD reserved1, DWORD priority, DWORD flags);
    HRESULT Stop();
    HRESULT GetCurrentPosition(LPDWORD playCursor, LPDWORD writeCursor);
    HRESULT SetFrequency(DWORD frequency);
    HRESULT GetStatus(LPDWORD status);
    HRESULT SetVolume(LONG volume);
    ULONG   AddRef();
    ULONG   Release();
};
typedef IDirectSoundBuffer *LPDIRECTSOUNDBUFFER;

class IDirectSound8 {
public:
    HRESULT SetCooperativeLevel(HWND hwnd, DWORD level);
    HRESULT CreateSoundBuffer(LPCDSBUFFERDESC desc, LPDIRECTSOUNDBUFFER *buffer, void *unkOuter);
    ULONG   AddRef();
    ULONG   Release();
};
typedef IDirectSound8 *LPDIRECTSOUND8;
typedef IDirectSound8 IDirectSound;
typedef IDirectSound8 *LPDIRECTSOUND;

class IDirectSoundCaptureBuffer {
public:
    HRESULT Start(DWORD flags);
    HRESULT Stop();
    HRESULT Lock(DWORD offset, DWORD bytes, LPVOID *audioPtr1, LPDWORD audioBytes1,
                 LPVOID *audioPtr2, LPDWORD audioBytes2, DWORD flags);
    HRESULT Unlock(LPVOID audioPtr1, DWORD audioBytes1, LPVOID audioPtr2, DWORD audioBytes2);
    HRESULT GetCurrentPosition(LPDWORD capturePos, LPDWORD readPos);
    ULONG   AddRef();
    ULONG   Release();
};
typedef IDirectSoundCaptureBuffer *LPDIRECTSOUNDCAPTUREBUFFER;

class IDirectSoundCapture {
public:
    HRESULT CreateCaptureBuffer(LPCDSCBUFFERDESC desc, LPDIRECTSOUNDCAPTUREBUFFER *buffer, void *unkOuter);
    ULONG   AddRef();
    ULONG   Release();
};
typedef IDirectSoundCapture *LPDIRECTSOUNDCAPTURE;
typedef IDirectSoundCapture IDirectSoundCapture8;
typedef IDirectSoundCapture *LPDIRECTSOUNDCAPTURE8;

HRESULT DirectSoundCreate8(const GUID *device, LPDIRECTSOUND8 *ds, void *unkOuter);
#define DirectSoundCreate DirectSoundCreate8
HRESULT DirectSoundCaptureCreate(const GUID *device, LPDIRECTSOUNDCAPTURE *dsc, void *unkOuter);
#define DirectSoundCaptureCreate8 DirectSoundCaptureCreate

#endif // NX_COMPAT_DSOUND_H

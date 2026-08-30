// ddraw.h -- DirectDraw 7 subset (r_texturemem.cpp video-memory probe only;
// loaded via LoadLibrary/GetProcAddress which return NULL on Switch, so the
// code paths are dead at runtime -- this only needs to compile).
#ifndef NX_COMPAT_DDRAW_H
#define NX_COMPAT_DDRAW_H

#include "windows.h"

#define DD_OK 0
#define DDSCL_NORMAL 0x8u
#define DDSCAPS_VIDEOMEMORY   0x4000u
#define DDSCAPS_LOCALVIDMEM   0x10000000u
#define DDSCAPS_NONLOCALVIDMEM 0x20000000u
#define DDSCAPS_TEXTURE       0x1000u

typedef struct _DDSCAPS2 {
    DWORD dwCaps;
    DWORD dwCaps2;
    DWORD dwCaps3;
    DWORD dwCaps4;
} DDSCAPS2, *LPDDSCAPS2;

class IDirectDraw7 {
public:
    HRESULT SetCooperativeLevel(HWND hwnd, DWORD flags);
    HRESULT GetAvailableVidMem(LPDDSCAPS2 caps, LPDWORD total, LPDWORD free_);
    ULONG   AddRef();
    ULONG   Release();
};
typedef IDirectDraw7 *LPDIRECTDRAW7;

extern const GUID IID_IDirectDraw7;

typedef HRESULT (WINAPI *LPDIRECTDRAWCREATEEX)(GUID *guid, LPVOID *dd, REFIID iid, void *unkOuter);
typedef BOOL (WINAPI *LPDDENUMCALLBACKEXA)(GUID *, LPSTR, LPSTR, LPVOID, HMONITOR);
typedef HRESULT (WINAPI *LPDIRECTDRAWENUMERATEEXA)(LPDDENUMCALLBACKEXA cb, LPVOID ctx, DWORD flags);
#define DDENUM_ATTACHEDSECONDARYDEVICES 1

HRESULT DirectDrawCreateEx(GUID *guid, LPVOID *dd, REFIID iid, void *unkOuter);

#endif // NX_COMPAT_DDRAW_H

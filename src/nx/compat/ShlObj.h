// ShlObj.h -- SHGetFolderPathA stub; maps user-documents folders to sdmc.
#ifndef NX_COMPAT_SHLOBJ_H
#define NX_COMPAT_SHLOBJ_H

#include "windows.h"

#define CSIDL_PERSONAL      0x0005
#define CSIDL_APPDATA       0x001A
#define CSIDL_LOCAL_APPDATA 0x001C
#define CSIDL_FLAG_CREATE   0x8000
#define SHGFP_TYPE_CURRENT  0

HRESULT SHGetFolderPathA(HWND hwnd, int csidl, HANDLE token, DWORD flags, LPSTR path);

#endif // NX_COMPAT_SHLOBJ_H

// TlHelp32.h -- toolhelp snapshot stubs (used by Sys_IsGameProcess only;
// always fails on Switch, which makes the caller take the "not found" path).
#ifndef NX_COMPAT_TLHELP32_H
#define NX_COMPAT_TLHELP32_H

#include "windows.h"

#define TH32CS_SNAPPROCESS 0x2
#define TH32CS_SNAPMODULE  0x8

typedef struct tagPROCESSENTRY32 {
    DWORD     dwSize;
    DWORD     cntUsage;
    DWORD     th32ProcessID;
    ULONG_PTR th32DefaultHeapID;
    DWORD     th32ModuleID;
    DWORD     cntThreads;
    DWORD     th32ParentProcessID;
    LONG      pcPriClassBase;
    DWORD     dwFlags;
    CHAR      szExeFile[MAX_PATH];
} PROCESSENTRY32, *LPPROCESSENTRY32;

typedef struct tagMODULEENTRY32 {
    DWORD     dwSize;
    DWORD     th32ModuleID;
    DWORD     th32ProcessID;
    DWORD     GlblcntUsage;
    DWORD     ProccntUsage;
    BYTE     *modBaseAddr;
    DWORD     modBaseSize;
    HMODULE   hModule;
    char      szModule[256];
    char      szExePath[MAX_PATH];
} MODULEENTRY32, *LPMODULEENTRY32;

HANDLE CreateToolhelp32Snapshot(DWORD flags, DWORD pid);
BOOL Process32First(HANDLE snap, LPPROCESSENTRY32 entry);
BOOL Process32Next(HANDLE snap, LPPROCESSENTRY32 entry);
BOOL Module32First(HANDLE snap, LPMODULEENTRY32 entry);
BOOL Module32Next(HANDLE snap, LPMODULEENTRY32 entry);

#endif // NX_COMPAT_TLHELP32_H

#include "tl_support.h"
#include <tl/tl_system.h>
#include <universal/mem_userhunk.h>
#include "common.h"
#include <win32/win_common.h>
#include <stdio.h>

HunkUser *s_tlHunkUser;

void __cdecl Sys_SetupTLCallbacks(int hunkMemSize)
{
    tlSystemCallbacks callbacks; // [esp+0h] [ebp-20h] BYREF

    s_tlHunkUser = Hunk_UserCreate(hunkMemSize, HU_SCHEME_DEFAULT, 4u, 0, "TL_MemAlloc support", 37);
    printf("TL: hunk=%p scheme=%d flags=%u sizeof(HunkUserDefault)=%u\n",
           (void *)s_tlHunkUser,
           s_tlHunkUser ? (int)s_tlHunkUser->scheme : -1,
           s_tlHunkUser ? s_tlHunkUser->flags : 0,
           (unsigned)sizeof(HunkUserDefault));
    callbacks.ReadFile = (bool (__cdecl *)(const char *, tlFileBuf *, unsigned int, unsigned int))TL_ReadFile;
    callbacks.ReleaseFile = (void (__cdecl *)(tlFileBuf *))TL_ReleaseFile;
    callbacks.CriticalError = TL_CriticalError;
    callbacks.Warning = TL_Warning;
    callbacks.DebugPrint = (void (__cdecl *)(const char *))TL_DebugPrint;
    callbacks.MemAlloc = (void *(__cdecl *)(unsigned int, unsigned int, unsigned int))TL_MemAlloc;
    callbacks.MemRealloc = (void *(__cdecl *)(void *, unsigned int, unsigned int, unsigned int))RETURN_ZERO32;
    callbacks.MemFree = TL_MemFree;
    tlSetSystemCallbacks(&callbacks);
}

void TL_DebugDumpHunk(const char *where)
{
    printf("TL check [%s]: user=%p scheme=%d\n", where,
           (void *)s_tlHunkUser,
           s_tlHunkUser ? (int)s_tlHunkUser->scheme : -1);
}

void __cdecl TL_Warning(const char *Text)
{
    Com_PrintWarning(14, "TL Warning: %s\n", Text);
}

bool __cdecl TL_ReadFile()
{
    TL_Warning("TL_ReadFile: no implementation");
    return 0;
}

void __cdecl TL_ReleaseFile()
{
    TL_Warning("TL_ReleaseFile: no implementation");
}

void __cdecl TL_DebugPrint(char *Text)
{
    Com_Printf(14, Text);
}

void *__cdecl TL_MemAlloc(unsigned int Size, unsigned int Align)
{
    void *v3; // [esp+8h] [ebp-Ch]

    Sys_EnterCriticalSection(CRITSECT_TL_MEMALLOC);
    printf("TL_MemAlloc: user=%p scheme=%d size=%u align=%u\n",
           (void *)s_tlHunkUser,
           s_tlHunkUser ? (int)s_tlHunkUser->scheme : -1, Size, Align);
    v3 = Hunk_UserAlloc(s_tlHunkUser, Size, Align, 0);
    printf("TL_MemAlloc: returned %p\n", v3);
    Sys_LeaveCriticalSection(CRITSECT_TL_MEMALLOC);
    return v3;
}

void __cdecl TL_MemFree(void *Ptr)
{
    Hunk_UserFree(s_tlHunkUser, Ptr);
}

void __cdecl TL_CriticalError(const char *msg)
{
    if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\tl_support.cpp", 95, 0, msg) )
        __debugbreak();
}


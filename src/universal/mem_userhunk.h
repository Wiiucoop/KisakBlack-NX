#pragma once

#include <stdint.h>
#include <stddef.h>

#define HF_FIXEDSIZE 4

enum HU_ALLOCATION_SCHEME : __int32
{                                                                             // XREF: HunkUser/r
    HU_SCHEME_DEFAULT = 0x0,
    HU_SCHEME_DEBUG = 0x1,
    HU_SCHEME_FIRSTFIT = 0x2,
    HU_SCHEME_FIXED = 0x3,
    HU_SCHEME_COUNT = 0x4,
};

struct HunkUser // sizeof=0x10
{                                                                             // XREF: HunkUserDefault/r
    HU_ALLOCATION_SCHEME scheme;
    unsigned int flags;
    const char *name;
    int type;
};

// buffer[0..3] were the HunkUser, buffer[4] the pointer to the FirstFit hunk
// that follows inline. Named so the offsets stay right when pointers are 8
// bytes.
struct DebugHunkUser
{
    HunkUser  hunkUser;
    HunkUser *firstFit;
};

struct __declspec(align(4)) HunkUserDefault // sizeof=0x2C
{
    HunkUser hunkUser;
    HunkUserDefault *current;
    HunkUserDefault *next;
    int maxSize;
    intptr_t end;
    intptr_t pos;
    intptr_t locked;
    unsigned __int8 buf[1];
    // padding byte
    // padding byte
    // padding byte
};

HunkUser *__cdecl Hunk_UserDebugInit(
                unsigned int *buffer,
                int size,
                HU_ALLOCATION_SCHEME scheme,
                unsigned int flags,
                void *scheme_specific_data,
                const char *name,
                int type);
void __cdecl Hunk_UserDebugReset(HunkUser *_user);
void __cdecl Hunk_UserDebugDestroy(HunkUser *_user);
void *__cdecl Hunk_UserDebugAlloc(HunkUser *_user, int size, int alignment);
void __cdecl Hunk_UserDebugFree(HunkUser *_user, void *ptr);
HunkUser *__cdecl Hunk_UserDefaultInit(
                unsigned __int8 *buffer,
                int size,
                HU_ALLOCATION_SCHEME scheme,
                unsigned int flags,
                void *scheme_specific_data,
                const char *name,
                int type);
void __cdecl Hunk_UserDefaultReset(HunkUserDefault *_user);
void __cdecl Hunk_UserDefaultDestroy(HunkUserDefault *_user);
void *__cdecl Hunk_UserDefaultAlloc(HunkUserDefault *_user, unsigned int size, int alignment, const char *name);
void __cdecl Hunk_UserDefaultFree(HunkUser *user, void *ptr);
void __cdecl Hunk_UserStartup();
void __cdecl Hunk_UserShutdown();
HunkUser *__cdecl Hunk_UserCreateFromBuffer(
                void *buffer,
                int size,
                HU_ALLOCATION_SCHEME scheme,
                unsigned int flags,
                void *scheme_specific_data,
                const char *name,
                int type);
HunkUser *__cdecl Hunk_UserCreate(
                int size,
                HU_ALLOCATION_SCHEME scheme,
                unsigned int flags,
                void *scheme_specific_data,
                const char *name,
                int type);
void *__cdecl Hunk_UserAlloc(HunkUser *user, int size, int alignment, const char *name);
void __cdecl Hunk_UserFree(HunkUser *user, void *ptr);
void __cdecl Hunk_UserReset(HunkUser *user);
void __cdecl Hunk_UserDestroy(HunkUser *user);
void __cdecl Hunk_UserSetPos(HunkUser *_user, unsigned char *pos);
char *__cdecl Hunk_CopyString(HunkUser *user, const char *in);


extern HunkUser *g_DebugHunkUser;
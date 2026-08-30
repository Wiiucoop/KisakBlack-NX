#include "mem_firstfit.h"
#include "assertive.h"
#include <win32/win_common.h>
#include <string.h>

// Bytes reserved before each user pointer. The original used 12 and stashed
// the owning node at userptr-4; on LP64 that back-pointer needs 8, so the
// header is 16 to keep alignment sane.
static const size_t kBlockHeader = 16;

// Written into node->next while a block is handed out, so a double free or a
// bad pointer is caught instead of corrupting the free list.
static _firstfit_heapnode *const kInUse = (_firstfit_heapnode *)(uintptr_t)0xDEADBEEF;

static inline uintptr_t alignUp(uintptr_t v, size_t a)
{
    return (v + a - 1) & ~(uintptr_t)(a - 1);
}

HunkUser *__cdecl Hunk_FirstFitInit(
                unsigned int *buffer,
                unsigned int size,
                HU_ALLOCATION_SCHEME scheme,
                unsigned int flags,
                void *scheme_specific_data,
                const char *name,
                int type)
{
    (void)scheme_specific_data;

    if ( (flags & 2) == 0
        && !Assert_MyHandler(
                    "src/universal/mem_firstfit.cpp", 49, 0, "%s",
                    "(flags & HF_FROMBUFFER)!=0") )
    {
        __debugbreak();
    }
    if ( size <= sizeof(FirstFitHunkUser) + sizeof(_firstfit_heapnode)
        && !Assert_MyHandler(
                    "src/universal/mem_firstfit.cpp", 50, 0, "%s",
                    "size>sizeof(FIRSTFIT_HUNKUSER)+sizeof(FIRSTFIT_HEAPNODE)") )
    {
        __debugbreak();
    }

    FirstFitHunkUser *hunk = (FirstFitHunkUser *)buffer;
    hunk->hunkUser.scheme = scheme;
    hunk->hunkUser.flags  = flags;
    hunk->hunkUser.name   = name;
    hunk->hunkUser.type   = type;
    hunk->size   = size;
    hunk->marker = -1;
    hunk->used   = sizeof(FirstFitHunkUser);

    hunk->freeBlocks = (_firstfit_heapnode *)(hunk + 1);
    hunk->freeBlocks->next = 0;
    hunk->freeBlocks->size = (int)(size - sizeof(FirstFitHunkUser));

    return (HunkUser *)hunk;
}

void __cdecl Hunk_FirstFitReset(HunkUser *_user)
{
    FirstFitHunkUser *hunk = (FirstFitHunkUser *)_user;
    hunk->freeBlocks = (_firstfit_heapnode *)(hunk + 1);
    hunk->freeBlocks->next = 0;
    hunk->freeBlocks->size = (int)(hunk->size - sizeof(FirstFitHunkUser));
    hunk->used = sizeof(FirstFitHunkUser);
}

void __cdecl Hunk_FirstFitDestroy(HunkUser *_user)
{
    memset(_user, 0, sizeof(FirstFitHunkUser));
}

void *__cdecl Hunk_FirstFitAlloc(HunkUser *_user, int size, int alignment)
{
    FirstFitHunkUser *hunk = (FirstFitHunkUser *)_user;

    if ( alignment <= 0
        && !Assert_MyHandler("src/universal/mem_firstfit.cpp", 90, 0, "%s",
                             "alignment>0") )
    {
        __debugbreak();
    }

    Sys_EnterCriticalSection(CRITSECT_MEMFIRSTFIT);

    _firstfit_heapnode *node = hunk->freeBlocks;
    _firstfit_heapnode *last = 0;
    uintptr_t userAddr = 0;
    int adjSize = 0;

    while ( node )
    {
        userAddr = alignUp((uintptr_t)node + kBlockHeader, (size_t)alignment);
        adjSize  = (int)(userAddr - (uintptr_t)node) + size;
        if ( node->size >= adjSize )
            break;
        last = node;
        node = node->next;
    }

    // Out of room, or the only candidate is the tail block and splitting it
    // would leave less than 1 KB -- the original refuses that case too.
    if ( !node || (!node->next && (node->size - adjSize) <= 1024) )
    {
        Sys_LeaveCriticalSection(CRITSECT_MEMFIRSTFIT);
        return 0;
    }

    if ( (node->size - adjSize) > 1024 )
    {
        _firstfit_heapnode *rest = (_firstfit_heapnode *)(userAddr + size);
        rest->next = node->next;
        rest->size = node->size - adjSize;
        node->next = rest;
        node->size = adjSize;
    }

    if ( last )
        last->next = node->next;
    else
        hunk->freeBlocks = node->next;

    node->next = kInUse;
    ((_firstfit_heapnode **)userAddr)[-1] = node;   // back-pointer for free()
    hunk->used += node->size;

    Sys_LeaveCriticalSection(CRITSECT_MEMFIRSTFIT);
    return (void *)userAddr;
}

void __cdecl Hunk_FirstFitFree(HunkUser *_user, void *ptr)
{
    if ( !ptr )
        return;

    FirstFitHunkUser *hunk = (FirstFitHunkUser *)_user;
    Sys_EnterCriticalSection(CRITSECT_MEMFIRSTFIT);

    _firstfit_heapnode *freed = ((_firstfit_heapnode **)ptr)[-1];

    if ( freed->next != kInUse )
    {
        if ( !Assert_MyHandler("src/universal/mem_firstfit.cpp", 173, 0,
                               "%s\n\t%s", "0",
                               "buffer overrun or underrun or illegal ptr detected") )
        {
            __debugbreak();
        }
        Sys_LeaveCriticalSection(CRITSECT_MEMFIRSTFIT);
        return;
    }

    hunk->used -= freed->size;

    if ( !hunk->freeBlocks
        && !Assert_MyHandler("src/universal/mem_firstfit.cpp", 178, 0, "%s",
                             "hunk->free_blocks!=NULL") )
    {
        __debugbreak();
    }

    _firstfit_heapnode *last = 0;
    _firstfit_heapnode *scan = hunk->freeBlocks;

    while ( scan )
    {
        if ( scan > freed )
        {
            if ( last )
            {
                freed->next = scan;
                last->next  = freed;
            }
            else
            {
                freed->next      = hunk->freeBlocks;
                hunk->freeBlocks = freed;
            }

            // coalesce with the previous block, then with the next one
            if ( last && (_firstfit_heapnode *)((char *)last + last->size) == freed )
            {
                last->size += freed->size;
                last->next  = freed->next;
                freed = last;
            }
            if ( freed->next &&
                 (_firstfit_heapnode *)((char *)freed + freed->size) == freed->next )
            {
                freed->size += freed->next->size;
                freed->next  = freed->next->next;
            }

            Sys_LeaveCriticalSection(CRITSECT_MEMFIRSTFIT);
            return;
        }
        last = scan;
        scan = scan->next;
    }

    if ( !Assert_MyHandler("src/universal/mem_firstfit.cpp", 220, 0,
                           "%s\n\t%s", "0",
                           "invalid pointer passed into first fit free\n") )
    {
        __debugbreak();
    }

    Sys_LeaveCriticalSection(CRITSECT_MEMFIRSTFIT);
}


#pragma once
#include "mem_userhunk.h"
#include <stdint.h>

struct _firstfit_heapnode
{
    _firstfit_heapnode *next;
    int size;
};

// The original laid this out as two consecutive HunkUser slots, reusing the
// second one's fields as size / free-list head / marker / used-bytes. That
// worked while pointers were 4 bytes. Named fields instead, so the struct
// grows correctly on LP64.
struct FirstFitHunkUser
{
    HunkUser            hunkUser;
    size_t              size;
    _firstfit_heapnode *freeBlocks;
    intptr_t            marker;
    size_t              used;
};

HunkUser *__cdecl Hunk_FirstFitInit(
                unsigned int *buffer,
                unsigned int size,
                HU_ALLOCATION_SCHEME scheme,
                unsigned int flags,
                void *scheme_specific_data,
                const char *name,
                int type);
void __cdecl Hunk_FirstFitReset(HunkUser *_user);
void __cdecl Hunk_FirstFitDestroy(HunkUser *_user);
void *__cdecl Hunk_FirstFitAlloc(HunkUser *_user, int size, int alignment);
void __cdecl Hunk_FirstFitFree(HunkUser *_user, void *ptr);

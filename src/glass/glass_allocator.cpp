#include "glass_allocator.h"
#include "glass_client.h"
#include <universal/assertive.h>

void __thiscall SmallAllocator::Init(void *buffer, unsigned int bs, unsigned int nb)
{
    unsigned int i; // [esp+4h] [ebp-8h]
    char *ptr; // [esp+8h] [ebp-4h]

    this->memory = buffer;
    this->blockSize = bs;
    this->numBlocks = nb;
    this->numUsed = 0;
    this->maxUsed = 0;
    this->freeHead = this->memory;
    ptr = (char *)this->memory;
    for ( i = 0; i < this->numBlocks - 1; ++i )
    {
        *(void **)ptr = &ptr[this->blockSize];   // nx-port: was a 32-bit store
        ptr += this->blockSize;
    }
    *(void **)ptr = 0;
}

void **__thiscall SmallAllocator::Allocate(unsigned int size)
{
    unsigned int numUsed; // [esp+0h] [ebp-14h]
    void **ret; // [esp+10h] [ebp-4h]

    if ( size > this->blockSize
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\glass\\glass_allocator.cpp",
                    59,
                    0,
                    "%s",
                    "size <= blockSize") )
    {
        __debugbreak();
    }
    if ( !this->freeHead
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\glass\\glass_allocator.cpp", 60, 0, "%s", "freeHead") )
    {
        __debugbreak();
    }
    if ( (signed int)this->maxUsed < (signed int)++this->numUsed )
        numUsed = this->numUsed;
    else
        numUsed = this->maxUsed;
    this->maxUsed = numUsed;
    ret = (void **)this->freeHead;
    this->freeHead = *ret;
    return ret;
}

void __thiscall SmallAllocator::Free(void **ptr, unsigned int num)
{
    if ( num != 1
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\glass\\glass_allocator.cpp", 73, 0, "%s", "num == 1") )
    {
        __debugbreak();
    }
    if ( !SmallAllocator::IsValidPointer(ptr)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\glass\\glass_allocator.cpp",
                    74,
                    0,
                    "%s",
                    "IsValidPointer( ptr )") )
    {
        __debugbreak();
    }
    if ( !this->numUsed
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\glass\\glass_allocator.cpp", 75, 0, "%s", "numUsed > 0") )
    {
        __debugbreak();
    }
    --this->numUsed;
    *ptr = this->freeHead;
    this->freeHead = ptr;
}

bool __thiscall SmallAllocator::IsValidPointer(void *ptr)
{
    return ptr >= this->memory
            && !(((uintptr_t)ptr - (uintptr_t)this->memory) % this->blockSize)
            && (uintptr_t)ptr - (uintptr_t)this->memory < this->numBlocks * this->blockSize;
}

void __thiscall Allocator::Memory::Init()
{
    this->next = 0;
    this->prev = 0;
    this->userData = 0;
    this->nextFree = 0;
    this->prevFree = 0;
}

char __thiscall Allocator::Memory::MakeFree()
{
    Allocator::Memory *p; // [esp+4h] [ebp-8h]
    Allocator::Memory *n; // [esp+8h] [ebp-4h]

    for ( n = this->next; n && !n->nextFree; n = n->next )
        ;
    if ( n )
    {
        this->nextFree = n;
        this->prevFree = n->prevFree;
        n->prevFree = this;
        if ( this->prevFree )
            this->prevFree->nextFree = this;
    }
    else
    {
        for ( p = this->prev; p && !p->nextFree; p = p->prev )
            ;
        if ( !p )
            return 0;
        this->prevFree = p;
        this->nextFree = p->nextFree;
        p->nextFree = this;
        if ( this->nextFree )
            this->nextFree->prevFree = this;
    }
    return 1;
}

void __thiscall Allocator::Init(void *buf, int size)
{
    int v4; // [esp+4h] [ebp-Ch]
    Allocator::Memory *end; // [esp+Ch] [ebp-4h]

    this->buffer = buf;
    end = (Allocator::Memory *)(((uintptr_t)this->buffer + size - sizeof(Allocator::Memory) - 1) & ~(uintptr_t)15);
    this->head = (Allocator::Memory *)(((uintptr_t)this->buffer + 15) & ~(uintptr_t)15);
    //Allocator::Memory::Init(this->head);
    this->head->Init();
    this->tail = end;
    this->tail->Init();
    //Allocator::Memory::Init(this->tail);
    Allocator::FreeAll();
    v4 = (char *)this->tail - (char *)this->head;
    this->maxUsed = v4 - Allocator::GetFree();
}

void __thiscall Allocator::FreeAll()
{
    this->head->nextFree = this->tail;
    this->head->next = this->head->nextFree;
    this->head->prevFree = 0;
    this->head->prev = 0;
    this->tail->prevFree = this->head;
    this->tail->prev = this->tail->prevFree;
    this->tail->nextFree = 0;
    this->tail->next = 0;
    this->freeHead = this->head;
}

Allocator::Memory **__thiscall Allocator::Allocate(int size, void *userData)
{
    int maxUsed; // [esp+0h] [ebp-30h]
    signed int v5; // [esp+4h] [ebp-2Ch]
    int v6; // [esp+8h] [ebp-28h]
    int v7; // [esp+Ch] [ebp-24h]
    signed int v8; // [esp+10h] [ebp-20h]
    int v10; // [esp+1Ch] [ebp-14h]
    int v11; // [esp+20h] [ebp-10h]
    Allocator::Memory *free; // [esp+28h] [ebp-8h]
    Allocator::Memory *bestFit; // [esp+2Ch] [ebp-4h]
    signed int sizea; // [esp+38h] [ebp+8h]

    if ( this->freeHead == this->tail )
        return 0;
    bestFit = 0;
    // nx-port: was (size + 31) & ~15: the 16-byte x86 header before the data
    sizea = (int)((size + GLASS_MEM_DATA_OFFSET + 15) & ~(uintptr_t)15);
    for ( free = this->freeHead; free != this->tail; free = free->nextFree )
    {
        if ( free->next )
            v8 = (char *)free->next - (char *)free;
        else
            v8 = 0;
        if ( v8 >= sizea )
        {
            if ( !bestFit
                || (!free->next ? (v7 = 0) : (v7 = (char *)free->next - (char *)free),
                        !bestFit->next ? (v6 = 0) : (v6 = (char *)bestFit->next - (char *)bestFit),
                        v7 < v6) )
            {
                bestFit = free;
            }
        }
    }
    if ( !bestFit )
        return 0;
    if ( bestFit->next )
        v5 = (char *)bestFit->next - (char *)bestFit;
    else
        v5 = 0;
    if ( v5 > sizea )
        Allocator::Split(bestFit, sizea);
    bestFit->userData = userData;
    if ( bestFit->prevFree )
    {
        bestFit->prevFree->nextFree = bestFit->nextFree;
    }
    else
    {
        if ( this->freeHead != bestFit
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\glass\\glass_allocator.cpp",
                        206,
                        0,
                        "%s",
                        "freeHead == bestFit") )
        {
            __debugbreak();
        }
        this->freeHead = bestFit->nextFree;
    }
    if ( bestFit->nextFree )
        bestFit->nextFree->prevFree = bestFit->prevFree;
    bestFit->nextFree = 0;
    bestFit->prevFree = 0;
    v11 = (char *)this->tail - (char *)this->head;
    v10 = v11 - Allocator::GetFree();
    if ( this->maxUsed < v10 )
        maxUsed = v10;
    else
        maxUsed = this->maxUsed;
    this->maxUsed = maxUsed;
    return &bestFit->prevFree;
}

void __thiscall Allocator::Free(unsigned int *ptr)
{
    Allocator::Memory *mem; // [esp+4h] [ebp-4h]

    if ( ptr )
    {
        mem = (Allocator::Memory *)((char *)ptr - GLASS_MEM_DATA_OFFSET);   // nx-port: was ptr - 4
        if ( mem->nextFree
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\glass\\glass_allocator.cpp",
                        233,
                        0,
                        "%s",
                        "!mem->IsFree()") )
        {
            __debugbreak();
        }
        if ( this->freeHead == this->tail )
        {
            mem->prevFree = 0;
            mem->nextFree = this->tail;
            this->freeHead = mem;
        }
        else
        {
            //if ( !Allocator::Memory::MakeFree(mem)
            if ( !mem->MakeFree()
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\glass\\glass_allocator.cpp",
                            250,
                            0,
                            "couldn't find a free Memory block!?!?!?") )
            {
                __debugbreak();
            }
            if ( !mem->prevFree )
                this->freeHead = mem;
        }
        while ( mem->prev && mem->prev == mem->prevFree )
            mem = mem->prev;
        while ( mem->nextFree && mem->nextFree == mem->next && mem->next != this->tail )
            Allocator::Merge(mem, mem->next);
    }
}

unsigned int __thiscall Allocator::GetMemorySize(unsigned int *ptr)
{
    if ( !ptr )
        return -1;
    Allocator::Memory *mem = (Allocator::Memory *)((char *)ptr - GLASS_MEM_DATA_OFFSET);   // nx-port: was ptr - 4
    if ( mem->next )
        return (unsigned int)((char *)mem->next - (char *)mem);
    else
        return 0;
}

void __thiscall Allocator::Split(Allocator::Memory *mem, int size)
{
    int v3; // [esp+0h] [ebp-10h]
    int v4; // [esp+4h] [ebp-Ch]
    Allocator::Memory *mem2; // [esp+Ch] [ebp-4h]

    if ( !mem->nextFree
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\glass\\glass_allocator.cpp", 277, 0, "%s", "mem->IsFree()") )
    {
        __debugbreak();
    }
    if ( mem->next )
        v4 = (char *)mem->next - (char *)mem;
    else
        v4 = 0;
    if ( v4 <= size
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\glass\\glass_allocator.cpp",
                    278,
                    0,
                    "%s",
                    "mem->GetSize() > size") )
    {
        __debugbreak();
    }
    if ( mem->next )
        v3 = (char *)mem->next - (char *)mem;
    else
        v3 = 0;
    if ( v3 - size > (int)((sizeof(Allocator::Memory) + 15) & ~(size_t)15) )   // nx-port: was 32
    {
        mem2 = (Allocator::Memory *)((char *)mem + size);
        *(void **)((char *)&mem->userData + size) = 0;
        mem2->next = mem->next;
        mem2->prev = mem;
        mem2->nextFree = mem->nextFree;
        mem2->prevFree = mem;
        if ( *(Allocator::Memory **)((char *)&mem->next + size) )
            mem2->next->prev = mem2;
        if ( mem2->nextFree )
            mem2->nextFree->prevFree = mem2;
        mem->next = mem2;
        mem->nextFree = mem2;
    }
}

void __thiscall Allocator::Merge(Allocator::Memory *mem1, Allocator::Memory *mem2)
{
    if ( mem2 != this->tail )
    {
        if ( !mem1->nextFree
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\glass\\glass_allocator.cpp",
                        305,
                        0,
                        "%s",
                        "mem1->IsFree()") )
        {
            __debugbreak();
        }
        if ( !mem2->nextFree
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\glass\\glass_allocator.cpp",
                        306,
                        0,
                        "%s",
                        "mem2->IsFree()") )
        {
            __debugbreak();
        }
        if ( &mem1->prevFree >= &mem2->prevFree
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\glass\\glass_allocator.cpp",
                        307,
                        0,
                        "%s",
                        "mem1->GetPtr() < mem2->GetPtr()") )
        {
            __debugbreak();
        }
        if ( mem1->next != mem2
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\glass\\glass_allocator.cpp",
                        308,
                        0,
                        "%s",
                        "mem1->next == mem2") )
        {
            __debugbreak();
        }
        mem1->next = mem2->next;
        mem1->nextFree = mem2->nextFree;
        if ( mem1->next )
            mem1->next->prev = mem1;
        if ( mem1->nextFree )
            mem1->nextFree->prevFree = mem1;
    }
}

int __thiscall Allocator::Defrag(void (__cdecl *func)(void *), int count)
{
    while ( count > 0 && this->freeHead != this->tail && this->freeHead->next != this->tail )
    {
        if ( this->freeHead->next->nextFree
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\glass\\glass_allocator.cpp",
                        401,
                        0,
                        "%s",
                        "!freeHead->next->IsFree()") )
        {
            __debugbreak();
        }
        if ( this->freeHead->next->userData )
            func(this->freeHead->next->userData);
        --count;
    }
    return count;
}

int __thiscall Allocator::GetFree()
{
    int v2; // [esp+0h] [ebp-10h]
    Allocator::Memory *mem; // [esp+8h] [ebp-8h]
    int free; // [esp+Ch] [ebp-4h]

    free = 0;
    for ( mem = this->freeHead; mem; mem = mem->nextFree )
    {
        if ( mem->next )
            v2 = (char *)mem->next - (char *)mem;
        else
            v2 = 0;
        free += v2;
    }
    return free;
}

int __thiscall Allocator::GetLargestFree()
{
    int v2; // [esp+0h] [ebp-14h]
    int v3; // [esp+8h] [ebp-Ch]
    Allocator::Memory *mem; // [esp+Ch] [ebp-8h]
    int largest; // [esp+10h] [ebp-4h]

    largest = 0;
    for ( mem = this->freeHead; mem; mem = mem->nextFree )
    {
        if ( mem->next )
            v3 = (char *)mem->next - (char *)mem;
        else
            v3 = 0;
        if ( largest < v3 )
            v2 = v3;
        else
            v2 = largest;
        largest = v2;
    }
    return largest;
}


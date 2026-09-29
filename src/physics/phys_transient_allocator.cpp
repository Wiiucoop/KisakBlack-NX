#include "phys_transient_allocator.h"
#include "phys_mem_new.h"

void phys_transient_allocator::reset()
{
    phys_transient_allocator::block_header *m_first_block; // eax
    phys_slot_pool *m_slot_pool; // ebx
    phys_transient_allocator::block_header *m_next_block; // esi

    m_first_block = this->m_first_block;
    m_slot_pool = (phys_slot_pool *)this->m_slot_pool;
    if ( this->m_first_block )
    {
        do
        {
            m_next_block = m_first_block->m_next_block;
            PSP_FREE(m_slot_pool, (unsigned __int8 *)m_first_block);
            m_first_block = m_next_block;
        }
        while ( m_next_block );
    }
    this->m_first_block = 0;
    this->m_cur = 0;
    this->m_end = 0;
    this->m_total_memory_allocated = 0;
}
void __thiscall phys_transient_allocator::reset_to_state(const phys_transient_allocator::allocator_state *as)
{
    phys_transient_allocator::block_header *m_first_block; // eax
    phys_transient_allocator::block_header *m_next_block; // esi
    phys_slot_pool *slot_pool; // [esp+8h] [ebp-4h]

    slot_pool = (phys_slot_pool *)this->m_slot_pool;
    m_first_block = this->m_first_block;
    if ( this->m_first_block != as->m_first_block )
    {
        do
        {
            m_next_block = m_first_block->m_next_block;
            PSP_FREE(slot_pool, (unsigned __int8 *)m_first_block);
            m_first_block = m_next_block;
        }
        while ( m_next_block != as->m_first_block );
    }
    // nx-port: was a copy of the whole state struct over the first members,
    // whose padding on LP64 reached into m_mutex.
    this->m_first_block = as->m_first_block;
    this->m_cur = as->m_cur;
    this->m_end = as->m_end;
    this->m_total_memory_allocated = as->m_total_memory_allocated;
}

// nx-port: everything below did its pointer arithmetic in int (the bump
// pointer, the alignment, the compare-and-swap and the return value), so every
// allocation came back cut to 32 bits; and the block header was written at
// its x86 offsets, 12 bytes long. Rewritten on uintptr_t and the real header.
static char *phys_transient_align(char *p, int alignment)
{
    uintptr_t mask = (uintptr_t)alignment - 1;
    return (char *)(((uintptr_t)p + mask) & ~mask);
}

void __thiscall phys_transient_allocator::resize()
{
    phys_slot_pool *m_slot_pool = (phys_slot_pool *)this->m_slot_pool;
    if ( !m_slot_pool )
    {
        m_slot_pool = GET_PHYS_SLOT_POOL(0x4000u, 4u);
        this->m_slot_pool = m_slot_pool;
    }
    char *v3 = PSP_ALLOC(m_slot_pool);
    if ( v3 )
    {
        block_header *header = (block_header *)v3;
        header->m_block_size = 0x4000;
        header->m_block_alignment = 4;
        header->m_next_block = this->m_first_block;
        this->m_first_block = header;
        this->m_total_memory_allocated += 0x4000;
        this->m_cur = v3 + sizeof(block_header);
        this->m_end = v3 + 0x4000;
    }
}

char *phys_transient_allocator::mt_allocate_internal(int size, int alignment)
{
    char *cur;
    char *ptr;

    do
    {
        cur = this->m_cur;
        if ( !cur )
            return 0;
        ptr = phys_transient_align(cur, alignment);
        if ( ptr + size > this->m_end )
            return 0;
    } while ( !__atomic_compare_exchange_n(&this->m_cur, &cur, ptr + size, false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST) );
    return ptr;
}

// Bump-allocates from the current block; NULL if it does not fit.
static char *phys_transient_bump(phys_transient_allocator *a, int size, int alignment)
{
    if ( !a->m_cur )
        return 0;
    char *ptr = phys_transient_align(a->m_cur, alignment);
    if ( ptr + size > a->m_end )
        return 0;
    a->m_cur = ptr + size;
    return ptr;
}

void *__thiscall phys_transient_allocator::allocate(
    int size,
    int alignment,
    int no_error,
    const char *error_msg)
{
    transient_allocator_update_largest_size();
    void *ptr = phys_transient_bump(this, size, alignment);
    if (!ptr)
    {
        this->resize();
        ptr = phys_transient_bump(this, size, alignment);
        if (!ptr
            && _tlAssert(
                "C:\projects_pc\cod\codsrc\tl\physics\include\phys_transient_allocator.h",
                79,
                "ptr",
                "transient allocation too large, increase block_size."))
        {
            __debugbreak();
        }
    }
    if (!ptr
        && !no_error
        && _tlAssert(
            "C:\projects_pc\cod\codsrc\tl\physics\include\phys_transient_allocator.h",
            81,
            "ptr || no_error",
            error_msg))
    {
        __debugbreak();
    }
    return ptr;
}

void *__thiscall phys_transient_allocator::mt_allocate(
    int size,
    int alignment,
    int no_error,
    const char *error_msg)
{
    transient_allocator_update_largest_size();
    this->m_mutex.ReadLock();
    void *ptr = this->mt_allocate_internal(size, alignment);
    this->m_mutex.ReadUnlock();
    if (!ptr)
    {
        this->m_mutex.WriteLock();
        ptr = phys_transient_bump(this, size, alignment);
        if (!ptr)
        {
            this->resize();
            ptr = phys_transient_bump(this, size, alignment);
            if (!ptr
                && _tlAssert(
                    "c:\projects_pc\cod\codsrc\tl\physics\include\phys_transient_allocator.h",
                    99,
                    "ptr",
                    "transient allocation too large, increase block_size."))
            {
                __debugbreak();
            }
        }
        this->m_mutex.WriteUnlock();
    }
    if (!ptr
        && !no_error
        && _tlAssert(
            "c:\projects_pc\cod\codsrc\tl\physics\include\phys_transient_allocator.h",
            103,
            "ptr || no_error",
            error_msg))
    {
        __debugbreak();
    }
    return ptr;
}

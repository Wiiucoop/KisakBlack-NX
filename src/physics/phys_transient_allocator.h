#pragma once
#include "phys_local.h"
#include "phys_mem_new.h"

struct phys_transient_allocator // sizeof=0x18
{                                       // XREF: pulse_sum_constraint_solver/r
    struct block_header // sizeof=0xC
    {
        unsigned int m_block_size;
        unsigned int m_block_alignment;
        phys_transient_allocator::block_header *m_next_block;
    };
    struct allocator_state // sizeof=0x10
    {                                       // XREF: gjk_query_output/r
        phys_transient_allocator::block_header *m_first_block;
        char *m_cur;                        // XREF: pulse_sum_constraint_solver::execute_constraint_solver(rigid_body * const)+2B9/w
        char *m_end;                        // XREF: pulse_sum_constraint_solver::execute_constraint_solver(rigid_body * const)+2CE/w
        unsigned int m_total_memory_allocated;
    };
    phys_transient_allocator::block_header *m_first_block;
    char *m_cur;                        // XREF: physics_system::time_step(float,bool)+14F/w
    char *m_end;                        // XREF: physics_system::time_step(float,bool)+152/w
    unsigned int m_total_memory_allocated;
    minspec_read_write_mutex m_mutex;   // XREF: physics_system::time_step(float,bool)+158/w
    void *m_slot_pool;                  // XREF: physics_system::time_step(float,bool)+15F/w

    phys_transient_allocator()
    {
        m_first_block = NULL;
        m_cur = NULL;
        m_end = NULL;
        m_total_memory_allocated = 0; // 16

        m_mutex.m_count = 1;
        m_slot_pool = NULL;
    }

    ~phys_transient_allocator()
    {
        // LWSS: if this goes off, it usually means that the allocator is declared in the wrong scope.
        // Should be declared by "        memset(&transient_buffer, 0, 16);"
        if (this->m_first_block)
        {
            if (_tlAssert(
                "c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\phys_transient_allocator.h",
                69,
                "m_first_block == NULL",
                ""))
            {
                __debugbreak();
            }
        }
    }

    void *allocate(
        int size,
        int alignment,
        int no_error,
        const char *error_msg);

    void *mt_allocate(
        int size,
        int alignment,
        int no_error,
        const char *error_msg);
    char *mt_allocate_internal(int size, int alignment);

    void reset();
    void reset_to_state(const phys_transient_allocator::allocator_state *as);
    void resize();

    bool is_empty() const
    {
        return m_cur == NULL;
    }
};
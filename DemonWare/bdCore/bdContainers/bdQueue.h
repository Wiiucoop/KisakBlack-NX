#pragma once

#pragma once

#include "bdLinkedList.h"

/*
===============================================================================
  bdQueue<T>
===============================================================================

  Binary notes:
  - sizeof(bdQueue<T>) == 0x0C
  - Contains exactly one bdLinkedList<T>
  - Destructor directly calls bdLinkedList<T>::clear()
*/

template <typename T>
struct bdQueue
{
    // +0x00
    bdLinkedList<T> m_list;

    // --------------------------------------------------------
    // ctor / dtor
    // --------------------------------------------------------

    inline bdQueue() = default;

    inline ~bdQueue()
    {
        // Matches all observed instantiations:
        //  bdQueue<bdPendingBufferTransfer>
        //  bdQueue<bdEndpoint>
        //  bdQueue<bdUnicastConnection::bdControlChunkStore>
        m_list.clear();
    }

    // --------------------------------------------------------
    // convenience passthroughs (optional but typical)
    // --------------------------------------------------------

    inline unsigned int size() const
    {
        return m_list.m_size;
    }

    inline bool empty() const
    {
        return m_list.m_size == 0;
    }

    inline void clear()
    {
        m_list.clear();
    }
};

#ifndef KISAK_NX // nx-port: x86 layout assert
static_assert(sizeof(bdQueue<void *>) == 0x0C, "bdQueue size mismatch");
#endif

#include "StdAfx.h"

#include <NativeLib/Allocators.h>

namespace nl
{
    static IAllocator* m_allocator = nullptr;

    IAllocator* GetAllocator()
    {
        nl_assert_if_debug(m_allocator != nullptr);
        return m_allocator;
    }

    void SetAllocator(IAllocator* allocator)
    {
        m_allocator = allocator;
        m_allocator->AddRef();
    }

    void ReleaseAllocator()
    {
        if (m_allocator)
        {
            m_allocator->Release();
            m_allocator = nullptr;
        }
    }
}
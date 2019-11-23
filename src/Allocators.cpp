#include "StdAfx.h"

#include <NativeLib/Allocators.h>

namespace nl
{
    namespace memory
    {
        // memory class

        Memory::MemoryDataInfo Memory::s_zeroSizeData = { 1, 0 };
        const void* const Memory::s_zeroSizeDataPointer = reinterpret_cast<const void*>(&Memory::s_zeroSizeData + 1);

        Memory::Memory(void* lp) noexcept :
            m_lp(lp)
        {
        }

        Memory::~Memory()
        {
            auto info = reinterpret_cast<MemoryDataInfo*>(m_lp) - 1;
            if (InterlockedDecrement(&info->References) == 0)
                nl::memory::Free(info);
        }

        Memory::Memory(const Memory& memory) noexcept
        {
            auto info = reinterpret_cast<MemoryDataInfo*>(memory.m_lp) - 1;
            InterlockedIncrement(&info->References);

            m_lp = memory.m_lp;
        }

        Memory::Memory(Memory&& memory) noexcept
        {
            m_lp = memory.m_lp;

            InterlockedIncrement(&s_zeroSizeData.References);
            memory.m_lp = &s_zeroSizeData;
        }

        Memory& Memory::operator =(const Memory& memory) noexcept
        {
            auto info = reinterpret_cast<MemoryDataInfo*>(m_lp) - 1;
            if (InterlockedDecrement(&info->References) == 0)
                nl::memory::Free(info);

            info = reinterpret_cast<MemoryDataInfo*>(memory.m_lp) - 1;
            InterlockedIncrement(&info->References);

            m_lp = memory.m_lp;
            return *this;
        }

        Memory& Memory::operator =(Memory&& memory) noexcept
        {
            auto info = reinterpret_cast<MemoryDataInfo*>(m_lp) - 1;
            if (InterlockedDecrement(&info->References) == 0)
                nl::memory::Free(info);

            m_lp = memory.m_lp;

            InterlockedIncrement(&s_zeroSizeData.References);
            memory.m_lp = &s_zeroSizeData;
            return *this;
        }

        size_t Memory::GetSize() const
        {
            auto info = reinterpret_cast<MemoryDataInfo*>(m_lp) - 1;
            return info->Size;
        }

        Memory Memory::Allocate(size_t size)
        {
            if (size == 0)
            {
                nl_assert_if_debug(size != 0);

                InterlockedIncrement(&s_zeroSizeData.References);
                return Memory(&s_zeroSizeData);
            }

            auto info = reinterpret_cast<MemoryDataInfo*>(nl::memory::Allocate(sizeof(MemoryDataInfo) + size));
            auto lp = reinterpret_cast<void*>(info + 1);

            info->Size = size;
            info->References = 1;
            return Memory(lp);
        }

        ////////////////////////////////////////////////////////////////////////////////////////////////////////

        static AllocateCallback g_allocate = nullptr;
        static ReallocateCallback g_reallocate = nullptr;
        static FreeCallback g_free = nullptr;

        void SetMemoryManagement(AllocateCallback allocate, ReallocateCallback reallocate, FreeCallback free)
        {
            if (!allocate)
                throw ArgumentException(L"allocate must be set");

            if (!reallocate)
                throw ArgumentException(L"reallocate must be set");

            if (!free)
                throw ArgumentException(L"free must be set");

            if (g_allocate)
                throw InvalidOperationException(L"Setting memory allocation callbacks can only be done once.");

            g_allocate = allocate;
            g_reallocate = reallocate;
            g_free = free;
        }

        void* Allocate(size_t size)
        {
            nl_assert_if_debug(size != 0);
            return g_allocate(size);
        }

        void* AllocateThrow(size_t size)
        {
            nl_assert_if_debug(size != 0);

            auto ptr = g_allocate(size);;
            if (!ptr)
                throw BadAllocationException();

            return ptr;
        }

        void* Reallocate(void* ptr, size_t size)
        {
            nl_assert_if_debug(ptr != nullptr);
            return g_reallocate(ptr, size);
        }

        void Free(void* ptr)
        {
            nl_assert_if_debug(ptr != nullptr);
            g_free(ptr);
        }
    }
}
#include "StdAfx.h"

#include <NativeLib/Allocators.h>
#include <NativeLib/SystemLayer/SystemLayer.h>
#include <NativeLib/Threading/Interlocked.h>

#ifdef NL_PLATFORM_WINDOWS
#include <Windows.h>
#endif

namespace nl
{
    namespace memory
    {
        // memory class

        Memory::MemoryDataInfo Memory::s_zeroSizeData = { 1, 0 };
        void* Memory::s_zeroSizeDataPointer = reinterpret_cast<void*>(&Memory::s_zeroSizeData + 1);

        Memory::Memory(void* lp) noexcept :
            m_lp(lp)
        {
        }

        Memory::~Memory()
        {
            auto info = reinterpret_cast<MemoryDataInfo*>(m_lp) - 1;
            if (nl::threading::Interlocked::Decrement(&info->References) == 0)
            {
                nl::memory::Free(info);
            }
        }

        Memory::Memory(Memory&& memory) noexcept
        {
            m_lp = memory.m_lp;

            nl::threading::Interlocked::Increment(&s_zeroSizeData.References);
            memory.m_lp = s_zeroSizeDataPointer;
        }

        Memory& Memory::operator =(Memory&& memory) noexcept
        {
            auto info = reinterpret_cast<MemoryDataInfo*>(m_lp) - 1;
            if (nl::threading::Interlocked::Decrement(&info->References) == 0)
                nl::memory::Free(info);

            m_lp = memory.m_lp;

            nl::threading::Interlocked::Increment(&s_zeroSizeData.References);
            memory.m_lp = s_zeroSizeDataPointer;
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

                nl::threading::Interlocked::Increment(&s_zeroSizeData.References);
                return Memory(s_zeroSizeDataPointer);
            }

            auto info = reinterpret_cast<MemoryDataInfo*>(nl::memory::Allocate(sizeof(MemoryDataInfo) + size));
            auto lp = reinterpret_cast<void*>(info + 1);

            info->Size = size;
            info->References = 1;
            return Memory(lp);
        }

        ////////////////////////////////////////////////////////////////////////////////////////////////////////

        void* Allocate(size_t size)
        {
            nl_assert_if_debug(size != 0);
            return nl::systemlayer::GetSystemLayerFunctions()->AllocateHeapMemory(size);
        }

        void* AllocateThrow(size_t size)
        {
            nl_assert_if_debug(size != 0);

            auto ptr = nl::systemlayer::GetSystemLayerFunctions()->AllocateHeapMemory(size);;
            if (!ptr)
                throw BadAllocationException();

            return ptr;
        }

        void* Reallocate(void* ptr, size_t size)
        {
            nl_assert_if_debug(ptr != nullptr);
            return nl::systemlayer::GetSystemLayerFunctions()->ReallocateHeapMemory(ptr, size);
        }

        void Free(void* ptr)
        {
            nl_assert_if_debug(ptr != nullptr);
            nl::systemlayer::GetSystemLayerFunctions()->FreeHeapMemory(ptr);
        }
    }
}
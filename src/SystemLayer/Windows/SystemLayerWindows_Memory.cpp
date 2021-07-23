#include "StdAfx.h"

#ifdef NL_PLATFORM_WINDOWS

#include <NativeLib/SystemLayer/SystemLayer.h>

#include "SystemLayerWindows.h"

#include <NativeLib/Exceptions.h>
#include <NativeLib/Assert.h>

#include <Windows.h>

namespace nl::systemlayer::defaults
{
    static void* AllocateHeapMemory(size_t size)
    {
        LPVOID lp = HeapAlloc(GetProcessHeap(), 0, (SIZE_T)size);
        nl_assert_if_debug(lp != nullptr);

#ifdef _DEBUG
        memset(lp, 0xcd, size);
#endif

        return lp;
    }

    static void* ReallocateHeapMemory(void* ptr, size_t new_size)
    {
        if (ptr == nullptr)
            return AllocateHeapMemory(new_size);

#ifdef _DEBUG
        SIZE_T old_size = HeapSize(GetProcessHeap(), 0, reinterpret_cast<void*>(reinterpret_cast<ULONG_PTR>(ptr) / 4096 * 4096));
#endif

        uint8_t* new_ptr = (uint8_t*)HeapReAlloc(GetProcessHeap(), 0, ptr, (SIZE_T)new_size);

        nl_assert_if_debug(new_ptr != nullptr);

#ifdef _DEBUG
        if (new_size > old_size)
            memset(new_ptr + old_size, 0xcd, new_size - old_size);
#endif

        return new_ptr;
    }

    static void FreeHeapMemory(void* ptr)
    {
        nl_assert_if_debug(ptr != nullptr);

#ifdef _DEBUG
        SIZE_T size = HeapSize(GetProcessHeap(), 0, ptr);
        memset(ptr, 0xdd, (size_t)size);
#endif

        HeapFree(GetProcessHeap(), 0, ptr);
    }

    static int64_t GetVirtualMemoryPageSize()
    {
        SYSTEM_INFO si;
        GetSystemInfo(&si);
        return (int64_t)si.dwPageSize;
    }

    static void* AllocateVirtualMemory(size_t size)
    {
        LPVOID lp = VirtualAlloc(nullptr, (SIZE_T)size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
        nl_assert_if_debug(lp != nullptr);

#ifdef _DEBUG
        memset(lp, 0xcd, size);
#endif

        return lp;
    }

    static void FreeVirtualMemory(void* ptr)
    {
        VirtualFree(ptr, 0, MEM_RELEASE);
    }

    bool SetMemory(SystemLayerFunctions* functions)
    {
        functions->AllocateHeapMemory = AllocateHeapMemory;
        functions->ReallocateHeapMemory = ReallocateHeapMemory;
        functions->FreeHeapMemory = FreeHeapMemory;
        functions->GetVirtualMemoryPageSize = GetVirtualMemoryPageSize;
        functions->AllocateVirtualMemory = AllocateVirtualMemory;
        functions->FreeVirtualMemory = FreeVirtualMemory;
        return true;
    }
}

#endif
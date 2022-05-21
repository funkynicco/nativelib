#include "StdAfx.h"

#ifdef NL_PLATFORM_LINUX

#include <NativeLib/SystemLayer/SystemLayer.h>

//!ALLOW_INCLUDE "SystemLayerLinux.h"
#include "SystemLayerLinux.h"

#include <NativeLib/Exceptions.h>
#include <NativeLib/Assert.h>

namespace nl::systemlayer::defaults
{
    static void* AllocateHeapMemory(size_t size)
    {
        throw NotImplementedException();
    }

    static void* ReallocateHeapMemory(void* ptr, size_t new_size)
    {
        throw NotImplementedException();
    }

    static void FreeHeapMemory(void* ptr)
    {
        throw NotImplementedException();
    }

    static int64_t GetVirtualMemoryPageSize()
    {
        throw NotImplementedException();
    }

    static void* AllocateVirtualMemory(size_t size)
    {
        throw NotImplementedException();
    }

    static void FreeVirtualMemory(void* ptr)
    {
        throw NotImplementedException();
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
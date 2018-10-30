#include "StdAfx.h"

#include <NativeLib/Allocators.h>

namespace nl
{
    namespace memory
    {
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
            return g_allocate(size);
        }

        void* Reallocate(void* ptr, size_t size)
        {
            return g_reallocate(ptr, size);
        }

        void Free(void* ptr)
        {
            g_free(ptr);
        }
    }
}
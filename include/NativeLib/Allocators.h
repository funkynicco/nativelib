#pragma once

#include <NativeLib/Definitions.h>

namespace nl
{
    namespace memory
    {
        typedef void*(*AllocateCallback)(size_t size);
        typedef void*(*ReallocateCallback)(void* ptr, size_t size);
        typedef void(*FreeCallback)(void* ptr);

        void SetMemoryManagement(AllocateCallback allocate, ReallocateCallback reallocate, FreeCallback free);

        void* Allocate(size_t size);
        void* Reallocate(void* ptr, size_t size);
        void Free(void* ptr);
    }
}
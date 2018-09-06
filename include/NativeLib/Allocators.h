#pragma once

#include <NativeLib/Definitions.h>

namespace nl
{
    struct IAllocator
    {
        virtual void* Allocate(size_t size) = 0;
        virtual void* Reallocate(void* ptr, size_t newSize) = 0;
        virtual void Free(void* ptr) = 0;

        virtual LONG AddRef() = 0;
        virtual LONG Release() = 0;
    };
}
#pragma once

#include <NativeLib/Allocators.h>

namespace raii::methods
{
    template <typename T>
    static void Release(T* obj) { obj->Release(); }

    static void Free(void* obj) { nl::memory::Free(obj); }
}
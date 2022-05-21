#include "StdAfx.h"

#include <NativeLib/Threading/Interlocked.h>

//!ALLOW_INCLUDE "Windows.h"

#ifdef NL_PLATFORM_WINDOWS
#include <Windows.h>
#endif

namespace nl::threading
{
    int32_t Interlocked::Increment(volatile int32_t* value)
    {
#if defined(NL_PLATFORM_WINDOWS)
        return (int32_t)InterlockedIncrement((volatile long*)value);
#else
        return __sync_add_and_fetch(value, 1);
#endif
    }

    int32_t Interlocked::Decrement(volatile int32_t* value)
    {
#if defined(NL_PLATFORM_WINDOWS)
        return (int32_t)InterlockedDecrement((volatile long*)value);
#else
        return __sync_sub_and_fetch(value, 1);
#endif
    }

    int64_t Interlocked::Increment(volatile int64_t* value)
    {
#if defined(NL_PLATFORM_WINDOWS)
        return (int64_t)InterlockedIncrement64((volatile long long*)value);
#else
        throw NotImplementedException();
#endif
    }

    int64_t Interlocked::Decrement(volatile int64_t* value)
    {
#if defined(NL_PLATFORM_WINDOWS)
        return (int64_t)InterlockedDecrement64((volatile long long*)value);
#else
        throw NotImplementedException();
#endif
    }
}
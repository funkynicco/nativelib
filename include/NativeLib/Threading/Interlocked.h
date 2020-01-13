#pragma once

#include <stdint.h>

namespace nl::threading
{
    class Interlocked
    {
    public:
        static int32_t Increment(volatile int32_t* value);
        static int32_t Decrement(volatile int32_t* value);
        static int64_t Increment(volatile int64_t* value);
        static int64_t Decrement(volatile int64_t* value);
    };
}
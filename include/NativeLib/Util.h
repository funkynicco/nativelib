#pragma once

#include <NativeLib/String.h>

namespace nl::util
{
    template <typename T>
    constexpr T Min(T left, T right)
    {
        return (left < right) ? left : right;
    }

    template <typename T>
    constexpr T Max(T left, T right)
    {
        return (left < right) ? right : left;
    }

    template <typename T>
    constexpr T Clamp(T value, T min, T max)
    {
        if (value < min)
            return min;

        if (value > max)
            return max;

        return value;
    }

    template <typename T>
    constexpr T Pow(T value, size_t power)
    {
        T result = 1;

        for (size_t i = 0; i < power; ++i)
        {
            result *= value;
        }

        return result;
    }

    // Generates a value that is equivalent or above the given value in the form of squared (2, 4, 8, 16, 32, 64, ...).
    template <typename T>
    constexpr T GetClosestSquared(T value)
    {
        T result = 1;
        while (result < value)
        {
            result <<= 1;
        }

        return result;
    }

    inline nl::String GetSize(size_t size)
    {
        double v = (double)size;
        int n = 0;
        while (v >= 1024.0)
        {
            v /= 1024.0;
            ++n;
        }

        char buf[128];
        int len = 0;

        switch (n)
        {
        case 0:
#ifdef NL_PLATFORM_WINDOWS
            len = sprintf(buf, "%llu B", size);
#else
            len = sprintf(buf, "%lu B", size);
#endif
            break;
        case 1:
            len = sprintf(buf, "%.2f KB", v);
            break;
        case 2:
            len = sprintf(buf, "%.2f MB", v);
            break;
        case 3:
            len = sprintf(buf, "%.2f GB", v);
            break;
        case 4:
            len = sprintf(buf, "%.2f TB", v);
            break;
        case 5:
            len = sprintf(buf, "%.2f PB", v);
            break;
        case 6:
            len = sprintf(buf, "%.2f EB", v);
            break;
        case 7:
            len = sprintf(buf, "%.2f ZB", v);
            break;
        case 8:
            len = sprintf(buf, "%.2f YB", v);
            break;
        }

        return nl::String(buf, len);
    }
}
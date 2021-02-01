#pragma once

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

        return value;
    }
}
#pragma once

namespace nl
{
    template <class TEnum, typename TValue = int>
    class Enum
    {
    public:
        constexpr Enum(TEnum value) :
            m_value((TValue)value)
        {
        }

        // Checks whether one of the provided values are set in the enum.
        template <typename... Args>
        constexpr bool Any(Args... value) const
        {
            auto combined = Combine(value...);
            return (m_value & (TValue)combined) != 0;
        }

        // Checks if all the provided values are set in the enum.
        template <typename... Args>
        constexpr bool All(Args... value) const
        {
            auto combined = Combine(value...);
            return (m_value & (TValue)combined) == (TValue)combined;
        }

        constexpr bool Has(TEnum value) const
        {
            return (m_value & (TValue)value) != 0;
        }

        constexpr void Set(TEnum value)
        {
            m_value |= (TValue)value;
        }

        constexpr void Reset(TEnum value)
        {
            m_value &= ~(TValue)value;
        }

        constexpr operator bool() const
        {
            return m_value != 0;
        }

        constexpr operator TEnum() const
        {
            return (TEnum)m_value;
        }

        // expression operators
        constexpr Enum operator &(TEnum value) const
        {
            return (TEnum)(m_value & (TValue)value);
        }

        constexpr Enum operator |(TEnum value) const
        {
            return (TEnum)(m_value | (TValue)value);
        }

        constexpr Enum operator ^(TEnum value) const
        {
            return (TEnum)(m_value ^ (TValue)value);
        }

        constexpr Enum operator ~() const
        {
            return (TEnum)~m_value;
        }

        // assignment operators
        constexpr Enum& operator &=(TEnum value)
        {
            m_value &= (TValue)value;
            return *this;
        }

        constexpr Enum& operator |=(TEnum value)
        {
            m_value |= (TValue)value;
            return *this;
        }

        constexpr Enum& operator ^=(TEnum value)
        {
            m_value ^= (TValue)value;
            return *this;
        }

    private:
        TValue m_value;

        template <typename... Args>
        static constexpr auto Combine(TEnum first, Args... args)
        {
            if constexpr (sizeof...(Args) > 0) // handle the last value where args is 0 compiler errors
            {
                return (TEnum)((TValue)first | (TValue)Combine(args...));
            }

            return first;
        }
    };
}
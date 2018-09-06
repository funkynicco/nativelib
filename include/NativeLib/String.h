#pragma once

#include <NativeLib/Allocators.h>
#include <NativeLib/Assert.h>

#include <cstring>
#include <any>
#include <array>

// mutable string (it changes itself; for performance benefit in C++)

namespace nl
{
    template <size_t StackSize>
    class BasicString
    {
    public:
        static const size_t npos = (size_t)-1;

        static_assert(StackSize >= 2, "StackSize should at least be 2 bytes or larger.");

        inline BasicString(IAllocator* allocator) :
            m_allocator(allocator),
            m_nCapacity(sizeof(m_stack)),
            m_nLength(0),
            m_pString(m_stack)
        {
            allocator->AddRef();
            *m_stack = 0;
        }

        inline ~BasicString()
        {
            if (m_pString != m_stack)
                m_allocator->Free(m_pString);

            m_allocator->Release();
        }

        inline BasicString(IAllocator* allocator, const char* str) :
            m_allocator(allocator),
            m_nCapacity(sizeof(m_stack)),
            m_nLength(0),
            m_pString(m_stack)
        {
            allocator->AddRef();
            size_t len = strlen(str);
            Set(str, len);
        }

        inline BasicString(IAllocator* allocator, const void* str, size_t len) :
            m_allocator(allocator),
            m_nCapacity(sizeof(m_stack)),
            m_nLength(0),
            m_pString(m_stack)
        {
            allocator->AddRef();
            Set(str, len);
        }

        template <size_t OtherSize>
        inline BasicString(const BasicString<OtherSize>& str) :
            m_allocator(str.m_allocator),
            m_nCapacity(sizeof(m_stack)),
            m_nLength(0),
            m_pString(m_stack)
        {
            m_allocator->AddRef();
            Set(str.m_pString, str.m_nLength);
        }

        inline BasicString(const BasicString& str) :
            m_allocator(str.m_allocator),
            m_nCapacity(sizeof(m_stack)),
            m_nLength(0),
            m_pString(m_stack)
        {
            m_allocator->AddRef();
            Set(str.m_pString, str.m_nLength);
        }

        template <size_t OtherSize>
        inline BasicString(BasicString<OtherSize>&& str) :
            m_allocator(str.m_allocator),
            m_nCapacity(sizeof(m_stack)),
            m_nLength(0),
            m_pString(m_stack)
        {
            m_allocator->AddRef();

            if (str.m_nLength + 1 > StackSize &&
                str.m_pString != str.m_stack)
            {
                m_nCapacity = str.m_nCapacity;
                m_nLength = str.m_nLength;
                m_pString = str.m_pString;
                str.m_pString = str.m_stack;
            }
            else
                Set(str.m_pString, str.m_nLength);
        }

        inline BasicString(BasicString&& str) :
            m_allocator(str.m_allocator),
            m_nCapacity(sizeof(m_stack)),
            m_nLength(0),
            m_pString(m_stack)
        {
            m_allocator->AddRef();

            if (str.m_nLength + 1 > StackSize &&
                str.m_pString != str.m_stack)
            {
                m_nCapacity = str.m_nCapacity;
                m_nLength = str.m_nLength;
                m_pString = str.m_pString;
                str.m_pString = str.m_stack;
            }
            else
                Set(str.m_pString, str.m_nLength);
        }

        inline BasicString& operator =(const char* str)
        {
            size_t len = strlen(str);
            Set(str, len);
        }

        inline BasicString& operator =(const BasicString& str)
        {
            Set(str.m_pString, str.m_nLength);
            return *this;
        }

        template <size_t OtherSize>
        inline BasicString& operator =(const BasicString<OtherSize>& str)
        {
            Set(str.m_pString, str.m_nLength);
            return *this;
        }

        template <size_t OtherSize>
        inline BasicString& operator =(BasicString<OtherSize>&& str)
        {
            if (str.m_nLength + 1 > StackSize &&
                str.m_pString != str.m_stack)
            {
                if (m_pString != m_stack)
                {
                    m_allocator->Free(m_pString);
                    m_allocator->Release();
                    m_allocator = str.m_allocator;
                    m_allocator->AddRef();
                }

                m_nCapacity = str.m_nCapacity;
                m_nLength = str.m_nLength;
                m_pString = str.m_pString;
                str.m_pString = str.m_stack;
            }
            else
                Set(str.m_pString, str.m_nLength);

            return *this;
        }

        inline BasicString& operator =(BasicString&& str)
        {
            if (str.m_nLength + 1 > StackSize &&
                str.m_pString != str.m_stack)
            {
                if (m_pString != m_stack)
                {
                    m_allocator->Free(m_pString);
                    m_allocator->Release();
                    m_allocator = str.m_allocator;
                    m_allocator->AddRef();
                }

                m_nCapacity = str.m_nCapacity;
                m_nLength = str.m_nLength;
                m_pString = str.m_pString;
                str.m_pString = str.m_stack;
            }
            else
                Set(str.m_pString, str.m_nLength);

            return *this;
        }

        inline char& operator [](size_t index) { return m_pString[index]; }
        inline char operator [](size_t index) const { return m_pString[index]; }

        inline const char* c_str() const { return m_pString; }
        inline operator const char*() const { return m_pString; }

        inline size_t GetLength() const { return m_nLength; }
        inline size_t GetCapacity() const { return m_nCapacity; }

        template <size_t OtherSize>
        inline BasicString operator +(const BasicString<OtherSize>& str) const
        {
            BasicString result = *this;
            result.Append(str.m_pString, str.m_nLength);
            return result;
        }

        inline BasicString operator +(const BasicString& str) const
        {
            BasicString result = *this;
            result.Append(str.m_pString, str.m_nLength);
            return result;
        }

        inline BasicString operator +(const char* str) const
        {
            BasicString result = *this;
            result.Append(str, strlen(str));
            return result;
        }

        template <size_t OtherSize>
        inline BasicString& operator +=(const BasicString<OtherSize>& str) { Append(str.m_pString, str.m_nLength); return *this; }
        inline BasicString& operator +=(const BasicString& str) { Append(str.m_pString, str.m_nLength); return *this; }
        inline BasicString& operator +=(const char* str) { Append(str, strlen(str)); return *this; }

        inline bool operator ==(const char* str) const { return strcmp(m_pString, str) == 0; }
        template <size_t OtherSize>
        inline bool operator ==(const BasicString<OtherSize>& str) { return strcmp(m_pString, str.m_pString) == 0; }
        inline bool operator ==(const BasicString& str) { return strcmp(m_pString, str.m_pString) == 0; }

        inline void Clear()
        {
            m_nLength = 0;
            *m_pString = 0;
        }

        inline void Append(const void* str, size_t len)
        {
            EnsureCapacity(m_nLength + len + 1);
            memcpy(m_pString + m_nLength, str, len);
            m_nLength += len;
            m_pString[m_nLength] = 0;
        }

        inline void Append(char c) { Append(&c, 1); }

        inline void Append(const char* str) { Append(str, strlen(str)); }
        template <size_t OtherSize>
        inline void Append(const BasicString<OtherSize>& str) { Append(str.m_pString, str.m_nLength); }
        inline void Append(const BasicString& str) { Append(str.m_pString, str.m_nLength); }

        inline BasicString Append(const void* str, size_t len) const
        {
            BasicString result = *this;
            result.Append(str, len);
            return result;
        }

        inline BasicString Append(const char* str) const { return Append(str, strlen(str)); }
        template <size_t OtherSize>
        inline BasicString Append(const BasicString<OtherSize>& str) const { return Append(str.m_pString, str.m_nLength); }
        inline BasicString Append(const BasicString& str) const { return Append(str.m_pString, str.m_nLength); }

        inline void Delete(size_t index, size_t count)
        {
            nl_assert(index + count < m_nLength);
            memmove(m_pString + index, m_pString + index + count, m_nLength - (index + count));
            m_nLength -= count;
            m_pString[m_nLength] = 0;
        }

        inline void Delete(size_t count) { Delete(0, count); }

        inline void Insert(size_t index, const void* str, size_t len)
        {
            EnsureCapacity(m_nLength + len + 1);
            memmove(m_pString + index + len, m_pString + index, m_nLength - index);
            memcpy(m_pString + index, str, len);
            m_nLength += len;
            m_pString[m_nLength] = 0;
        }

        inline void Insert(size_t index, const char* str) { Insert(index, str, strlen(str)); }
        inline void Insert(size_t index, const BasicString& str) { Insert(index, str.m_pString, str.m_nLength); }

        inline size_t IndexOf(const void* str, size_t len, size_t start) const
        {
            nl_assert(start <= m_nLength);
            if (len > m_nLength)
                return npos;

            size_t i;
            const char* p = m_pString + start;
            const char* until = m_pString + start + ((m_nLength - start) - len);
            while (p <= until)
            {
                for (i = 0; i < len; ++i)
                {
                    if (p[i] != *((const char*)str + i))
                        break;
                }

                if (i == len)
                    return size_t(p - m_pString);

                ++p;
            }

            return npos;
        }

        inline size_t IndexOf(const char* str, size_t start = 0) const { return IndexOf(str, strlen(str), start); }
        template <size_t OtherSize>
        inline size_t IndexOf(const BasicString<OtherSize>& str, size_t start = 0) const { return IndexOf(str.m_pString, str.m_nLength, start); }
        inline size_t IndexOf(const BasicString& str, size_t start = 0) const { return IndexOf(str.m_pString, str.m_nLength, start); }
        inline size_t IndexOf(char c, size_t start = 0) const { return IndexOf(&c, 1, start); }

        inline BasicString Substring(size_t index, size_t count = npos) const
        {
            if (count == npos)
                count = m_nLength - index;

            nl_assert(index + count <= m_nLength);
            return BasicString(m_pString + index, count);
        }

        inline bool StartsWith(const void* str, size_t len) const
        {
            if (len > m_nLength)
                return FALSE;

            return memcmp(m_pString, str, len) == 0;
        }

        inline bool StartsWith(const char* str) const { return StartsWith(str, strlen(str)); }
        template <size_t OtherSize>
        inline bool StartsWith(const BasicString<OtherSize>& str) const { return StartsWith(str.m_pString, str.m_nLength); }
        inline bool StartsWith(const BasicString& str) const { return StartsWith(str.m_pString, str.m_nLength); }

        inline bool EndsWith(const void* str, size_t len) const
        {
            if (len > m_nLength)
                return FALSE;

            return memcmp(m_pString + m_nLength - len, str, len) == 0;
        }

        inline bool EndsWith(const char* str) const { return EndsWith(str, strlen(str)); }
        template <size_t OtherSize>
        inline bool EndsWith(const BasicString<OtherSize>& str) const { return EndsWith(str.m_pString, str.m_nLength); }
        inline bool EndsWith(const BasicString& str) const { return EndsWith(str.m_pString, str.m_nLength); }

        inline void PadLeft(size_t count, char ch = ' ') // right-aligns characters
        {
            if (m_nLength >= count)
                return;

            EnsureCapacity(count + 1);

            memmove(m_pString + count - m_nLength, m_pString, m_nLength);
            memset(m_pString, ch, count - m_nLength);
            m_nLength = count;
            m_pString[m_nLength] = 0;
        }

        inline void PadRight(size_t count, char ch = ' ') // left-aligns characters
        {
            if (m_nLength >= count)
                return;

            EnsureCapacity(count + 1);

            memset(m_pString + m_nLength, ch, count - m_nLength);
            m_nLength = count;
            m_pString[m_nLength] = 0;
        }

        inline void Trim()
        {
            if (m_nLength == 0)
                return;

            const char* start = m_pString;
            while (*start &&
                (*start == ' ' || *start == '\t'))
                ++start;

            const char* end = m_pString + m_nLength;
            while (end > start &&
                (*(end - 1) == ' ' || *(end - 1) == '\t'))
                --end;

            m_nLength = size_t(end - start);
            memmove(m_pString, start, m_nLength);
            m_pString[m_nLength] = 0;
        }

        inline void MakeLower()
        {
            _strlwr(m_pString);
        }

        inline void MakeUpper()
        {
            _strupr(m_pString);
        }

        inline void EnsureCapacity(size_t nCapacity)
        {
            if (nCapacity <= m_nCapacity)
                return;

            m_nCapacity = nCapacity;
            if (m_pString == m_stack)
            {
                m_pString = reinterpret_cast<char*>(m_allocator->Allocate(m_nCapacity));
                nl_assert(m_pString != NULL);
                memcpy(m_pString, m_stack, GetLength() + 1);
            }
            else
            {
                m_pString = reinterpret_cast<char*>(m_allocator->Reallocate(m_pString, m_nCapacity));
                nl_assert(m_pString != NULL);
            }
        }

        inline void Set(const void* str, size_t len)
        {
            EnsureCapacity(len + 1);

            memcpy(m_pString, str, len);
            m_pString[len] = 0;
            m_nLength = len;
        }

#define AppendTemplateValue_Impl_For(type) \
    template <size_t OtherSize, std::enable_if_t<std::is_same_v<type, type>>* = nullptr> \
    inline static void AppendTemplateValue(IAllocator* allocator, BasicString<OtherSize>& str, type value)

        AppendTemplateValue_Impl_For(const char*)
        {
            str.Append(value);
        }

        AppendTemplateValue_Impl_For(int)
        {
            str.Append(IntToString(allocator, value));
        }

        AppendTemplateValue_Impl_For(float)
        {
            str.Append(IntToString(allocator, (int)value));
        }

        template <
            size_t OtherSize,
            size_t ThirdSize,
            std::enable_if_t<std::is_same_v<const BasicString<ThirdSize>&, const BasicString<ThirdSize>&>>* = nullptr>
        inline static void AppendTemplateValue(IAllocator* allocator, BasicString<OtherSize>& str, const BasicString<ThirdSize>& value)
        {
            str.Append(value);
        }

        inline static BasicString IntToString(IAllocator* allocator, int value)
        {
            char buf[32] = {};
            char* p = buf + sizeof(buf) - 1;

            bool is_negative = value < 0;
            if (is_negative)
                value = -value;

            while (value != 0)
            {
                *--p = '0' + (value % 10);
                value /= 10;
            }

            if (is_negative)
                *--p = '-';

            return BasicString(allocator, p);
        }

        // this handles when Args... gets empty, as an overload for when the "T value" is not supplied
        template <size_t OtherSize>
        inline static void Test(
            IAllocator* allocator,
            BasicString<OtherSize>& output,
            const char* format,
            const char* format_end)
        {
            while (format < format_end)
            {
                if (*format == '{')
                {
                    if (format + 1 >= format_end)
                        return;

                    ++format;

                    if (*format == '}')
                    {
                        ++format;
                        output.Append("{}");
                        continue;
                    }

                    return;
                }
                else
                    output.Append(*format++);
            }
        }

        template <size_t OtherSize, typename T, typename... Args>
        inline static void Test(
            IAllocator* allocator,
            BasicString<OtherSize>& output,
            const char* format,
            const char* format_end,
            const T& value,
            const Args&... args)
        {
            while (format < format_end)
            {
                if (*format == '{')
                {
                    if (format + 1 >= format_end)
                        return;

                    if (*(format + 1) == '{')
                    {
                        format += 2;
                        output.Append('{');
                        continue;
                    }

                    ++format;
                    if (*format != '}')
                        return;

                    AppendTemplateValue(allocator, output, value);
                    ++format;

                    Test(allocator, output, format, format_end, args...);
                    return;
                }
                else if (*format == '}' &&
                    format + 1 < format_end &&
                    *(format + 1) == '}')
                {
                    format += 2;
                    output.Append('}');
                }
                else
                    output.Append(*format++);
            }
        }

        template <typename... Args>
        inline static BasicString Format(IAllocator* allocator, const char* format, const Args&... args)
        {
            BasicString output(allocator);
            const char* format_end = format + strlen(format);

            //std::array<std::any, sizeof...(args)> test = { args... };

            Test(allocator, output, format, format_end, args...);

            return output;
        }

        /*inline static BasicString Format(const char* format, ...)
        {
            va_list l;
            va_start(l, format);
            BasicString str = FormatArgs(format, l);
            va_end(l);
            return str;
        }

        template <size_t OtherSize>
        inline static BasicString Format(const BasicString<OtherSize>& format, ...)
        {
            va_list l;
            va_start(l, format);
            BasicString str = FormatArgs(format.c_str(), l);
            va_end(l);
            return str;
        }

        inline static BasicString FormatArgs(const char* format, va_list args)
        {
            BasicString str;
            int n = _vscprintf(format, args);
            str.EnsureCapacity(n + 1);
            vsprintf_s(str.m_pString, str.m_nCapacity, format, args);
            str.m_nLength = n;
            return str;
        }

        template <size_t OtherSize>
        inline static BasicString FormatArgs(const BasicString<OtherSize>& format, va_list args)
        {
            return FormatArgs(format.c_str(), args);
        }*/

#ifdef _WIN32____
        static BasicString FromHResult(HRESULT hr, va_list* l = NULL)
        {
            LPSTR pstr = NULL;
            DWORD dwSize = FormatMessage(
                FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER,
                NULL,
                hr,
                LANG_NEUTRAL,
                (LPSTR)&pstr,
                0,
                l);

            if (dwSize == 0 ||
                !pstr)
                return "";

            if (dwSize > 2 &&
                pstr[dwSize - 2] == '\r' &&
                pstr[dwSize - 1] == '\n')
            {
                BasicString str(pstr, dwSize - 2);
                LocalFree(pstr);
                return str;
            }

            BasicString str(pstr, dwSize);
            LocalFree(pstr);
            return str;
    }
#endif

    private:
        template <size_t> friend class BasicString;

        IAllocator* m_allocator;

        size_t m_nCapacity;
        size_t m_nLength;
        char* m_pString;
        char m_stack[StackSize];
};

    typedef BasicString<256> LargeString;
    typedef BasicString<64> String;
    typedef BasicString<16> TinyString;
}
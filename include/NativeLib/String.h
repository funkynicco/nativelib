#pragma once

#include <NativeLib/Allocators.h>
#include <NativeLib/Assert.h>

#include <cstring>
#include <any>
#include <array>
#include <cstdio>

#include <type_traits>
#include <string_view>

// mutable string (it changes itself; for performance benefit in C++)

// TODO: change to only have a char* in the class so it's compatible with printf
// TODO: implement string_view and basic_ostream conversion for std::cout
// TODO: most of the function definitions should be in a cpp, only keep templating ones in header

// TODO: add more std::string_view compatibility things

namespace nl
{
	class String
	{
	public:
		static constexpr size_t StackSize = 32; // max length: abcdefghijklmnopqrstuvwxyzabcde

		static const size_t npos = (size_t)-1;

		String() :
			m_nCapacity(sizeof(m_stack)),
			m_nLength(0),
			m_pString(m_stack)
		{
			*m_stack = 0;
		}

		~String()
		{
			if (m_pString != m_stack)
				nl::memory::Free(m_pString);
		}

		String(const char* str) :
			m_nCapacity(sizeof(m_stack)),
			m_nLength(0),
			m_pString(m_stack)
		{
			size_t len = strlen(str);
			Set(str, len);
		}

		String(const void* str, size_t len) :
			m_nCapacity(sizeof(m_stack)),
			m_nLength(0),
			m_pString(m_stack)
		{
			Set(str, len);
		}

		String(size_t len, char ch) :
			m_nCapacity(sizeof(m_stack)),
			m_nLength(0),
			m_pString(m_stack)
		{
			EnsureCapacity(len + 1);
			memset(m_pString, ch, len);
			m_nLength = len;
			m_pString[len] = 0;
		}

		String(std::string_view str) :
			m_nCapacity(sizeof(m_stack)),
			m_nLength(0),
			m_pString(m_stack)
		{
			Set(str.data(), str.length());
		}

		String(const String& str) :
			m_nCapacity(sizeof(m_stack)),
			m_nLength(0),
			m_pString(m_stack)
		{
			Set(str.m_pString, str.m_nLength);
		}

		String(String&& str) :
			m_nCapacity(sizeof(m_stack)),
			m_nLength(0),
			m_pString(m_stack)
		{
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

		String& operator =(const char* str)
		{
			size_t len = strlen(str);
			Set(str, len);
            return *this;
		}

		String& operator =(const String& str)
		{
			Set(str.m_pString, str.m_nLength);
			return *this;
		}

		String& operator =(String&& str)
		{
			if (str.m_nLength + 1 > StackSize &&
				str.m_pString != str.m_stack)
			{
				if (m_pString != m_stack)
                    nl::memory::Free(m_pString);

				m_nCapacity = str.m_nCapacity;
				m_nLength = str.m_nLength;
				m_pString = str.m_pString;
				str.m_pString = str.m_stack;
			}
			else
				Set(str.m_pString, str.m_nLength);

			return *this;
		}

		char& operator [](size_t index)
		{
			nl_assert_if_debug(index < m_nLength);
			return m_pString[index];
		}

		char operator [](size_t index) const
		{
			nl_assert_if_debug(index < m_nLength);
			return m_pString[index];
		}

		char* data() { return m_pString; }
		const char* c_str() const { return m_pString; }
		operator const char*() const { return m_pString; }
		operator std::string_view() const { return std::string_view(m_pString, m_nLength); }

		size_t GetLength() const { return m_nLength; }
		size_t GetCapacity() const { return m_nCapacity; }

		String operator +(const String& str) const
		{
			String result = *this;
			result.Append(str.m_pString, str.m_nLength);
			return result;
		}

		String operator +(const char* str) const
		{
			String result = *this;
			result.Append(str, strlen(str));
			return result;
		}

		String& operator +=(const String& str) { Append(str.m_pString, str.m_nLength); return *this; }
		String& operator +=(const char* str) { Append(str, strlen(str)); return *this; }

		bool operator ==(const char* str) const { return strcmp(m_pString, str) == 0; }
		bool operator ==(const String& str) { return strcmp(m_pString, str.m_pString) == 0; }

		void Clear()
		{
			m_nLength = 0;
			*m_pString = 0;
		}

		char* GetBuffer() { return m_pString; }
		const char* GetBuffer() const { return m_pString; }

		void SetLength(size_t length)
		{
			EnsureCapacity(length + 1);
			m_nLength = length;
			m_pString[length] = 0;
		}

		void Append(const void* str, size_t len)
		{
			EnsureCapacity(m_nLength + len + 1);
			memcpy(m_pString + m_nLength, str, len);
			m_nLength += len;
			m_pString[m_nLength] = 0;
		}

		void Append(char c) { Append(&c, 1); }

		void Append(const char* str) { Append(str, strlen(str)); }
		void Append(const String& str) { Append(str.m_pString, str.m_nLength); }

		String Append(const void* str, size_t len) const
		{
			String result = *this;
			result.Append(str, len);
			return result;
		}

		String Append(const char* str) const { return Append(str, strlen(str)); }
		String Append(const String& str) const { return Append(str.m_pString, str.m_nLength); }

		void Delete(size_t index, size_t count)
		{
			nl_assert(index + count < m_nLength);
			memmove(m_pString + index, m_pString + index + count, m_nLength - (index + count));
			m_nLength -= count;
			m_pString[m_nLength] = 0;
		}

		void Delete(size_t count) { Delete(0, count); }

		void Insert(size_t index, const void* str, size_t len)
		{
			EnsureCapacity(m_nLength + len + 1);
			memmove(m_pString + index + len, m_pString + index, m_nLength - index);
			memcpy(m_pString + index, str, len);
			m_nLength += len;
			m_pString[m_nLength] = 0;
		}

		void Insert(size_t index, const char* str) { Insert(index, str, strlen(str)); }
		void Insert(size_t index, const String& str) { Insert(index, str.m_pString, str.m_nLength); }

		size_t IndexOf(const void* str, size_t len, size_t start) const
		{
			nl_assert_if_debug(start <= m_nLength);
			if (start + len > m_nLength)
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

		size_t IndexOf(const char* str, size_t start = 0) const { return IndexOf(str, strlen(str), start); }
		size_t IndexOf(const String& str, size_t start = 0) const { return IndexOf(str.m_pString, str.m_nLength, start); }
		size_t IndexOf(char c, size_t start = 0) const { return IndexOf(&c, 1, start); }

		String Substring(size_t index, size_t count = npos) const
		{
			if (count == npos)
				count = m_nLength - index;

			nl_assert_if_debug(index + count <= m_nLength);
			return String(m_pString + index, count);
		}

		bool StartsWith(const void* str, size_t len) const
		{
			if (len > m_nLength)
				return false;

			return memcmp(m_pString, str, len) == 0;
		}

		bool StartsWith(const char* str) const { return StartsWith(str, strlen(str)); }
		bool StartsWith(const String& str) const { return StartsWith(str.m_pString, str.m_nLength); }

		bool EndsWith(const void* str, size_t len) const
		{
			if (len > m_nLength)
				return false;

			return memcmp(m_pString + m_nLength - len, str, len) == 0;
		}

		bool EndsWith(const char* str) const { return EndsWith(str, strlen(str)); }
		bool EndsWith(const String& str) const { return EndsWith(str.m_pString, str.m_nLength); }

		void PadLeft(size_t count, char ch = ' ') // right-aligns characters
		{
			if (m_nLength >= count)
				return;

			EnsureCapacity(count + 1);

			memmove(m_pString + count - m_nLength, m_pString, m_nLength);
			memset(m_pString, ch, count - m_nLength);
			m_nLength = count;
			m_pString[m_nLength] = 0;
		}

		void PadRight(size_t count, char ch = ' ') // left-aligns characters
		{
			if (m_nLength >= count)
				return;

			EnsureCapacity(count + 1);

			memset(m_pString + m_nLength, ch, count - m_nLength);
			m_nLength = count;
			m_pString[m_nLength] = 0;
		}

		void Trim()
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

		void MakeLower()
		{
			_strlwr(m_pString);
		}

		void MakeUpper()
		{
			_strupr(m_pString);
		}

		void EnsureCapacity(size_t nCapacity)
		{
			if (nCapacity <= m_nCapacity)
				return;

#ifdef NL_STRING_EXPONENTIAL_CAPACITY
			if constexpr ((int)Flags & (int)string::Flags::ExponentialCapacity)
			{
				size_t newCapacity = m_nCapacity;
				while (newCapacity < nCapacity)
					newCapacity *= 2;

				m_nCapacity = newCapacity;
			}
			else
#endif
				m_nCapacity = nCapacity;

			if (m_pString == m_stack)
			{
				m_pString = reinterpret_cast<char*>(nl::memory::Allocate(m_nCapacity));
				nl_assert(m_pString != NULL);
				memcpy(m_pString, m_stack, GetLength() + 1);
			}
			else
			{
				m_pString = reinterpret_cast<char*>(nl::memory::Reallocate(m_pString, m_nCapacity));
				nl_assert(m_pString != NULL);
			}
		}

		void Set(const void* str, size_t len)
		{
			EnsureCapacity(len + 1);

			memcpy(m_pString, str, len);
			m_pString[len] = 0;
			m_nLength = len;
		}

#define AppendTemplateValue_Impl_For(type) \
    template <std::enable_if_t<std::is_same_v<type, type>>* = nullptr> \
    static void AppendTemplateValue(String& str, type value)

		AppendTemplateValue_Impl_For(const char*) { str.Append(value); }
		AppendTemplateValue_Impl_For(char) { str.Append(value); }
		AppendTemplateValue_Impl_For(unsigned char) { str.Append(NumberToStringUnsigned(value)); }
		AppendTemplateValue_Impl_For(short) { str.Append(NumberToString(value)); }
		AppendTemplateValue_Impl_For(unsigned short) { str.Append(NumberToStringUnsigned(value)); }
		AppendTemplateValue_Impl_For(int) { str.Append(NumberToString(value)); }
		AppendTemplateValue_Impl_For(unsigned int) { str.Append(NumberToStringUnsigned(value)); }
		AppendTemplateValue_Impl_For(long) { str.Append(NumberToString(value)); }
		AppendTemplateValue_Impl_For(unsigned long) { str.Append(NumberToStringUnsigned(value)); }
		AppendTemplateValue_Impl_For(long long) { str.Append(NumberToString(value)); }
		AppendTemplateValue_Impl_For(unsigned long long) { str.Append(NumberToStringUnsigned(value)); }

		AppendTemplateValue_Impl_For(float)
		{
			char buf[32];
			int len = snprintf(buf, 32, "%f", value);
			str.Append(buf, len);
		}

		AppendTemplateValue_Impl_For(double)
		{
			char buf[64];
			int len = snprintf(buf, 64, "%lf", value);
			str.Append(buf, len);
		}

		AppendTemplateValue_Impl_For(const void*)
		{
			char buf[64];
			int len = snprintf(buf, 64, "0x%p", value);
			str.Append(buf, len);
		}

#ifdef NL_PLATFORM_WINDOWS
		AppendTemplateValue_Impl_For(const wchar_t*)
		{
			size_t required_len = (size_t)WideCharToMultiByte(CP_UTF8, 0, value, -1, nullptr, 0, nullptr, nullptr);
			str.EnsureCapacity(str.m_nLength + required_len);
			WideCharToMultiByte(CP_UTF8, 0, value, -1, str.m_pString + str.m_nLength, (int)required_len, nullptr, nullptr);
			str.m_nLength += required_len - 1;
			str.m_pString[str.m_nLength] = 0;
		}
#endif

		static void AppendTemplateValue(String& str, const String& value)
		{
			str.Append(value);
		}

#undef AppendTemplateValue_Impl_For

		static String NumberToString(long long value)
		{
			char buf[64] = {};
			char* p_end = buf + sizeof(buf) - 1;
			char* p = p_end;

			bool is_negative = value < 0;
			if (is_negative)
				value = -value;

			while (value != 0)
			{
				*--p = '0' + (value % 10);
				value /= 10;
			}


			if (p == p_end)
				*--p = '0';

			if (is_negative)
				*--p = '-';

			return String(p, size_t(p_end - p));
		}

		static String NumberToStringUnsigned(unsigned long long value)
		{
			char buf[64] = {};
			char* p_end = buf + sizeof(buf) - 1;
			char* p = p_end;

			while (value != 0)
			{
				*--p = '0' + (value % 10);
				value /= 10;
			}

			if (p == p_end)
				*--p = '0';

			return String(p, size_t(p_end - p));
		}

		// this handles when Args... gets empty, as an overload for when the "T value" is not supplied
		static void Test(String& output, const char* format, const char* format_end) // TODO: rename Test into something that makes more sense
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

		template <typename T, typename... Args>
		static void Test(String& output, const char* format, const char* format_end, const T& value, const Args&... args)
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

					AppendTemplateValue(output, value);
					++format;

					Test(output, format, format_end, args...);
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
		static String Format(const char* format, const Args&... args)
		{
			String output;
			const char* format_end = format + strlen(format);

			Test(output, format, format_end, args...);

			return output;
		}

#ifdef NL_PLATFORM_WINDOWS
		static String FromHResult(int32_t hr, va_list* l = NULL);
#endif

	private:
		size_t m_nCapacity;
		size_t m_nLength;
		char* m_pString;
		char m_stack[StackSize];
	};
}

namespace std
{
	template<>
	struct hash<nl::String>
	{
		size_t operator()(const nl::String& str) const
		{
			return std::hash<std::string_view>()(str);
		}
	};
}
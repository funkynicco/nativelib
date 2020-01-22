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

// TODO: add more std::string_view compatibility things

namespace nl
{
	class String
	{
	public:
		static constexpr size_t StackSize = 32; // max length: abcdefghijklmnopqrstuvwxyzabcde

		static const size_t npos = (size_t)-1;

		String();
		~String();
		String(const char* str);
		String(const void* str, size_t len);
		String(size_t len, char ch);
		String(std::string_view str);
		String(const String& str);
		String(String&& str);

		String& operator =(const char* str);
		String& operator =(const String& str);
		String& operator =(String&& str);

		char& operator [](size_t index);
		char operator [](size_t index) const;

		char* data();
		const char* c_str() const;
		operator const char* () const;
		operator std::string_view() const;

		size_t GetLength() const;
		size_t GetCapacity() const;

		String operator +(const String& str) const;
		String operator +(const char* str) const;

		String& operator +=(const String& str);
		String& operator +=(const char* str);

		bool operator ==(const char* str) const;
		bool operator ==(const String& str) const;

		void Clear();

		char* GetBuffer();
		const char* GetBuffer() const;

		void SetLength(size_t length);

		void Append(const void* str, size_t len);
		void Append(char c);
		void Append(const char* str);
		void Append(const String& str);

		String Append(const void* str, size_t len) const;
		String Append(const char* str) const;
		String Append(const String& str) const;

		void Delete(size_t index, size_t count);
		void Delete(size_t count);

		void Insert(size_t index, const void* str, size_t len);
		void Insert(size_t index, const char* str);
		void Insert(size_t index, const String& str);

		size_t IndexOf(const void* str, size_t len, size_t start) const;
		size_t IndexOf(const char* str, size_t start = 0) const;
		size_t IndexOf(const String& str, size_t start = 0) const;
		size_t IndexOf(char c, size_t start = 0) const;

		String Substring(size_t index, size_t count = npos) const;

		bool StartsWith(const void* str, size_t len) const;
		bool StartsWith(const char* str) const;
		bool StartsWith(const String& str) const;

		bool EndsWith(const void* str, size_t len) const;
		bool EndsWith(const char* str) const;
		bool EndsWith(const String& str) const;

		void PadLeft(size_t count, char ch = ' '); // right-aligns characters
		void PadRight(size_t count, char ch = ' '); // left-aligns characters
		void Trim();
		void MakeLower();
		void MakeUpper();
		void EnsureCapacity(size_t nCapacity);
		void Set(const void* str, size_t len);

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

		static String NumberToString(long long value);
		static String NumberToStringUnsigned(unsigned long long value);
		
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
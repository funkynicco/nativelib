#include "StdAfx.h"

#include <NativeLib/String.h>

#ifdef NL_PLATFORM_WINDOWS
#include <Windows.h>
#endif

namespace nl
{
	String::String() :
		m_nCapacity(sizeof(m_stack)),
		m_nLength(0),
		m_pString(m_stack)
	{
		*m_pString = 0;
	}

	String::~String()
	{
		if (m_pString != m_stack)
			nl::memory::Free(m_pString);
	}

	String::String(const char* str) :
		m_nCapacity(sizeof(m_stack)),
		m_nLength(0),
		m_pString(m_stack)
	{
		size_t len = strlen(str);
		Set(str, len);
	}

	String::String(const void* str, size_t len) :
		m_nCapacity(sizeof(m_stack)),
		m_nLength(0),
		m_pString(m_stack)
	{
		Set(str, len);
	}

	String::String(size_t len, char ch) :
		m_nCapacity(sizeof(m_stack)),
		m_nLength(0),
		m_pString(m_stack)
	{
		EnsureCapacity(len + 1);
		memset(m_pString, ch, len);
		m_nLength = len;
		m_pString[len] = 0;
	}

	String::String(std::string_view str) :
		m_nCapacity(sizeof(m_stack)),
		m_nLength(0),
		m_pString(m_stack)
	{
		Set(str.data(), str.length());
	}

	String::String(const String& str) :
		m_nCapacity(sizeof(m_stack)),
		m_nLength(0),
		m_pString(m_stack)
	{
		Set(str.m_pString, str.m_nLength);
	}

	String::String(String&& str) :
		m_nCapacity(sizeof(m_stack)),
		m_nLength(0),
		m_pString(m_stack)
	{
		if (str.m_nLength + 1 > StackSize&&
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

	String& String::operator =(const char* str)
	{
		size_t len = strlen(str);
		Set(str, len);
		return *this;
	}

	String& String::operator =(const String& str)
	{
		Set(str.m_pString, str.m_nLength);
		return *this;
	}

	String& String::operator =(String&& str)
	{
		if (str.m_nLength + 1 > StackSize&&
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

	char& String::operator [](size_t index)
	{
		nl_assert_if_debug(index < m_nLength);
		return m_pString[index];
	}

	char String::operator [](size_t index) const
	{
		nl_assert_if_debug(index < m_nLength);
		return m_pString[index];
	}

	char* String::data()
	{
		return m_pString;
	}

	const char* String::c_str() const
	{
		return m_pString;
	}

	String::operator const char* () const
	{
		return m_pString;
	}

	String::operator std::string_view() const
	{
		return std::string_view(m_pString, m_nLength);
	}

	size_t String::GetLength() const
	{
		return m_nLength;
	}

	size_t String::GetCapacity() const
	{
		return m_nCapacity;
	}

	String String::operator +(const String& str) const
	{
		String result = *this;
		result.Append(str.m_pString, str.m_nLength);
		return result;
	}

	String String::operator +(const char* str) const
	{
		String result = *this;
		result.Append(str, strlen(str));
		return result;
	}

	String& String::operator +=(const String& str)
	{
		Append(str.m_pString, str.m_nLength);
		return *this;
	}

	String& String::operator +=(const char* str)
	{
		Append(str, strlen(str));
		return *this;
	}

	bool String::operator ==(const char* str) const
	{
		return strcmp(m_pString, str) == 0;
	}

	bool String::operator ==(const String& str) const
	{
		return strcmp(m_pString, str.m_pString) == 0;
	}

	void String::Clear()
	{
		m_nLength = 0;
		*m_pString = 0;
	}

	char* String::GetBuffer()
	{
		return m_pString;
	}

	const char* String::GetBuffer() const
	{
		return m_pString;
	}

	void String::SetLength(size_t length)
	{
		EnsureCapacity(length + 1);
		m_nLength = length;
		m_pString[length] = 0;
	}

	void String::Append(const void* str, size_t len)
	{
		EnsureCapacity(m_nLength + len + 1);
		memcpy(m_pString + m_nLength, str, len);
		m_nLength += len;
		m_pString[m_nLength] = 0;
	}

	void String::Append(char c)
	{
		Append(&c, 1);
	}

	void String::Append(const char* str)
	{
		Append(str, strlen(str));
	}

	void String::Append(const String& str)
	{
		Append(str.m_pString, str.m_nLength);
	}

	String String::Append(const void* str, size_t len) const
	{
		String result = *this;
		result.Append(str, len);
		return result;
	}

	String String::Append(const char* str) const
	{
		return Append(str, strlen(str));
	}

	String String::Append(const String& str) const
	{
		return Append(str.m_pString, str.m_nLength);
	}

	void String::Delete(size_t index, size_t count)
	{
		nl_assert(index + count < m_nLength);
		memmove(m_pString + index, m_pString + index + count, m_nLength - (index + count));
		m_nLength -= count;
		m_pString[m_nLength] = 0;
	}

	void String::Delete(size_t count)
	{
		Delete(0, count);
	}

	void String::Insert(size_t index, const void* str, size_t len)
	{
		EnsureCapacity(m_nLength + len + 1);
		memmove(m_pString + index + len, m_pString + index, m_nLength - index);
		memcpy(m_pString + index, str, len);
		m_nLength += len;
		m_pString[m_nLength] = 0;
	}

	void String::Insert(size_t index, const char* str)
	{
		Insert(index, str, strlen(str));
	}

	void String::Insert(size_t index, const String& str)
	{
		Insert(index, str.m_pString, str.m_nLength);
	}

	size_t String::IndexOf(const void* str, size_t len, size_t start) const
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

	size_t String::IndexOf(const char* str, size_t start) const
	{
		return IndexOf(str, strlen(str), start);
	}

	size_t String::IndexOf(const String& str, size_t start) const
	{
		return IndexOf(str.m_pString, str.m_nLength, start);
	}

	size_t String::IndexOf(char c, size_t start) const
	{
		return IndexOf(&c, 1, start);
	}

	String String::Substring(size_t index, size_t count) const
	{
		if (count == npos)
			count = m_nLength - index;

		nl_assert_if_debug(index + count <= m_nLength);
		return String(m_pString + index, count);
	}

	bool String::StartsWith(const void* str, size_t len) const
	{
		if (len > m_nLength)
			return false;

		return memcmp(m_pString, str, len) == 0;
	}

	bool String::StartsWith(const char* str) const
	{
		return StartsWith(str, strlen(str));
	}

	bool String::StartsWith(const String& str) const
	{
		return StartsWith(str.m_pString, str.m_nLength);
	}

	bool String::EndsWith(const void* str, size_t len) const
	{
		if (len > m_nLength)
			return false;

		return memcmp(m_pString + m_nLength - len, str, len) == 0;
	}

	bool String::EndsWith(const char* str) const
	{
		return EndsWith(str, strlen(str));
	}

	bool String::EndsWith(const String& str) const
	{
		return EndsWith(str.m_pString, str.m_nLength);
	}

	void String::PadLeft(size_t count, char ch)
	{
		if (m_nLength >= count)
			return;

		EnsureCapacity(count + 1);

		memmove(m_pString + count - m_nLength, m_pString, m_nLength);
		memset(m_pString, ch, count - m_nLength);
		m_nLength = count;
		m_pString[m_nLength] = 0;
	}

	void String::PadRight(size_t count, char ch)
	{
		if (m_nLength >= count)
			return;

		EnsureCapacity(count + 1);

		memset(m_pString + m_nLength, ch, count - m_nLength);
		m_nLength = count;
		m_pString[m_nLength] = 0;
	}

	void String::Trim()
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

	void String::MakeLower()
	{
		_strlwr(m_pString);
	}

	void String::MakeUpper()
	{
		_strupr(m_pString);
	}

	void String::EnsureCapacity(size_t nCapacity)
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

	void String::Set(const void* str, size_t len)
	{
		EnsureCapacity(len + 1);

		memcpy(m_pString, str, len);
		m_pString[len] = 0;
		m_nLength = len;
	}

	// static

	String String::NumberToString(long long value)
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

	String String::NumberToStringUnsigned(unsigned long long value)
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

#ifdef NL_PLATFORM_WINDOWS
	String String::FromHResult(int32_t hr, va_list* l)
	{
		LPSTR pstr = NULL;
		DWORD dwSize = FormatMessageA(
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
			String str(pstr, dwSize - 2);
			LocalFree(pstr);
			return str;
		}

		String str(pstr, dwSize);
		LocalFree(pstr);
		return str;
	}
#endif
}
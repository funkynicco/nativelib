#include "StdAfx.h"

#include <NativeLib/String.h>

#ifdef NL_PLATFORM_WINDOWS
#include <Windows.h>
#endif

namespace nl
{
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
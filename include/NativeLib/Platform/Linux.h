#ifndef __NL_PLATFORM_H_SCOPE
#error Cannot directly include NativeLib/Platform/Linux.h - Use NativeLib/Platform/Platform.h instead.
#endif

#pragma once

#include <errno.h>
#include <string.h>
#include <ctype.h>

//#ifdef __EMSCRIPTEN__
//#endif

#define NL_NOINLINE __attribute__ ((noinline))

#define __analysis_assume(expr) expr

#define _strlwr strlwr
#define _strupr strupr
#define sprintf_s sprintf

typedef unsigned long DWORD; // TODO: remove ...
typedef int errno_t;

inline errno_t strcpy_s(char* dst, size_t size, const char* src)
{
    const char* end = dst + size;
    const size_t len = strlen(src);

    if (len + 1 > size)
    {
        if (size != 0)
            *dst = 0;

        return 1;
    }

    memcpy(dst, src, len + 1);
    return 0;
}

template <size_t _Size>
inline errno_t strcpy_s(char(&dst)[_Size], const char* src)
{
    return strcpy_s(dst, _Size, src);
}

inline char* strlwr(char* s)
{
    char* p = s;
    while (*p)
    {
        *p = tolower((unsigned char)*p);
        p++;
    }

    return s;
}

inline char* strupr(char* s)
{
    char* p = s;
    while (*p)
    {
        *p = toupper((unsigned char)*p);
        p++;
    }

    return s;
}

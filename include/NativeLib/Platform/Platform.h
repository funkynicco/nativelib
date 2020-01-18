#pragma once

#include <stdint.h>

#if defined(NL_ARCHITECTURE_X64)
static_assert(sizeof(void*) == 8, "Platform expected to be x64 but pointers are 32bit!");
#elif defined(NL_ARCHITECTURE_X86)
static_assert(sizeof(void*) == 4, "Platform expected to be x86 but pointers are 64bit!");
#else
#error No platform architecture defined!
#endif

#ifdef _WIN32
#define NL_PLATFORM_WINDOWS
#else
#define NL_PLATFORM_LINUX
#endif

#define __NL_PLATFORM_H_SCOPE

#ifdef NL_PLATFORM_WINDOWS
#include <NativeLib/Platform/Windows.h>
#endif

#ifdef NL_PLATFORM_LINUX
#include <NativeLib/Platform/Linux.h>
#endif

#define NL_PLATFORM_CHECK_TYPESIZE(type, expected) static_assert(sizeof(type) == expected, "[Platform checks] Fatal: sizeof(" #type ") is not " #expected)

NL_PLATFORM_CHECK_TYPESIZE(char, 1);
NL_PLATFORM_CHECK_TYPESIZE(unsigned char, 1);

NL_PLATFORM_CHECK_TYPESIZE(short, 2);
NL_PLATFORM_CHECK_TYPESIZE(unsigned short, 2);

NL_PLATFORM_CHECK_TYPESIZE(int, 4);
NL_PLATFORM_CHECK_TYPESIZE(unsigned int, 4);

NL_PLATFORM_CHECK_TYPESIZE(long, 4);
NL_PLATFORM_CHECK_TYPESIZE(unsigned long, 4);

NL_PLATFORM_CHECK_TYPESIZE(long long, 8);
NL_PLATFORM_CHECK_TYPESIZE(unsigned long long, 8);

#undef NL_PLATFORM_CHECK_TYPESIZE

#undef __NL_PLATFORM_H_SCOPE
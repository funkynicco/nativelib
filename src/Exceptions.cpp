#include "StdAfx.h"

#include <NativeLib/Exceptions.h>
#include <NativeLib/String.h>

//!ALLOW_INCLUDE "Windows.h"
//!ALLOW_INCLUDE "DbgHelp.h"

#ifdef NL_PLATFORM_WINDOWS
#include <Windows.h>
#include <DbgHelp.h>

typedef USHORT(WINAPI* CaptureStackBackTraceType)(ULONG FramesToSkip, ULONG FramesToCapture, PVOID* BackTrace, PULONG BackTraceHash);

static SRWLOCK g_lock = SRWLOCK_INIT;
static bool g_initializedSymbol = false;
static HANDLE g_hProcess = nullptr;
static HMODULE g_hKernel = nullptr;
static CaptureStackBackTraceType g_captureStackBackTrace = nullptr;

static bool InitializeCallstackCapturing()
{
    g_hProcess = GetCurrentProcess();

    if (!SymInitialize(g_hProcess, nullptr, TRUE))
        return false;

    g_hKernel = LoadLibraryW(L"kernel32.dll");
    if (!g_hKernel)
        return false;

    g_captureStackBackTrace = reinterpret_cast<CaptureStackBackTraceType>(GetProcAddress(g_hKernel, "RtlCaptureStackBackTrace"));
    if (!g_captureStackBackTrace)
        return false;

    g_initializedSymbol = true;
    return true;
}
#endif

Exception::Exception(const char* message) noexcept
{
    strcpy_s(m_message, message);

#ifdef NL_PLATFORM_WINDOWS
    *m_callstack = 0;
    CaptureCallstack();
#endif
}

#ifdef NL_PLATFORM_WINDOWS
NL_NOINLINE void Exception::CaptureCallstack() noexcept
{
    AcquireSRWLockExclusive(&g_lock);
    if (!g_initializedSymbol &&
        !InitializeCallstackCapturing())
    {
        ReleaseSRWLockExclusive(&g_lock);
        return;
    }

    USHORT frames;
    LPVOID stack[100] = {};
    if ((frames = g_captureStackBackTrace(2, 100, stack, nullptr)) == 0)
    {
        ReleaseSRWLockExclusive(&g_lock);
        return;
    }

    SYMBOL_INFO* symbol = (SYMBOL_INFO*)calloc(sizeof(SYMBOL_INFO) * 256 * sizeof(char), 1);
    ZeroMemory(symbol, sizeof(SYMBOL_INFO) + 256 * sizeof(char));
    symbol->MaxNameLen = 255;
    symbol->SizeOfStruct = sizeof(SYMBOL_INFO);

    nl::String str;

    for (USHORT i = 0; i < frames; ++i)
    {
        SymFromAddr(g_hProcess, reinterpret_cast<DWORD64>(stack[i]), nullptr, symbol);

        if (str.GetLength() != 0)
            str.Append("\n");

        str.Append(nl::String::Format("- [{}] {}", (const void*)symbol->Address, symbol->Name));
    }

    strcpy_s(m_callstack, str.c_str());
    ReleaseSRWLockExclusive(&g_lock);
}

Win32Exception::Win32Exception() :
    Win32Exception(GetLastError())
{
}

Win32Exception::Win32Exception(uint32_t dwCode) :
    Exception(GetWin32ErrorMessage(dwCode))
{
}

const char* Win32Exception::GetWin32ErrorMessage(uint32_t dwCode)
{
    thread_local char message[1024];
    DWORD dwLen = FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, nullptr, dwCode, LANG_NEUTRAL, message, 1024, nullptr);
    while (dwLen > 0 &&
        (message[dwLen - 1] == '\r' || message[dwLen - 1] == '\n'))
        --dwLen;
    message[dwLen] = 0;
    return message;
}
#endif

IOException::IOException(const char* msg) :
    Exception(msg)
{
#ifdef NL_PLATFORM_WINDOWS
    m_dwErrorCode = GetLastError();
#else
    m_dwErrorCode = (DWORD)errno;
#endif
}

SocketException::SocketException(const char* message, uint32_t code) :
    Exception(message)
{
    m_code = code;
}

uint32_t SocketException::GetCode() const
{
    return m_code;
}
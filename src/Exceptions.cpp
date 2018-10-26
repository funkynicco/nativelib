#include "StdAfx.h"
#include <NativeLib/Exceptions.h>

#include <DbgHelp.h>
#pragma comment(lib, "dbghelp.lib")

SRWLOCK Exception::m_lock = SRWLOCK_INIT;
bool Exception::m_initializedSymbol = false;
HANDLE Exception::m_hProcess = nullptr;
HMODULE Exception::m_hKernel = nullptr;
Exception::CaptureStackBackTraceType Exception::m_captureStackBackTrace = nullptr;

Exception::Exception(const wchar_t* message) noexcept
{
    wcscpy_s(m_message, message);
    *m_callstack = 0;

    CaptureCallstack();
}

Exception::Exception(const char* message) noexcept
{
    m_message[MultiByteToWideChar(CP_UTF8, 0, message, -1, m_message, sizeof(m_message) / sizeof(m_message[0]))] = 0;
    *m_callstack = 0;

    CaptureCallstack();
}

bool Exception::InitializeCallstackCapturing() noexcept
{
    m_hProcess = GetCurrentProcess();

    if (!SymInitialize(m_hProcess, nullptr, TRUE))
        return false;

    m_hKernel = LoadLibrary(L"kernel32.dll");
    if (!m_hKernel)
        return false;

    m_captureStackBackTrace = reinterpret_cast<CaptureStackBackTraceType>(GetProcAddress(m_hKernel, "RtlCaptureStackBackTrace"));
    if (!m_captureStackBackTrace)
        return false;

    m_initializedSymbol = true;
    return true;
}

DECLSPEC_NOINLINE void Exception::CaptureCallstack() noexcept
{
    AcquireSRWLockExclusive(&m_lock);
    if (!m_initializedSymbol &&
        !InitializeCallstackCapturing())
    {
        ReleaseSRWLockExclusive(&m_lock);
        return;
    }

    USHORT frames;
    LPVOID stack[100] = {};
    if ((frames = m_captureStackBackTrace(2, 100, stack, nullptr)) == 0)
    {
        ReleaseSRWLockExclusive(&m_lock);
        return;
    }

    SYMBOL_INFO* symbol = (SYMBOL_INFO*)calloc(sizeof(SYMBOL_INFO) * 256 * sizeof(char), 1);
    ZeroMemory(symbol, sizeof(SYMBOL_INFO) + 256 * sizeof(char));
    symbol->MaxNameLen = 255;
    symbol->SizeOfStruct = sizeof(SYMBOL_INFO);

    nl::BasicString<1024> str;

    for (USHORT i = 0; i < frames; ++i)
    {
        SymFromAddr(m_hProcess, reinterpret_cast<DWORD64>(stack[i]), nullptr, symbol);

        if (str.GetLength() != 0)
            str.Append("\n");

        str.Append(nl::BasicString<128>::Format("- [{}] {}", (const void*)symbol->Address, symbol->Name));
    }

    m_callstack[MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.GetLength(), m_callstack, sizeof(m_callstack) / sizeof(m_callstack[0]))] = 0;
    ReleaseSRWLockExclusive(&m_lock);
}
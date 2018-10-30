#pragma once

#include <Windows.h>

class Exception
{
public:
    DECLSPEC_NOINLINE Exception(const wchar_t* message) noexcept;
    DECLSPEC_NOINLINE Exception(const char* message) noexcept;
    inline Exception(const Exception& other) noexcept = default;
    inline Exception& operator =(const Exception& other) noexcept = default;

    inline const wchar_t* GetMessage() const noexcept { return m_message; }
    inline const wchar_t* GetCallstack() const noexcept { return m_callstack; }

private:
    DECLSPEC_NOINLINE void CaptureCallstack() noexcept;
    static bool InitializeCallstackCapturing() noexcept;

    wchar_t m_message[1024];
    wchar_t m_callstack[1024];

    typedef USHORT(WINAPI *CaptureStackBackTraceType)(ULONG FramesToSkip, ULONG FramesToCapture, PVOID* BackTrace, PULONG BackTraceHash);

    static SRWLOCK m_lock;
    static bool m_initializedSymbol;
    static HANDLE m_hProcess;
    static HMODULE m_hKernel;
    static CaptureStackBackTraceType m_captureStackBackTrace;
};

#define DeclareGenericException(name, msg) class name : public Exception {public: inline name():Exception(msg){}};
#define DeclarePassthroughException(name) class name : public Exception {public: inline name(const wchar_t* msg):Exception(msg){}};

DeclareGenericException(AbortApplicationException, L"Abort application");
DeclareGenericException(BadAllocationException, L"Bad allocation");
DeclareGenericException(NotImplementedException, L"Not implemented");
DeclarePassthroughException(ArgumentException);
DeclarePassthroughException(InvalidOperationException);

class Win32Exception : public Exception
{
public:
    inline Win32Exception(DWORD dwCode) :
        Exception(GetWin32ErrorMessage(dwCode))
    {
    }

    inline Win32Exception() :
        Win32Exception(GetLastError())
    {
    }

private:
    inline static const wchar_t* GetWin32ErrorMessage(DWORD dwCode)
    {
        thread_local wchar_t message[1024];
        DWORD dwLen = FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM, nullptr, dwCode, LANG_NEUTRAL, message, 1024, nullptr);
        while (dwLen > 0 &&
            (message[dwLen - 1] == L'\r' || message[dwLen - 1] == L'\n'))
            --dwLen;
        message[dwLen] = 0;
        return message;
    }
};

class IOException : public Exception
{
public:
    static constexpr const wchar_t* ReadFailed = L"Read failed";
    static constexpr const wchar_t* WriteFailed = L"Write failed";
    static constexpr const wchar_t* SeekFailed = L"Seek failed";

    inline IOException(const wchar_t* msg) :
        Exception(msg)
    {
        m_dwErrorCode = GetLastError();
    }

private:
    DWORD m_dwErrorCode;
};
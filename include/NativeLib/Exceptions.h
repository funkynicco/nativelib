#pragma once

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


DeclareGenericException(AbortApplicationException, L"Abort application");
DeclareGenericException(BadAllocationException, L"Bad allocation");

#undef DeclareGenericException
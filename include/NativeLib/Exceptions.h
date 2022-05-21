#pragma once

#include <NativeLib/Platform/Platform.h>

class Exception
{
public:
    NL_NOINLINE Exception(const char* message) noexcept;
    Exception(const Exception& other) noexcept = default;
    Exception& operator =(const Exception& other) noexcept = default;

    const char* GetMessage() const noexcept { return m_message; }

#ifdef NL_PLATFORM_WINDOWS
    const char* GetCallstack() const noexcept { return m_callstack; }
#endif

private:
#ifdef NL_PLATFORM_WINDOWS
    NL_NOINLINE void CaptureCallstack() noexcept;
    char m_callstack[1024];
#endif

    char m_message[1024];
};

#define DeclareGenericException(name, msg) class name : public Exception {public: name():Exception(msg){}};
#define DeclarePassthroughException(name) class name : public Exception {public: name(const char* msg):Exception(msg){}};

DeclareGenericException(AbortApplicationException, "Abort application");
DeclareGenericException(BadAllocationException, "Bad allocation");
DeclareGenericException(NotImplementedException, "Not implemented");
DeclarePassthroughException(ArgumentException);
DeclarePassthroughException(InvalidOperationException);
DeclarePassthroughException(NotSupportedException);

#ifdef NL_PLATFORM_WINDOWS
class Win32Exception : public Exception
{
public:
    Win32Exception();

    Win32Exception(uint32_t dwCode);

private:
    static const char* GetWin32ErrorMessage(uint32_t dwCode);
};
#endif

class IOException : public Exception
{
public:
    static constexpr const char* ReadFailed = "Read failed";
    static constexpr const char* WriteFailed = "Write failed";
    static constexpr const char* SeekFailed = "Seek failed";

    IOException(const char* msg);

private:
    uint32_t m_dwErrorCode;
};

class SocketException : public Exception
{
public:
    SocketException(const char* message, uint32_t code = 0);

    uint32_t GetCode() const;

private:
    uint32_t m_code;
};
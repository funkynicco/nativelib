#pragma once

namespace nl
{
    namespace assert
    {
        struct Assert
        {
            const char* Expression;
            const char* Filename;
            int Line;
            const char* Function;
        };

        typedef void(*pfnAssertHandler)(const Assert& assert);

        void CallAssertHandler(const char* expression, const char* filename, int line, const char* function);
        void DebugBreak();
    }
}

#define nl_assert(__expr) \
    if (!(__expr)) \
    { \
        nl::assert::CallAssertHandler(#__expr, __FILE__, __LINE__, __FUNCTION__); \
        nl::assert::DebugBreak(); \
    } \
    __analysis_assume(__expr)

#if _DEBUG
#define nl_assert_if_debug(__expr) nl_assert(__expr)
#else
#define nl_assert_if_debug(__expr) __expr
#endif
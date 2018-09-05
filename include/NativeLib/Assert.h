#pragma once

namespace nl
{
    struct Assert
    {
        const char* Expression;
        const char* Filename;
        int Line;
        const char* Function;
    };

    typedef void(*pfnAssertHandler)(const Assert& assert);

    extern pfnAssertHandler g_assertHandler;

    void SetAssertHandler(pfnAssertHandler assertHandler);
    void CallAssertHandler(const char* expression, const char* filename, int line, const char* function);
}

#define nl_assert(__expr) if (!(__expr)) nl::CallAssertHandler(#__expr, __FILE__, __LINE__, __FUNCTION__);

#if _DEBUG
#define nl_assert_if_debug(__expr) nl_assert(__expr)
#else
#define nl_assert_if_debug(__expr)
#endif
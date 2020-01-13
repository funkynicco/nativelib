#include "StdAfx.h"

#include <NativeLib/Assert.h>
#include <NativeLib/SystemLayer/SystemLayer.h>

#ifdef NL_PLATFORM_WINDOWS
#include <Windows.h>
#endif

#include <csignal>

namespace nl::assert
{
    void CallAssertHandler(const char* expression, const char* filename, int line, const char* function)
    {
        Assert assert = {};
        assert.Expression = expression;
        assert.Filename = filename;
        assert.Line = line;
        assert.Function = function;

        nl::systemlayer::GetSystemLayerFunctions()->AssertHandler(assert);
    }

    void DebugBreak()
    {
#ifdef NL_PLATFORM_WINDOWS
        ::DebugBreak();
#endif

#ifdef NL_PLATFORM_LINUX
        raise(SIGTRAP);
#endif
    }
}
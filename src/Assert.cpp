#include "StdAfx.h"

#include <NativeLib/Assert.h>

nl::pfnAssertHandler g_assertHandler = nullptr;

void nl::SetAssertHandler(nl::pfnAssertHandler assertHandler)
{
    g_assertHandler = assertHandler;
}

void nl::CallAssertHandler(const char* expression, const char* filename, int line, const char* function)
{
    Assert assert = {};
    assert.Expression = expression;
    assert.Filename = filename;
    assert.Line = line;
    assert.Function = function;

    if (g_assertHandler)
        g_assertHandler(assert);
}
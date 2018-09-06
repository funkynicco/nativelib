#include "StdAfx.h"

#include <NativeLib/Assert.h>

nl::assert::pfnAssertHandler& GetAssertHandlerPointer()
{
    static nl::assert::pfnAssertHandler ptr = nullptr;
    return ptr;
}

void nl::assert::SetAssertHandler(nl::assert::pfnAssertHandler assertHandler)
{
    GetAssertHandlerPointer() = assertHandler;
}

void nl::assert::CallAssertHandler(const char* expression, const char* filename, int line, const char* function)
{
    Assert assert = {};
    assert.Expression = expression;
    assert.Filename = filename;
    assert.Line = line;
    assert.Function = function;

    if (GetAssertHandlerPointer())
        GetAssertHandlerPointer()(assert);
}
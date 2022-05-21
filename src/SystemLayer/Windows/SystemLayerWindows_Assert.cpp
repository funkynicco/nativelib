#include "StdAfx.h"

#ifdef NL_PLATFORM_WINDOWS

#include <NativeLib/SystemLayer/SystemLayer.h>

//!ALLOW_INCLUDE "SystemLayerWindows.h"
#include "SystemLayerWindows.h"

#include <NativeLib/Assert.h>
#include <NativeLib/Exceptions.h>

namespace nl::systemlayer::defaults
{
    static void WindowsAssert(const nl::assert::Assert& assert)
    {
        throw NotImplementedException();
    }

    bool SetAssert(SystemLayerFunctions* functions)
    {
        functions->AssertHandler = WindowsAssert;
        return true;
    }
}

#endif
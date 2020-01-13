#include "StdAfx.h"

#ifdef NL_PLATFORM_LINUX

#include <NativeLib/SystemLayer/SystemLayer.h>

#include "SystemLayerLinux.h"

#include <NativeLib/Assert.h>
#include <NativeLib/Exceptions.h>

namespace nl::systemlayer::defaults
{
    static void LinuxAssert(const nl::assert::Assert& assert)
    {
        throw NotImplementedException();
    }

    bool SetAssert(SystemLayerFunctions* functions)
    {
        functions->AssertHandler = LinuxAssert;
        return true;
    }
}

#endif
#include "StdAfx.h"

#include <NativeLib/SystemLayer/SystemLayer.h>

//!ALLOW_INCLUDE "SystemLayer/Windows/SystemLayerWindows.h"
//!ALLOW_INCLUDE "SystemLayer/Linux/SystemLayerLinux.h"

#if defined(NL_PLATFORM_WINDOWS)
#include "SystemLayer/Windows/SystemLayerWindows.h"
#elif defined(NL_PLATFORM_LINUX)
#include "SystemLayer/Linux/SystemLayerLinux.h"
#endif

namespace nl::systemlayer
{
    static SystemLayerFunctions s_functions = {};

    const SystemLayerFunctions* GetSystemLayerFunctions()
    {
        return &s_functions;
    }

    void SetSystemLayerFunctions(const SystemLayerFunctions* functions)
    {
        // TODO: crash if any of the functions are not set...

        memcpy(&s_functions, functions, sizeof(SystemLayerFunctions));
    }

    bool GetDefaultSystemLayerFunctions(SystemLayerFunctions* functions)
    {
        if (!nl::systemlayer::defaults::SetAssert(functions))
        {
            return false;
        }

        if (!nl::systemlayer::defaults::SetMemory(functions))
        {
            return false;
        }

        if (!nl::systemlayer::defaults::SetFileIO(functions))
        {
            return false;
        }

        return true;
    }

    bool SetDefaultSystemLayerFunctions()
    {
        SystemLayerFunctions functions;
        if (!GetDefaultSystemLayerFunctions(&functions))
        {
            return false;
        }

        SetSystemLayerFunctions(&functions);
        return true;
    }
}
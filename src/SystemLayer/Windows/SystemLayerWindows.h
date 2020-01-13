#pragma once

#ifdef NL_PLATFORM_WINDOWS

#include <NativeLib/SystemLayer/SystemLayer.h>

namespace nl::systemlayer::defaults
{
    bool SetAssert(SystemLayerFunctions* functions);
    bool SetFileIO(SystemLayerFunctions* functions);
    bool SetMemory(SystemLayerFunctions* functions);
}

#endif
#include "StdAfx.h"

#include <NativeLib/File.h>
#include <NativeLib/SystemLayer/SystemLayer.h>

namespace nl::file
{
    bool Exists(std::string_view path)
    {
        return nl::systemlayer::GetSystemLayerFunctions()->FileOrDirectoryExists(nl::String(path));
    }
}
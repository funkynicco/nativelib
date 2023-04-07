#pragma once

#include <string_view>

namespace nl::file
{
    // Checks whether a file or directory exists
    bool Exists(std::string_view path);
}
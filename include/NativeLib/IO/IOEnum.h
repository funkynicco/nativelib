#pragma once

namespace nl::io
{
    enum class CreateMode
    {
        CreateNew,
        CreateAlways,
        OpenExisting,
        OpenAlways,
        TruncateExisting,
    };

    enum class SeekMode
    {
        Begin,
        Current,
        End
    };
}
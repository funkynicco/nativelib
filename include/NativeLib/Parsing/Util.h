#pragma once

namespace nl
{
    namespace parsing
    {
        enum class TokenType
        {
            Error = -2,
            EndOfFile = -1,
            Keyword,
            Delimiter,
            String,
            Number,
            Hex,
            Float
        };
    }
}
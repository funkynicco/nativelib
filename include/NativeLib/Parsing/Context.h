#pragma once

#include <NativeLib/Parsing/Util.h>
#include <NativeLib/RAII/Shared.h>
#include <NativeLib/String.h>

namespace nl
{
    namespace parsing
    {
        struct Context
        {
            /*** constant variables ***/
            nl::Shared<nl::String> const lpContainer;

            const char* const DataBegin;
            const char* const DataEnd;

            /*** these variables changes ***/

            const char* ViewBegin;
            const char* ViewEnd;

            const char* TokenBegin;
            const char* TokenEnd;
            TokenType TokenType;

            int32_t Line;

            // use only when a token needs to change value, ie \n in a string
            nl::String TempToken;

            Context(const char* dataBegin, const char* dataEnd) noexcept;
            Context(nl::Shared<nl::String> container) noexcept;
            Context(const Context& context);
            
            ~Context();

            Context(Context&&) noexcept = delete;
            Context& operator =(Context&& context) noexcept = delete;
            Context& operator =(const Context&) = delete;

            void Reset();
            void Empty();
            void SetToken(const char* tokenBegin, const char* tokenEnd, nl::parsing::TokenType tokenType);
            void SetTokenToTemp(nl::parsing::TokenType tokenType);
            
            bool IsEnd() const;
            bool IsTokenSet() const;
            
            std::string_view GetView() const;
            std::string_view GetTokenView() const;
        };
    }
}
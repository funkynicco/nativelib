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

            const char* LastViewBegin;
            const char* ViewBegin;
            const char* ViewEnd;

            int32_t Line;

            Context(const char* dataBegin, const char* dataEnd) noexcept;
            Context(nl::Shared<nl::String> container) noexcept;
            Context(const Context& context);
            
            ~Context();

            Context(Context&&) noexcept = delete;
            Context& operator =(Context&& context) noexcept = delete;
            Context& operator =(const Context&) = delete;

            void Reset();
            void Empty();
            
            bool IsEnd() const;
            
            std::string_view GetView() const;
        };
    }
}
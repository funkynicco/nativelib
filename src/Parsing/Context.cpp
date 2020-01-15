#include "StdAfx.h"

#include <NativeLib/Parsing/Context.h>

namespace nl
{
    namespace parsing
    {
        Context::Context(const char* dataBegin, const char* dataEnd) noexcept :
            DataBegin(dataBegin),
            DataEnd(dataEnd)
        {
            Reset();
        }

        Context::Context(nl::Shared<nl::String> container) noexcept :
            lpContainer(container),
            DataBegin(container->c_str()),
            DataEnd(container->c_str() + container->GetLength())
        {
            Reset();
        }

        Context::Context(const Context& context) :
            lpContainer(context.lpContainer),
            DataBegin(context.DataBegin),
            DataEnd(context.DataEnd)
        {
            Reset();
        }

        Context::~Context()
        {
        }

        void Context::Reset()
        {
            LastViewBegin = ViewBegin = DataBegin;
            ViewEnd = DataEnd;
            Line = 1;
        }

        void Context::Empty()
        {
            LastViewBegin = ViewBegin = ViewEnd = nullptr;
            Line = 1;
        }

        bool Context::IsEnd() const
        {
            return ViewBegin == ViewEnd;
        }

        std::string_view Context::GetView() const
        {
            return std::string_view(ViewBegin, ViewEnd - ViewBegin);
        }
    }
}
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
            ViewBegin = DataBegin;
            ViewEnd = DataEnd;
            TokenBegin = TokenEnd = nullptr;
            TokenType = TokenType::Error;
            Line = 1;
        }

        void Context::Empty()
        {
            ViewBegin = ViewEnd = nullptr;
            TokenBegin = TokenEnd = nullptr;
            TokenType = TokenType::Error;
            Line = 1;
        }

        void Context::SetToken(const char* tokenBegin, const char* tokenEnd, nl::parsing::TokenType tokenType)
        {
            TokenBegin = tokenBegin;
            TokenEnd = tokenEnd;
            TokenType = tokenType;
        }

        void Context::SetTokenToTemp(nl::parsing::TokenType tokenType)
        {
            TokenBegin = TempToken.c_str();
            TokenEnd = TokenBegin + TempToken.GetLength();
            TokenType = tokenType;
        }

        bool Context::IsEnd() const
        {
            return ViewBegin == ViewEnd;
        }

        bool Context::IsTokenSet() const
        {
            return TokenBegin != nullptr;
        }

        std::string_view Context::GetView() const
        {
            return std::string_view(ViewBegin, ViewEnd - ViewBegin);
        }

        std::string_view Context::GetTokenView() const
        {
            return std::string_view(TokenBegin, TokenEnd - TokenBegin);
        }
    }
}
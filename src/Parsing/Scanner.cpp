#include "StdAfx.h"

#include <NativeLib/Parsing/Scanner.h>

#include <NativeLib/Assert.h>
#include <NativeLib/IO/File.h>

// TODO: this parser does not yet support \n \t \" etc in string tokens due to using string_view

namespace nl
{
    namespace parsing
    {
        // 16x16 grid
        constexpr uint8_t HexTable[] =
        {
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //   0
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //  16
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //  32
            0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //  48
            0x00, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //  64
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //  80
            0x00, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //  96
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 112
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 128
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 144
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 160
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 176
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 192
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 208
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 224
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 240
        };

        std::string_view TokenTypeToString(TokenType tokenType)
        {
            switch (tokenType)
            {
            case TokenType::Error: return std::string_view("Error");
            case TokenType::EndOfFile: return std::string_view("End of File");
            case TokenType::Keyword: return std::string_view("Keyword");
            case TokenType::Delimiter: return std::string_view("Delimiter");
            case TokenType::String: return std::string_view("String");
            case TokenType::Number: return std::string_view("Number");
            case TokenType::Hex: return std::string_view("Hex");
            case TokenType::Float: return std::string_view("Float");
            }

            return std::string_view("Unknown token type");
        }

        Scanner::Scanner() noexcept :
            m_endOfFileToken(0, TokenType::EndOfFile),
            m_errorToken(0, TokenType::Error)
        {
        }

        Scanner::Scanner(std::string_view str) :
            m_endOfFileToken(0, TokenType::EndOfFile),
            m_errorToken(0, TokenType::Error)
        {
            m_contextStack.Push(nl::ConstructShared<Context>(str.data(), str.data() + str.length()));
            SaveContext(); // save a duplicate
        }

        Scanner::Scanner(const char* str, size_t length) :
            m_endOfFileToken(0, TokenType::EndOfFile),
            m_errorToken(0, TokenType::Error)
        {
            nl_assert_if_debug(str != nullptr);

            if (length == (size_t)-1)
                length = strlen(str);

            auto container = nl::ConstructShared<nl::String>(str, length);
            m_contextStack.Push(nl::ConstructShared<Context>(container));
            SaveContext(); // save a duplicate
        }

        Scanner::Scanner(const nl::String& str) :
            m_endOfFileToken(0, TokenType::EndOfFile),
            m_errorToken(0, TokenType::Error)
        {
            auto container = nl::ConstructShared<nl::String>(str);
            m_contextStack.Push(nl::ConstructShared<Context>(container));
            SaveContext(); // save a duplicate
        }

        Scanner::Scanner(nl::String&& str) :
            m_endOfFileToken(0, TokenType::EndOfFile),
            m_errorToken(0, TokenType::Error)
        {
            auto container = nl::ConstructShared<nl::String>(std::move(str));
            m_contextStack.Push(nl::ConstructShared<Context>(container));
            SaveContext(); // save a duplicate
        }

        Scanner::Scanner(Scanner&& other) noexcept :
            m_endOfFileToken(0, TokenType::EndOfFile),
            m_errorToken(0, TokenType::Error),
            m_contextStack(std::move(other.m_contextStack))
        {
        }

        Scanner& Scanner::operator =(Scanner&& other) noexcept
        {
            m_contextStack = std::move(other.m_contextStack);
            return *this;
        }

        Scanner::~Scanner()
        {
        }

        bool Scanner::IsWhitespace(char c) const
        {
            switch (c)
            {
            case ' ':
            case '\t':
            case '\r':
            case '\n':
                return true;
            }

            return false;
        }

        bool Scanner::IsKeyword(char c, bool first) const
        {
            if (isalpha(c))
                return true;

            if (first)
                return false;

            switch (c)
            {
            case '_':
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                return true;
            }

            return false;
        }

        bool Scanner::IsHex(char c) const
        {
            if (c >= '0' && c <= '9')
                return true;

            if (c >= 'a' && c <= 'f')
                return true;

            if (c >= 'A' && c <= 'F')
                return true;

            return false;
        }

        bool Scanner::SkipBlank()
        {
            Context* const context = m_contextStack.GetTop();
            const char*& p = context->ViewBegin;
            const char*& end = context->ViewEnd;

            while (
                p < end &&
                IsWhitespace(*p))
                ++p;

            if (p == end)
                return false;

            return true;
        }

        bool Scanner::SkipSingleComment()
        {
            Context* const context = m_contextStack.GetTop();
            const char*& p = context->ViewBegin;
            const char*& end = context->ViewEnd;

            while (p < end)
            {
                if (*p == '\r')
                {
                    if (p + 1 < end &&
                        p[1] == '\n')
                    {
                        p += 2;
                        return true;
                    }

                    ++p;
                    return true;
                }

                if (*p == '\n')
                {
                    ++p;
                    return true;
                }

                ++p;
            }

            return false;
        }

        bool Scanner::SkipMultiComment()
        {
            Context* const context = m_contextStack.GetTop();
            const char*& p = context->ViewBegin;
            const char*& end = context->ViewEnd;

            while (p < end)
            {
                if (p[0] == '*' &&
                    p + 1 < end &&
                    p[1] == '/')
                {
                    p += 2;
                    return true;
                }

                ++p;
            }

            return false;
        }

        Token Scanner::Next()
        {
            Context* const context = m_contextStack.GetTop();
            context->TempToken.Clear();

            auto tokenType = TokenType::Error;

            const char*& p = context->ViewBegin;
            const char*& end = context->ViewEnd;

            for (;;)
            {
                if (!SkipBlank())
                {
                    context->Empty();
                    return m_endOfFileToken;
                }

                if (*p == '/' &&
                    p + 1 < end)
                {
                    if (*(p + 1) == '/')
                    {
                        p += 2;
                        if (!SkipSingleComment())
                        {
                            context->Empty();
                            return m_endOfFileToken;
                        }

                        continue;
                    }

                    if (*(p + 1) == '*')
                    {
                        p += 2;
                        if (!SkipMultiComment())
                        {
                            context->Empty();
                            return m_endOfFileToken;
                        }

                        continue;
                    }
                }

                break;
            }

            const char* token_start = p;

            // calculate lines between previous token point (or start)
            const char* previousTokenStart = context->TokenBegin;
            if (previousTokenStart == nullptr)
                previousTokenStart = context->DataBegin;

            context->Line += CalculateLines(previousTokenStart, token_start);

            // double quoted strings
            if (*p == '"')
            {
                token_start = ++p;
                const char* token_end = token_start;

                while (p < end)
                {
                    if (*p == '\\' &&
                        p + 1 < end)
                    {
                        if (p[1] == '"')
                        {
                            p += 2; // escaped double quote
                            continue;
                        }

                        // TODO: add handlers for \r, \n and \t etc
                    }

                    if (*p == '"')
                    {
                        token_end = p++;
                        break;
                    }

                    token_end = ++p;
                }

                tokenType = TokenType::String;
                if (TransformToken(
                    tokenType,
                    std::string_view(token_start, size_t(token_end - token_start)),
                    context->TempToken))
                {
                    context->SetTokenToTemp(tokenType);
                    return Token(context->Line, tokenType, context->GetTokenView());
                }

                context->SetToken(token_start, token_end, tokenType);
                return Token(context->Line, TokenType::String, context->GetTokenView());
            }

            // single quoted strings
            if (*p == '\'')
            {
                token_start = ++p;
                const char* token_end = token_start;

                while (p < end)
                {
                    if (*p == '\\' &&
                        p + 1 < end)
                    {
                        if (p[1] == '\'')
                        {
                            p += 2; // escaped single quote
                            continue;
                        }

                        // TODO: add handlers for \r, \n and \t etc
                    }

                    if (*p == '\'')
                    {
                        token_end = p++;
                        break;
                    }

                    token_end = ++p;
                }

                tokenType = TokenType::String;
                if (TransformToken(
                    tokenType,
                    std::string_view(token_start, size_t(token_end - token_start)),
                    context->TempToken))
                {
                    context->SetTokenToTemp(tokenType);
                    return Token(context->Line, tokenType, context->GetTokenView());
                }

                context->SetToken(token_start, token_end, tokenType);
                return Token(context->Line, TokenType::String, context->GetTokenView());
            }

            /*
            Checklist
            - [x] single line comments
            - [x] multi line comments
            - [x] double quoted string
            - [x] single quoted string
            - [x] number with hex
            - [x] float
            - [x] keyword
            */

            if (isdigit(*p) ||
                (*p == '-' && p + 1 < end && isdigit(p[1])))
            {
                if (*p == '-')
                    ++p;

                // is it a hex number?
                if (p + 1 < end &&
                    p[1] == 'x')
                {
                    p += 2;
                    token_start = p;

                    while (p < end &&
                        IsHex(*p))
                        ++p;

                    tokenType = TokenType::Hex;
                    context->SetToken(token_start, p, tokenType);

                    if (TransformToken(tokenType, context->GetTokenView(), context->TempToken))
                        context->SetTokenToTemp(tokenType);

                    return Token(context->Line, tokenType, context->GetTokenView());
                }
                else
                {
                    // regular number, or floating point
                    bool floating = false;
                    while (p < end)
                    {
                        if (isdigit(*p))
                        {
                            ++p;
                            continue;
                        }

                        if (*p == '.')
                        {
                            if (floating)
                            {
                                // multiple dots found in number (illegal)
                                context->Empty();
                                return m_errorToken;
                            }

                            floating = true;
                            ++p;
                            continue;
                        }

                        break;
                    }

                    tokenType = TokenType::Number;
                    context->SetToken(token_start, p, tokenType);

                    if (TransformToken(tokenType, context->GetTokenView(), context->TempToken))
                        context->SetTokenToTemp(tokenType);

                    return Token(context->Line, tokenType, context->GetTokenView());
                }
            }

            // keyword
            if (IsKeyword(*p, true))
            {
                ++p;
                while (p < end &&
                    IsKeyword(*p, false))
                    ++p;

                tokenType = TokenType::Keyword;
                context->SetToken(token_start, p, tokenType);

                if (TransformToken(tokenType, context->GetTokenView(), context->TempToken))
                    context->SetTokenToTemp(tokenType);

                return Token(context->Line, tokenType, context->GetTokenView());
            }

            tokenType = TokenType::Delimiter;
            context->SetToken(token_start, ++p, tokenType);

            if (TransformToken(tokenType, context->GetTokenView(), context->TempToken))
                context->SetTokenToTemp(tokenType);

            return Token(context->Line, tokenType, context->GetTokenView());
        }

        Token Scanner::Peek()
        {
            SaveContext();
            auto token = Next();
            RestoreContext();
            return token;
        }

        void Scanner::SaveContext()
        {
            m_contextStack.Push(nl::ConstructShared<Context>(*m_contextStack.GetTop()));
        }

        void Scanner::RestoreContext()
        {
            nl_assert_if_debug(m_contextStack.GetCount() > 2); // must contain at least 3 since the 2 are for ResetContext
            auto context = m_contextStack.Pop(); // context is pulled then destroyed at end of this function
        }

        void Scanner::ResetContext()
        {
            nl_assert_if_debug(m_contextStack.GetCount() >= 2); // assert that the stack must contain at least 2 contexts
            auto context = m_contextStack.Pop(); // context is pulled then destroyed at end of this function
            m_contextStack.Push(nl::ConstructShared<Context>(*m_contextStack.GetTop()));
        }

        bool Scanner::TransformToken(TokenType& tokenType, std::string_view token, nl::String& result)
        {
            // if the token is hex, turn it into a 64bit unsigned number
            if (tokenType == TokenType::Hex)
            {
                uint64_t value = 0;

                const char* p = token.data();
                const char* end = p + token.length();

                while (p < end)
                {
                    value = (value << 4) | HexTable[*p++];
                }

                char num[128];
                int32_t chars = sprintf_s(num, "%llu", value);

                tokenType = TokenType::Number;
                result.Set(num, chars);
                return true;
            }

            return false; // no changes to token
        }

        Scanner Scanner::FromFile(std::string_view filename)
        {
            auto file = nl::io::File::Open(filename, nl::io::CreateMode::OpenExisting, false);
            if (!file)
                throw OpenFileFailedException();

            size_t fileSize = (size_t)file.GetSize();

            auto data = nl::String(fileSize, 0);
            if (file.Read(data.data(), fileSize) != fileSize)
                throw ReadFailedException();

            return Scanner(std::move(data));
        }

        int32_t Scanner::CalculateLines(const char* start, const char* end)
        {
            int32_t line = 0;

            const char* p = start;

            while (p < end)
            {
                if (*p == '\r')
                {
                    if (p + 1 < end &&
                        *(p + 1) == '\n')
                    {
                        p += 2;
                        ++line;
                        continue;
                    }

#if 1 // do we count just a single \r as a new line?
                    ++line;
#endif

                    ++p;
                    continue;
                }

                if (*p == '\n')
                    ++line;

                ++p;
            }

            return line;
        }
    }
}
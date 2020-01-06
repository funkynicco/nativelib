#include "StdAfx.h"

#include <NativeLib/Parsing/Scanner.h>

#include <NativeLib/Assert.h>

// TODO: this parser does not yet support \n \t \" etc in string tokens due to using string_view

namespace nl
{
    namespace parsing
    {
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

        template <size_t SizeOfTable>
        inline void InitializeHexLookup(unsigned char(&aHexLookup)[SizeOfTable])
        {
            static_assert(SizeOfTable == 256, "Hex table must be 256 bytes");
            ZeroMemory(aHexLookup, sizeof(aHexLookup));

            for (unsigned char i = 0; i < 10; ++i)
            {
                aHexLookup[i + '0'] = i;
            }

            for (unsigned char i = 0; i < 6; ++i)
            {
                aHexLookup[i + 'a'] = i + 10;
                aHexLookup[i + 'A'] = i + 10;
            }
        }

        Scanner::Scanner(std::string_view str) :
            m_context(this),
            m_markedContext(this)
        {
            m_dataBegin = str.data();
            m_dataEnd = str.data() + str.length();
            m_context.Reset();

            InitializeHexLookup(m_aHexLookup);
        }

        Scanner::Scanner(const char* str, size_t length) :
            m_context(this),
            m_markedContext(this)
        {
            nl_assert_if_debug(str != nullptr);

            if (length == (size_t)-1)
                length = strlen(str);

            m_container = std::string(str, length);
            m_dataBegin = str;
            m_dataEnd = str + length;
            m_context.Reset();

            InitializeHexLookup(m_aHexLookup);
        }

        Scanner::Scanner(const std::string& str) :
            m_container(str),
            m_context(this),
            m_markedContext(this)
        {
            m_dataBegin = m_container.c_str();
            m_dataEnd = m_dataBegin + m_container.length();
            m_context.Reset();

            InitializeHexLookup(m_aHexLookup);
        }

        Scanner::Scanner(std::string&& str) :
            m_container(std::move(str)),
            m_context(this),
            m_markedContext(this)
        {
            m_dataBegin = m_container.c_str();
            m_dataEnd = m_dataBegin + m_container.length();
            m_context.Reset();

            InitializeHexLookup(m_aHexLookup);
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
            const char*& p = m_context.ViewBegin;
            const char*& end = m_context.ViewEnd;

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
            const char*& p = m_context.ViewBegin;
            const char*& end = m_context.ViewEnd;

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
            const char*& p = m_context.ViewBegin;
            const char*& end = m_context.ViewEnd;

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
            m_context.TempToken.clear();

            auto tokenType = TokenType::Error;

            const char*& p = m_context.ViewBegin;
            const char*& end = m_context.ViewEnd;

            for (;;)
            {
                if (!SkipBlank())
                {
                    m_context.Empty();
                    return TokenType::EndOfFile;
                }

                if (*p == '/' &&
                    p + 1 < end)
                {
                    if (*(p + 1) == '/')
                    {
                        p += 2;
                        if (!SkipSingleComment())
                        {
                            m_context.Empty();
                            return TokenType::EndOfFile;
                        }

                        continue;
                    }

                    if (*(p + 1) == '*')
                    {
                        p += 2;
                        if (!SkipMultiComment())
                        {
                            m_context.Empty();
                            return TokenType::EndOfFile;
                        }

                        continue;
                    }
                }

                break;
            }

            const char* token_start = p;

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
                    m_context.TempToken))
                {
                    m_context.SetTokenToTemp(tokenType);
                    return Token(tokenType, m_context.GetTokenView());
                }

                m_context.SetToken(token_start, token_end, tokenType);
                return Token(TokenType::String, m_context.GetTokenView());
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
                    m_context.TempToken))
                {
                    m_context.SetTokenToTemp(tokenType);
                    return Token(tokenType, m_context.GetTokenView());
                }

                m_context.SetToken(token_start, token_end, tokenType);
                return Token(TokenType::String, m_context.GetTokenView());
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
                    m_context.SetToken(token_start, p, tokenType);

                    if (TransformToken(tokenType, m_context.GetTokenView(), m_context.TempToken))
                        m_context.SetTokenToTemp(tokenType);

                    return Token(tokenType, m_context.GetTokenView());
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
                                m_context.Empty();
                                return TokenType::Error;
                            }

                            floating = true;
                            ++p;
                            continue;
                        }

                        break;
                    }

                    tokenType = TokenType::Number;
                    m_context.SetToken(token_start, p, tokenType);

                    if (TransformToken(tokenType, m_context.GetTokenView(), m_context.TempToken))
                        m_context.SetTokenToTemp(tokenType);

                    return Token(tokenType, m_context.GetTokenView());
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
                m_context.SetToken(token_start, p, tokenType);

                if (TransformToken(tokenType, m_context.GetTokenView(), m_context.TempToken))
                    m_context.SetTokenToTemp(tokenType);

                return Token(tokenType, m_context.GetTokenView());
            }

            tokenType = TokenType::Delimiter;
            m_context.SetToken(token_start, ++p, tokenType);

            if (TransformToken(tokenType, m_context.GetTokenView(), m_context.TempToken))
                m_context.SetTokenToTemp(tokenType);

            return Token(tokenType, m_context.GetTokenView());
        }

        void Scanner::SetMark()
        {
            m_markedContext = m_context;
        }

        void Scanner::GoMark()
        {
            m_context = m_markedContext;
        }

        bool Scanner::TransformToken(TokenType& tokenType, std::string_view token, std::string& result)
        {
            // if the token is hex, turn it into a 64bit unsigned number
            if (tokenType == TokenType::Hex)
            {
                unsigned __int64 value = 0;

                const char* p = token.data();
                const char* end = p + token.length();

                while (p < end)
                {
                    value = (value << 4) | m_aHexLookup[*p++];
                }

                char num[128];
                int chars = sprintf_s(num, "%llu", value);

                tokenType = TokenType::Number;
                result.assign(num, chars);
                return true;
            }

            return false; // no changes to token
        }

        Scanner Scanner::FromFile(const char* filename)
        {
            // TODO: implement more safe file i/o

            HANDLE hFile = CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
            if (hFile == INVALID_HANDLE_VALUE)
                throw std::exception("Could not open file");

            LARGE_INTEGER liSize;
            GetFileSizeEx(hFile, &liSize);

            std::string data((size_t)liSize.QuadPart, 0);

            DWORD dw;
            ReadFile(hFile, data.data(), (DWORD)liSize.QuadPart, &dw, nullptr);
            CloseHandle(hFile);

            return Scanner(std::move(data));
        }
    }
}
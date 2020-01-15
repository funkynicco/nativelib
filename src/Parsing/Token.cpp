#include "StdAfx.h"

#include <NativeLib/Parsing/Token.h>
#include <NativeLib/Parsing/Scanner.h>

#include <NativeLib/Assert.h>

namespace nl
{
    namespace parsing
    {
        Token::Token() :
            m_line(1),
            m_tokenType(TokenType::EndOfFile)
        {
        }

        Token::Token(int32_t line, TokenType tokenType) :
            m_line(line),
            m_tokenType(tokenType)
        {
        }

        Token::Token(int32_t line, TokenType tokenType, std::string_view token) :
            m_line(line),
            m_tokenType(tokenType),
            m_token(token)
        {
        }

        Token::Token(int32_t line, TokenType tokenType, const nl::String& token) :
            m_line(line),
            m_tokenType(tokenType),
            m_transformedToken(token),
            m_token(m_transformedToken)
        {
        }

        Token::Token(int32_t line, TokenType tokenType, nl::String&& token) :
            m_line(line),
            m_tokenType(tokenType),
            m_transformedToken(std::move(token)),
            m_token(m_transformedToken)
        {
        }

        Token::operator TokenType() const
        {
            return m_tokenType;
        }

        Token::operator std::string_view() const
        {
            return m_token;
        }

        Token::operator nl::String() const
        {
            return nl::String(m_token);
        }

        Token::operator bool() const
        {
            return (int)m_tokenType >= 0;
        }

        bool Token::operator ==(std::string_view value) const
        {
            return value == m_token;
        }

        bool Token::operator !=(std::string_view value) const
        {
            return value != m_token;
        }

        std::string_view Token::view() const
        {
            return m_token;
        }

        const char* Token::data() const
        {
            return m_token.data();
        }

        size_t Token::length() const
        {
            return m_token.length();
        }

        bool Token::GetToken(int32_t* pnValue) const
        {
            auto token = m_token;
            if (token.length() == 0)
                return false;

            bool isNegative = token[0] == '-';
            if (isNegative)
                token = token.substr(1);

            int32_t value = 0;
            for (auto x : token)
            {
                value *= 10;
                value += x - '0';
            }

            if (isNegative)
                value = -value;

            *pnValue = value;
            return true;
        }

        bool Token::GetToken(uint32_t* pnValue) const
        {
            auto token = m_token;
            if (token.length() == 0)
                return false;

            uint32_t value = 0;
            for (auto x : token)
            {
                value *= 10;
                value += x - '0';
            }

            *pnValue = value;
            return true;
        }

        bool Token::GetToken(int64_t* pnValue) const
        {
            auto token = m_token;
            if (token.length() == 0)
                return false;

            bool isNegative = token[0] == '-';
            if (isNegative)
                token = token.substr(1);

            int64_t value = 0;
            for (auto x : token)
            {
                value *= 10;
                value += (int64_t)x - '0';
            }

            if (isNegative)
                value = -value;

            *pnValue = value;
            return true;
        }

        bool Token::GetToken(uint64_t* pnValue) const
        {
            auto token = m_token;
            if (token.length() == 0)
                return false;

            uint64_t value = 0;
            for (auto x : token)
            {
                value *= 10;
                value += (uint64_t)x - '0';
            }

            *pnValue = value;
            return true;
        }

        bool Token::GetToken(float* pnValue) const
        {
            auto token = m_token;

            char bytes[128];
            memcpy(bytes, token.data(), token.length());
            bytes[token.length()] = 0;

            *pnValue = strtof(bytes, nullptr);
            return true;
        }

        bool Token::GetToken(double* pnValue) const
        {
            auto token = m_token;

            char bytes[128];
            memcpy(bytes, token.data(), token.length());
            bytes[token.length()] = 0;

            *pnValue = strtod(bytes, nullptr);
            return true;
        }

        bool Token::IsKeyword() const
        {
            return m_tokenType == TokenType::Keyword;
        }

        bool Token::IsDelimiter() const
        {
            return m_tokenType == TokenType::Delimiter;
        }

        bool Token::IsString() const
        {
            return m_tokenType == TokenType::String;
        }

        bool Token::IsNumber() const
        {
            return m_tokenType == TokenType::Number;
        }

        bool Token::IsHex() const
        {
            return m_tokenType == TokenType::Hex;
        }

        bool Token::IsFloat() const
        {
            return m_tokenType == TokenType::Float;
        }

        int32_t Token::GetLine() const
        {
            return m_line;
        }
    }
}
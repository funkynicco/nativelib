#include "StdAfx.h"

#include <NativeLib/Parsing/Scanner.h>

#include <NativeLib/Assert.h>

namespace nl
{
    namespace parsing
    {
        Token::Token() :
            m_tokenType(TokenType::EndOfFile)
        {
        }

        Token::Token(TokenType tokenType) :
            m_tokenType(tokenType)
        {
        }

        Token::Token(TokenType tokenType, std::string_view token) :
            m_tokenType(tokenType),
            m_token(token)
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

        Token::operator std::string() const
        {
            return std::string(m_token);
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

        const char* Token::data() const
        {
            return m_token.data();
        }

        size_t Token::length() const
        {
            return m_token.length();
        }

        bool Token::GetToken(int* pnValue) const
        {
            auto token = m_token;
            if (token.length() == 0)
                return false;

            bool isNegative = token[0] == '-';
            if (isNegative)
                token = token.substr(1);

            int value = 0;
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

        bool Token::GetToken(unsigned int* pnValue) const
        {
            auto token = m_token;
            if (token.length() == 0)
                return false;

            unsigned int value = 0;
            for (auto x : token)
            {
                value *= 10;
                value += x - '0';
            }

            *pnValue = value;
            return true;
        }

        bool Token::GetToken(__int64* pnValue) const
        {
            auto token = m_token;
            if (token.length() == 0)
                return false;

            bool isNegative = token[0] == '-';
            if (isNegative)
                token = token.substr(1);

            __int64 value = 0;
            for (auto x : token)
            {
                value *= 10;
                value += (__int64)x - '0';
            }

            if (isNegative)
                value = -value;

            *pnValue = value;
            return true;
        }

        bool Token::GetToken(unsigned __int64* pnValue) const
        {
            auto token = m_token;
            if (token.length() == 0)
                return false;

            unsigned __int64 value = 0;
            for (auto x : token)
            {
                value *= 10;
                value += (unsigned __int64)x - '0';
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
    }
}
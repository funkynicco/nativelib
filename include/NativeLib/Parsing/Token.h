#pragma once

#include <NativeLib/Parsing/Util.h>
#include <NativeLib/String.h>

#include <ostream>

namespace nl
{
    namespace parsing
    {
        class Token
        {
        public:
            Token();
            Token(int32_t line, TokenType tokenType);
            Token(int32_t line, TokenType tokenType, std::string_view token);

            operator TokenType() const;
            operator std::string_view() const;
            operator nl::String() const;
            operator bool() const;

            bool operator ==(std::string_view value) const;
            bool operator !=(std::string_view value) const;

            std::string_view view() const;
            // data is NOT nullterminated!
            const char* data() const;
            size_t length() const;

            bool GetToken(int32_t* pnValue) const;
            bool GetToken(uint32_t* pnValue) const;
            bool GetToken(int64_t* pnValue) const;
            bool GetToken(uint64_t* pnValue) const;
            bool GetToken(float* pnValue) const;
            bool GetToken(double* pnValue) const;

            bool IsKeyword() const;
            bool IsDelimiter() const;
            bool IsString() const;
            bool IsNumber() const;
            bool IsHex() const;
            bool IsFloat() const;

            int32_t GetLine() const;

        private:
            TokenType m_tokenType;
            std::string_view m_token;
            int32_t m_line;
        };

        template <class _Elem, class _Traits>
        inline std::basic_ostream<_Elem, _Traits>& operator <<(std::basic_ostream<_Elem, _Traits>& _Ostr, const Token& token)
        {
            _Ostr << token.view();
            return _Ostr;
        }
    }
}
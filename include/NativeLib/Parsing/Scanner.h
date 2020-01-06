#pragma once

#include <NativeLib/Logger.h>
#include <NativeLib/RAII/Scoped.h>

#include <string>

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

        std::string_view TokenTypeToString(TokenType tokenType);

        enum
        {
            SCANNER_OPTION_NONE = 0,
            SCANNER_OPTION_NO_WARN_ON_OPEN = (1 << 0),
            SCANNER_OPTION_DEFAULT = SCANNER_OPTION_NONE
        };

        class Scanner
        {
        public:
            Scanner(std::string_view str);
            Scanner(const char* str, size_t length = (size_t)-1);
            Scanner(const std::string& str);
            Scanner(std::string&& str);
            virtual ~Scanner();

            Scanner(const Scanner&) = delete;
            Scanner(Scanner&&) noexcept = delete;
            Scanner& operator =(const Scanner&) = delete;
            Scanner& operator =(Scanner&&) noexcept = delete;

            TokenType NextToken();

            void SetMark();
            void GoMark();

            TokenType GetTokenType() const;
            std::string_view GetToken() const;
            bool GetToken(int* pnValue) const;
            bool GetToken(unsigned int* pnValue) const;
            bool GetToken(__int64* pnValue) const;
            bool GetToken(unsigned __int64* pnValue) const;
            bool GetToken(float* pnValue) const;
            bool GetToken(double* pnValue) const;

            static Scanner FromFile(const char* filename);

        protected:
            virtual bool TransformToken(TokenType& tokenType, std::string_view token, std::string& result);

        private:
            struct Context
            {
                // int LineNumber; ??

                class Scanner* lpScanner;

                const char* ViewBegin;
                const char* ViewEnd;

                const char* TokenBegin;
                const char* TokenEnd;
                TokenType TokenType;

                // use only when a token needs to change value, ie \n in a string
                std::string TempToken;

                Context(class Scanner* scanner) :
                    lpScanner(scanner)
                {
                    Reset();
                }

                Context(const Context&) = delete;
                Context(Context&&) noexcept = delete;
                Context& operator =(Context&&) noexcept = delete;

                Context& operator =(const Context& context)
                {
                    lpScanner = context.lpScanner;
                    ViewBegin = context.ViewBegin;
                    ViewEnd = context.ViewEnd;
                    TokenBegin = context.TokenBegin;
                    TokenEnd = context.TokenEnd;
                    TokenType = context.TokenType;
                    return *this;
                }

                void Reset()
                {
                    ViewBegin = lpScanner->m_dataBegin;
                    ViewEnd = lpScanner->m_dataEnd;
                    TokenBegin = TokenEnd = nullptr;
                    TokenType = TokenType::Error;
                }

                void Empty()
                {
                    ViewBegin = ViewEnd = lpScanner->m_dataEnd;
                    TokenBegin = TokenEnd = nullptr;
                    TokenType = TokenType::Error;
                }

                void SetToken(const char* tokenBegin, const char* tokenEnd, nl::parsing::TokenType tokenType)
                {
                    TokenBegin = tokenBegin;
                    TokenEnd = tokenEnd;
                    TokenType = tokenType;
                }

                void SetTokenToTemp(nl::parsing::TokenType tokenType)
                {
                    TokenBegin = TempToken.c_str();
                    TokenEnd = TokenBegin + TempToken.length();
                    TokenType = tokenType;
                }

                bool IsEnd() const
                {
                    return ViewBegin == ViewEnd;
                }

                bool IsTokenSet() const
                {
                    return TokenBegin != nullptr;
                }

                std::string_view GetView() const
                {
                    return std::string_view(ViewBegin, ViewEnd - ViewBegin);
                }

                std::string_view GetTokenView() const
                {
                    return std::string_view(TokenBegin, TokenEnd - TokenBegin);
                }
            };

            std::string m_container;
            
            const char* m_dataBegin;
            const char* m_dataEnd;

            Context m_context;
            Context m_markedContext;

            unsigned char m_aHexLookup[256];

            bool IsWhitespace(char c) const;
            bool IsKeyword(char c, bool first) const;
            bool IsHex(char c) const;
            bool SkipBlank();
            bool SkipSingleComment();
            bool SkipMultiComment();
        };
    }
}
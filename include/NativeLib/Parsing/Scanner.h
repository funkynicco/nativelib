#pragma once

#include <NativeLib/Parsing/Util.h>
#include <NativeLib/Parsing/Token.h>
#include <NativeLib/Parsing/Context.h>

#include <NativeLib/RAII/Shared.h>
#include <NativeLib/Containers/Stack.h>

#include <string>
#include <stack>

namespace nl
{
    namespace parsing
    {
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
            Scanner() noexcept;
            Scanner(std::string_view str);
            Scanner(const char* str, size_t length = (size_t)-1);
            Scanner(const std::string& str);
            Scanner(std::string&& str);

            Scanner(Scanner&& other) noexcept;
            Scanner& operator =(Scanner&& other) noexcept;

            virtual ~Scanner();

            Scanner(const Scanner&) = delete;
            Scanner& operator =(const Scanner&) = delete;
            
            // Advances the scanner to the next token
            Token Next();

            // Pushes the context onto the context stack
            void SaveContext();
            
            // Restores the previous context in the stack
            void RestoreContext();

            // Restores the context and then saves context again to reset the context to the previous context in the stack
            void ResetContext();

            static Scanner FromFile(const char* filename);

        protected:
            virtual bool TransformToken(TokenType& tokenType, std::string_view token, std::string& result);

        private:
            nl::Stack<nl::Shared<Context>, 4> m_contextStack;

            const Token m_endOfFileToken;
            const Token m_errorToken;

            bool IsWhitespace(char c) const;
            bool IsKeyword(char c, bool first) const;
            bool IsHex(char c) const;
            bool SkipBlank();
            bool SkipSingleComment();
            bool SkipMultiComment();

            static int CalculateLines(const char* start, const char* end);
        };
    }
}
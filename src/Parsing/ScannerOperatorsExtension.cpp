#include "StdAfx.h"

#include <NativeLib/Parsing/Scanner.h>

#define CASE_SINGLE(ch, op) case ch: ++p; *operatorType = op; return true;

namespace nl::parsing
{
    bool Scanner::ReadOperator(const char*& p, const char* end, OperatorType* operatorType)
    {
        switch (*p)
        {
        case ':':
            if (p + 1 < end && p[1] == ':')
            {
                p += 2;
                *operatorType = OperatorType::ScopeResolution;
            }
            else
            {
                ++p;
                *operatorType = OperatorType::Colon;
            }

            return true;

            CASE_SINGLE(';', OperatorType::Semicolon);
            CASE_SINGLE('.', OperatorType::Period);
            CASE_SINGLE(',', OperatorType::Comma);
            CASE_SINGLE('#', OperatorType::Hash);
            CASE_SINGLE('?', OperatorType::Question);
            CASE_SINGLE('\\', OperatorType::BackwardsSlash);
            CASE_SINGLE('$', OperatorType::Dollar);
            CASE_SINGLE('_', OperatorType::Underscore);
            CASE_SINGLE('(', OperatorType::OpenParenthesis);
            CASE_SINGLE(')', OperatorType::CloseParenthesis);
            CASE_SINGLE('{', OperatorType::OpenBrace);
            CASE_SINGLE('}', OperatorType::CloseBrace);
            CASE_SINGLE('[', OperatorType::OpenBracket);
            CASE_SINGLE(']', OperatorType::CloseBracket);

        case '+':
            if (p + 1 < end)
            {
                if (p[1] == '+')
                {
                    p += 2;
                    *operatorType = OperatorType::SuffixIncrement;
                    return true;
                }

                if (p[1] == '=')
                {
                    p += 2;
                    *operatorType = OperatorType::AssignAdd;
                    return true;
                }
            }

            ++p;
            *operatorType = OperatorType::Add;
            return true;

        case '-':
            if (p + 1 < end)
            {
                if (p[1] == '-')
                {
                    p += 2;
                    *operatorType = OperatorType::SuffixDecrement;
                    return true;
                }

                if (p[1] == '=')
                {
                    p += 2;
                    *operatorType = OperatorType::AssignSubtract;
                    return true;
                }

                if (p[1] == '>')
                {
                    p += 2;
                    *operatorType = OperatorType::PointerAccess;
                    return true;
                }
            }

            ++p;
            *operatorType = OperatorType::Subtract;
            return true;

        case '*':
            if (p + 1 < end && p[1] == '=')
            {
                p += 2;
                *operatorType = OperatorType::AssignMultiply;
                return true;
            }

            ++p;
            *operatorType = OperatorType::Multiply;
            return true;

        case '/':
            if (p + 1 < end && p[1] == '=')
            {
                p += 2;
                *operatorType = OperatorType::AssignDivide;
                return true;
            }

            ++p;
            *operatorType = OperatorType::Divide;
            return true;

        case '%':
            if (p + 1 < end && p[1] == '=')
            {
                p += 2;
                *operatorType = OperatorType::AssignModulus;
                return true;
            }

            ++p;
            *operatorType = OperatorType::Modulus;
            return true;

        case '!':
            if (p + 1 < end && p[1] == '=')
            {
                p += 2;
                *operatorType = OperatorType::NotEqual;
                return true;
            }

            ++p;
            *operatorType = OperatorType::LogicalNot;
            return true;

        case '&':
            if (p + 1 < end)
            {
                if (p[1] == '&')
                {
                    p += 2;
                    *operatorType = OperatorType::LogicalAnd;
                    return true;
                }

                if (p[1] == '=')
                {
                    p += 2;
                    *operatorType = OperatorType::AssignAnd;
                    return true;
                }
            }

            ++p;
            *operatorType = OperatorType::BinaryAnd;
            return true;

        case '|':
            if (p + 1 < end)
            {
                if (p[1] == '|')
                {
                    p += 2;
                    *operatorType = OperatorType::LogicalOr;
                    return true;
                }

                if (p[1] == '=')
                {
                    p += 2;
                    *operatorType = OperatorType::AssignOr;
                    return true;
                }
            }

            ++p;
            *operatorType = OperatorType::BinaryOr;
            return true;

            CASE_SINGLE('~', OperatorType::BinaryNot);

        case '^':
            if (p + 1 < end && p[1] == '=')
            {
                p += 2;
                *operatorType = OperatorType::AssignXor;
                return true;
            }

            ++p;
            *operatorType = OperatorType::BinaryXor;
            return true;

        case '<':
            if (p + 1 < end)
            {
                if (p[1] == '<')
                {
                    if (p + 2 < end && p[2] == '=')
                    {
                        p += 3;
                        *operatorType = OperatorType::AssignLeftShift;
                        return true;
                    }

                    p += 2;
                    *operatorType = OperatorType::LeftShift;
                    return true;
                }

                if (p[1] == '=')
                {
                    p += 2;
                    *operatorType = OperatorType::LessOrEqual;
                    return true;
                }
            }

            ++p;
            *operatorType = OperatorType::Less;
            return true;

        case '>':
            if (p + 1 < end)
            {
                if (p[1] == '>')
                {
                    if (p + 2 < end && p[2] == '=')
                    {
                        p += 3;
                        *operatorType = OperatorType::AssignRightShift;
                        return true;
                    }

                    p += 2;
                    *operatorType = OperatorType::RightShift;
                    return true;
                }

                if (p[1] == '=')
                {
                    p += 2;
                    *operatorType = OperatorType::GreaterOrEqual;
                    return true;
                }
            }

            ++p;
            *operatorType = OperatorType::Greater;
            return true;

        case '=':
            if (p + 1 < end && p[1] == '=')
            {
                p += 2;
                *operatorType = OperatorType::Equal;
            }
            else
            {
                ++p;
                *operatorType = OperatorType::Assign;
            }

            return true;
        }

        return false;
    }
}
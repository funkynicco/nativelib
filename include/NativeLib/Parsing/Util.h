#pragma once

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
            Float,
            
            // A token can be an operator if the operator extension is enabled
            // Check the 
            Operator
        };

        enum class OperatorType
        {
            Unset,
            ScopeResolution, // ::
            Colon, // :
            Semicolon, // ;
            Period, // .
            Comma, // ,
            Hash, // #
            Question, // ?
            BackwardsSlash,
            Dollar, // $
            Underscore, // _
            SuffixIncrement, // ++
            SuffixDecrement, // --
            OpenParenthesis, // (
            CloseParenthesis, // )
            OpenBrace, // {
            CloseBrace, // }
            OpenBracket, // [
            CloseBracket, // ]
            PointerAccess, // ->
            Add, // +
            Subtract, // -
            Multiply, // *
            Divide, // /
            Modulus, // %
            LogicalNot, // !
            LogicalAnd, // &&
            LogicalOr, // ||
            BinaryNot, // ~
            BinaryAnd, // &
            BinaryOr, // |
            BinaryXor, // ^
            LeftShift, // <<
            RightShift, // >>
            Less, // <
            Greater, // >
            LessOrEqual, // <=
            GreaterOrEqual, // >=
            Equal, // ==
            NotEqual, // !=
            Assign, // =
            AssignAdd, // +=
            AssignSubtract, // -=
            AssignMultiply, // *=
            AssignDivide, // /=
            AssignModulus, // %=
            AssignLeftShift, // <<=
            AssignRightShift, // >>=
            AssignAnd, // &=
            AssignOr, // |=
            AssignXor, // ^=
        };
    }
}
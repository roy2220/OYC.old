#pragma once


#include <cstdint>
#include <string>


namespace OYC {

enum class TokenType: std::uint32_t
{
    No = 0,
    EndOfFile,
    WhiteSpace,
    Comment,
    IntegerLiteral,
    FloatingPointLiteral,
    StringLiteral,
    Identifier,
    KeywordBegin,
    NilKeyword = KeywordBegin,
    FalseKeyword,
    TrueKeyword,
    DictKeyword,
    FuncKeyword,
    AutoKeyword,
    BreakKeyword,
    ContinueKeyword,
    ReturnKeyword,
    IfKeyword,
    ElseKeyword,
    SwitchKeyword,
    CaseKeyword,
    DefaultKeyword,
    WhileKeyword,
    DoKeyword,
    ForKeyword,
    ForeachKeyword,
    KeywordEnd,
    Illegal = KeywordEnd
};


struct Token
{
    TokenType type = TokenType::No;
    std::string value;
    int lineNumber = 0;
    int columnNumber = 0;
};


constexpr TokenType MakeTokenType(char, char = '\0', char = '\0');

const char *TokenTypeToString(TokenType);


constexpr TokenType
MakeTokenType(char x, char y, char z)
{
    return static_cast<TokenType>((x & UINT32_C(0x7F)) << 11 | (y & UINT32_C(0x7F)) << 18
                                  | (z & UINT32_C(0x7F)) << 25);
}

} // namespace OYC

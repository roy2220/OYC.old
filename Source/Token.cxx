#include "Token.h"


namespace OYC {

namespace {

const char *PredefinedTokenTypeToString[] = {
    nullptr,
    "end-of-file",
    "white space",
    "comment",
    "integer literal",
    "floating-point literal",
    "string literal",
    "identifier",
    "illegal",
    "null",
    "false",
    "true",
    "dict",
    "func",
    "auto",
    "break",
    "continue",
    "return",
    "if",
    "else",
    "switch",
    "case",
    "default",
    "while",
    "do",
    "for",
    "foreach",
    "sizeof",
    "this"
};

} // namespace


const char *
TokenTypeToString(TokenType tokenType)
{
    auto k = static_cast<std::uint32_t>(tokenType);
    std::int32_t i = k & 0x7FF;

    if (i == 0) {
        static char string[4];
        string[0] = k >> 11 & 0x7F;
        string[1] = k >> 18 & 0x7F;
        string[2] = k >> 25;
        return string;
    } else {
        return PredefinedTokenTypeToString[i];
    }
}

} // namespace OYC

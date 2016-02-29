#include "SyntaxError.h"

#include <cctype>


namespace OYC {

namespace {

void representToken(TokenType, std::string *);

} // namespace


SyntaxError
SyntaxError::IllegalToken(const Token &token)
{
    std::string description;
    description += "Illegal token `";
    description += token.value;
    description += '`';
    return SyntaxError(token.lineNumber, token.columnNumber, description);
}


SyntaxError
SyntaxError::UnexpectedToken(const Token &token, TokenType tokenType1, TokenType tokenType2)
{
    std::string description;
    description += "Unexpected token `";
    description += token.value;
    description += '`';

    if (tokenType1 != TokenType::No) {
        description += ", expect ";
        representToken(tokenType1, &description);

        if (tokenType2 != TokenType::No) {
            description += " or ";
            representToken(tokenType2, &description);
        }
    }

    return SyntaxError(token.lineNumber, token.columnNumber, description);
}


SyntaxError::SyntaxError(int lineNumber, int columnNumber, const std::string &description)
{
    message_ += "Syntax Error: ";
    message_ += std::to_string(lineNumber);
    message_ += ':';
    message_ += std::to_string(columnNumber);
    message_ += ": ";
    message_ += description;
}


namespace {

void
representToken(TokenType tokenType, std::string *representation)
{
    const char *string = TokenTypeToString(tokenType);

    if (std::isupper(static_cast<unsigned char>(*string))) {
        *representation += string;
    } else {
        *representation += '`';
        *representation += string;
        *representation += '`';
    }
}

} // namespace

} // namespace OYC

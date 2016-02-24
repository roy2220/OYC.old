#include "SyntaxError.h"

#include <cctype>

#include "Token.h"


namespace OYC {


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
SyntaxError::UnexpectedToken(const Token &token, TokenType tokenType)
{
    std::string description;
    description += "Unexpected token `";
    description += token.value;
    description += '`';

    if (tokenType != TokenType::No) {
        const char *string = TokenTypeToString(tokenType);
        description += ", expect ";

        if (std::isupper(static_cast<unsigned char>(*string))) {
            description += string;
        } else {
            description += '`';
            description += string;
            description += '`';
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

} // namespace OYC

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
    description += "illegal token `";
    description += token.value;
    description += '`';
    return SyntaxError(token.lineNumber, token.columnNumber, description);
}


SyntaxError
SyntaxError::UnexpectedToken(const Token &token, TokenType tokenType1, TokenType tokenType2)
{
    std::string description;
    description += "expected ";
    representToken(tokenType1, &description);

    if (tokenType2 != TokenType::No) {
        description += " or ";
        representToken(tokenType2, &description);
    }

    description += " before token `";
    description += token.value;
    description += '`';
    return SyntaxError(token.lineNumber, token.columnNumber, description);
}


SyntaxError
SyntaxError::UnexpectedToken(const Token &token, const std::string &something)
{
    std::string description;
    description += "expected ";
    description += something;
    description += " before token `";
    description += token.value;
    description += '`';
    return SyntaxError(token.lineNumber, token.columnNumber, description);
}


SyntaxError
SyntaxError::DuplicateDefaultLabel(const Token &token)
{
    return SyntaxError(token.lineNumber, token.columnNumber, "duplicate default label");
}


SyntaxError
SyntaxError::UndeclaredVariable(const Token &token)
{
    std::string description;
    description += "undeclared variable `";
    description += token.value;
    description += '`';
    return SyntaxError(token.lineNumber, token.columnNumber, description);
}


SyntaxError::SyntaxError(int lineNumber, int columnNumber, const std::string &description)
{
    message_ += std::to_string(lineNumber);
    message_ += ':';
    message_ += std::to_string(columnNumber);
    message_ += ": ";
    message_ += "syntax error: ";
    message_ += description;
}


namespace {

void
representToken(TokenType tokenType, std::string *representation)
{
    const char *string = TokenTypeToString(tokenType);

    if (TokenTypeIsAbstract(tokenType)) {
        *representation += string;
    } else {
        *representation += '`';
        *representation += string;
        *representation += '`';
    }
}

} // namespace

} // namespace OYC

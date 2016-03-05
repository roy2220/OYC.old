#pragma once


#include <cstdint>
#include <string>
#include <utility>

#include "Token.h"


namespace OYC {

class SyntaxError final : public std::exception
{
    SyntaxError(const SyntaxError &) = delete;
    SyntaxError &operator=(const SyntaxError &) = delete;

public:
    inline SyntaxError(SyntaxError &&);

    inline const char *what() const noexcept override;

    static SyntaxError IllegalToken(const Token &);
    static SyntaxError UnexpectedToken(const Token &, TokenType, TokenType = TokenType::No);
    static SyntaxError UnexpectedToken(const Token &, const std::string &);
    static SyntaxError DuplicateDefaultLabel(const Token &);
    static SyntaxError UndeclaredVariable(const Token &);

private:
    std::string message_;

    explicit SyntaxError(int, int, const std::string &);
};


SyntaxError::SyntaxError(SyntaxError &&other)
    : message_(std::move(other.message_))
{
}


const char *
SyntaxError::what() const noexcept
{
    return message_.c_str();
}

} // namespace OYC

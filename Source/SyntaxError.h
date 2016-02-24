#pragma once


#include <cstdint>
#include <string>
#include <utility>


namespace OYC {

enum class TokenType: std::uint32_t;

struct Token;


class SyntaxError final: public std::exception
{
    SyntaxError(const SyntaxError &) = delete;
    SyntaxError &operator=(SyntaxError &) = delete;

public:
    inline SyntaxError(SyntaxError &&);

    inline const char *what() const noexcept override;

    static SyntaxError IllegalToken(const Token &);
    static SyntaxError UnexpectedToken(const Token &, TokenType);

private:
    std::string message_;

    SyntaxError(int, int, const std::string &);
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

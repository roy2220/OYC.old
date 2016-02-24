#include "Scanner.h"

#include <cctype>
#include <cstring>
#include <unordered_map>
#include <type_traits>
#include <iterator>

#include "Token.h"


namespace OYC {

namespace {

const auto KeywordToTokenType = [] () -> std::unordered_map<std::string, TokenType> {
    std::unordered_map<std::string, TokenType> result;

    for (auto k = static_cast<std::underlying_type_t<TokenType>>(TokenType::KeywordBegin)
         ; k < static_cast<std::underlying_type_t<TokenType>>(TokenType::KeywordEnd); ++k) {
        auto tokenType = static_cast<TokenType>(k);
        result.emplace(TokenTypeToString(tokenType), tokenType);
    }

    return result;
}();


bool isodigit(int);

} // namespace


Token
Scanner::readToken()
{
    Token token;
    token.lineNumber = lineNumber_;
    token.columnNumber = columnNumber_;
    int c1 = peekChar(1);

    if (c1 < 0) {
        token.type = TokenType::EndOfFile;
    } else switch (c1) {
        int c2;
        int c3;

    case '!':
    case '%':
    case '&':
    case '*':
    case '^':
    case '|':
    case '=':
        token.value += readChar();
        c2 = peekChar(1);

        if (c2 == '=') {
            token.value += readChar();
            token.type = MakeTokenType(c1, c2);
        } else {
            token.type = MakeTokenType(c1);
        }

        break;

    case '\"':
        matchStringLiteralToken(&token);
        break;

    case '(':
    case ')':
    case ',':
    case ':':
    case ';':
    case '?':
    case '[':
    case ']':
    case '{':
    case '}':
    case '~':
        token.value += readChar();
        token.type = MakeTokenType(c1);
        break;

    case '+':
    case '-':
        token.value += readChar();
        c2 = peekChar(1);

        if (c2 == c1 || c2 == '=') {
            token.value += readChar();
            token.type = MakeTokenType(c1, c2);
        } else {
            token.type = MakeTokenType(c1);
        }

        break;

    case '.':
        c2 = peekChar(2);

        if (std::isdigit(c2)) {
            matchNumberLiteralToken(&token);
        } else {
            token.value += readChar();
            token.type = MakeTokenType(c1);
        }

        break;

    case '/':
        c2 = peekChar(2);

        if (c2 == '*' || c2 == '/') {
            matchCommentToken(&token);
        } else if (c2 == '=') {
            token.value += readChar();
            token.value += readChar();
            token.type = MakeTokenType(c1, c2);
        } else {
            token.value += readChar();
            token.type = MakeTokenType(c1);
        }

        break;

    case '<':
    case '>':
        token.value += readChar();
        c2 = peekChar(1);

        if (c2 == c1) {
            token.value += readChar();
            c3 = peekChar(1);

            if (c3 == '=') {
                token.value += readChar();
                token.type = MakeTokenType(c1, c2, c3);
            } else {
                token.type = MakeTokenType(c1, c1);
            }
        } else if (c2 == '=') {
            token.value += readChar();
            token.type = MakeTokenType(c1, c2);
        } else {
            token.type = MakeTokenType(c1);
        }

        break;

    default:
        if (std::isspace(c1)) {
            matchWhiteSpaceToken(&token);
        } else if (std::isdigit(c1)) {
            matchNumberLiteralToken(&token);
        } else if (std::isalpha(c1) || c1 == '_') {
            matchNameToken(&token);
        } else {
            token.value += readChar();
            token.type = TokenType::Illegal;
        }
    }

    return token;
}


int
Scanner::doReadChar()
{
    return charReader_();
}


int
Scanner::peekChar(int position)
{
    for (int n = position - static_cast<int>(prereadChars_.size()); n >= 1; --n) {
        prereadChars_.push_back(doReadChar());
    }

    return *std::next(prereadChars_.begin(), position - 1);
}


int
Scanner::readChar()
{
    int c;

    if (prereadChars_.empty()) {
        c = doReadChar();
    } else {
        c = prereadChars_.front();
        prereadChars_.pop_front();
    }

    if (c >= 0) {
        if (c == '\n') {
            ++lineNumber_;
            columnNumber_ = 1;
        } else {
            ++columnNumber_;
        }
    }

    return c;
}


void
Scanner::matchWhiteSpaceToken(Token *token)
{
    token->value += readChar();
    int c = peekChar(1);

    while (std::isspace(c)) {
        token->value += readChar();
        c = peekChar(1);
    }

    token->type = TokenType::WhiteSpace;
}


void
Scanner::matchCommentToken(Token *token)
{
    token->value += readChar();
    int c = peekChar(1);

    if (c == '*') {
        token->value += readChar();
        c = peekChar(1);

        while (c >= 0) {
            if (c == '*') {
                token->value += readChar();
                c = peekChar(1);

                if (c == '/') {
                    token->value += readChar();
                    token->type = TokenType::Comment;
                    return;
                }
            } else {
                token->value += readChar();
                c = peekChar(1);
            }
        }

        token->type = TokenType::Illegal;
    } else {
        token->value += readChar();
        c = peekChar(1);

        while (c >= 0 && c != '\n') {
            token->value += readChar();
            c = peekChar(1);
        }

        token->type = TokenType::Comment;
    }
}


void
Scanner::matchNumberLiteralToken(Token *token)
{
    if (peekChar(1) == '0' && peekChar(2) == 'x') {
        matchNumberLiteralToken16(token);
    } else {
        matchNumberLiteralToken10(token);
    }
}


void
Scanner::matchNumberLiteralToken10(Token *token)
{
    int c = peekChar(1);

    while (std::isdigit(c)) {
        token->value += readChar();
        c = peekChar(1);
    }

    bool floatingPointFlag = false;

    if (c == '.') {
        floatingPointFlag = true;
        token->value += readChar();
        c = peekChar(1);

        while (std::isdigit(c)) {
            token->value += readChar();
            c = peekChar(1);
        }
    }

    if (c == 'E' || c == 'e') {
        floatingPointFlag = true;
        token->value += readChar();
        c = peekChar(1);

        if (c == '+' || c == '-') {
            token->value += readChar();
            c = peekChar(1);
        }

        if (!std::isdigit(c)) {
            if (c >= 0 && c != '\n') {
                token->value += readChar();
            }

            token->type = TokenType::Illegal;
            return;
        }

        token->value += readChar();
        c = peekChar(1);

        while (std::isdigit(c)) {
            token->value += readChar();
            c = peekChar(1);
        }
    }

    if (std::isalpha(c) || c == '_') {
        token->value += readChar();
        token->type = TokenType::Illegal;
        return;
    }

    token->type = floatingPointFlag ? TokenType::FloatingPointLiteral : TokenType::IntegerLiteral;
}


void
Scanner::matchNumberLiteralToken16(Token *token)
{
    token->value += readChar();
    token->value += readChar();
    int c = peekChar(1);

    if (!std::isxdigit(c)) {
        if (c >= 0 && c != '\n') {
            token->value += readChar();
        }

        token->type = TokenType::Illegal;
        return;
    }

    token->value += readChar();
    c = peekChar(1);

    while (std::isxdigit(c)) {
        token->value += readChar();
        c = peekChar(1);
    }

    if (std::isalpha(c) || c == '_') {
        token->value += readChar();
        token->type = TokenType::Illegal;
        return;
    }

    token->type = TokenType::IntegerLiteral;
}


void
Scanner::matchStringLiteralToken(Token *token)
{
    token->value += readChar();
    int c = peekChar(1);

    while (c >= 0 && c != '\n') {
        if (c == '\"') {
            token->value += readChar();
            token->type = TokenType::StringLiteral;
            return;
        }

        if (c == '\\') {
            token->value += readChar();
            c = peekChar(1);

            if (isodigit(c)) {
                token->value += readChar();
                c = peekChar(1);

                if (isodigit(c)) {
                    token->value += readChar();
                    c = peekChar(1);

                    if (isodigit(c)) {
                        token->value += readChar();
                        c = peekChar(1);
                    }
                }
            } else if (c == 'X' || c == 'x' ) {
                token->value += readChar();
                c = peekChar(1);

                if (!std::isxdigit(c)) {
                    if (c >= 0 && c != '\n') {
                        token->value += readChar();
                    }

                    break;
                }

                token->value += readChar();
                c = peekChar(1);

                if (std::isxdigit(c)) {
                    token->value += readChar();
                    c = peekChar(1);
                }
            } else {
                if (std::strchr("\"\'\?\\abfnrtv", c) == nullptr) {
                    if (c >= 0 && c != '\n') {
                        token->value += readChar();
                    }

                    break;
                }

                token->value += readChar();
                c = peekChar(1);
            }
        } else {
            token->value += readChar();
            c = peekChar(1);
        }
    }

    token->type = TokenType::Illegal;
}


void
Scanner::matchNameToken(Token *token)
{
    token->value += readChar();
    int c = peekChar(1);

    while (std::isalnum(c) || c == '_') {
        token->value += readChar();
        c = peekChar(1);
    }

    std::unordered_map<std::string, TokenType>::const_iterator it = KeywordToTokenType
                                                                    .find(token->value);
    token->type = it == KeywordToTokenType.end() ? TokenType::Identifier : it->second;
}


namespace {

bool
isodigit(int c)
{
    return c >= '0' && c <= '7';
}

} // namespace

} // namespace OYC

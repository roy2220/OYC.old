#include "Scanner.h"

#include <cctype>
#include <iterator>
#include <type_traits>
#include <unordered_map>

#include "Error.h"
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
    matchToken(&token);
    return token;
}


int
Scanner::doReadChar()
{
    return input_();
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
Scanner::matchToken(Token *match)
{
    int c1 = peekChar(1);

    if (c1 < 0) {
        match->type = TokenType::EndOfFile;
        return;
    } else switch (c1) {
        int c2;
        int c3;

    case '\t':
    case '\n':
    case '\v':
    case '\f':
    case '\r':
    case ' ':
        matchWhiteSpaceToken(match);
        return;

    case '!':
    case '%':
    case '&':
    case '*':
    case '^':
    case '|':
    case '=':
        match->value += readChar();
        c2 = peekChar(1);

        if (c2 == '=') {
            match->value += readChar();
            match->type = MakeTokenType(c1, c2);
            return;
        } else {
            match->type = MakeTokenType(c1);
            return;
        }

    case '\"':
        matchStringLiteralToken(match);
        return;

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
        match->value += readChar();
        match->type = MakeTokenType(c1);
        return;

    case '+':
    case '-':
        match->value += readChar();
        c2 = peekChar(1);

        if (c2 == c1 || c2 == '=') {
            match->value += readChar();
            match->type = MakeTokenType(c1, c2);
            return;
        } else {
            match->type = MakeTokenType(c1);
            return;
        }

    case '.':
        c2 = peekChar(2);

        if (std::isdigit(c2)) {
            matchNumberLiteralToken(match);
            return;
        } else {
            match->value += readChar();

            if (c2 == '.' && (c3 = peekChar(2)) == '.') {
                match->value += readChar();
                match->value += readChar();
                match->type = MakeTokenType(c1, c2, c3);
                return;
            } else {
                match->type = MakeTokenType(c1);
                return;
            }
        }

    case '/':
        c2 = peekChar(2);

        if (c2 == '*' || c2 == '/') {
            matchCommentToken(match);
            return;
        } else {
            match->value += readChar();

            if (c2 == '=') {
                match->value += readChar();
                match->type = MakeTokenType(c1, c2);
                return;
            } else {
                match->type = MakeTokenType(c1);
                return;
            }
        }

    case '<':
    case '>':
        match->value += readChar();
        c2 = peekChar(1);

        if (c2 == c1) {
            match->value += readChar();
            c3 = peekChar(1);

            if (c3 == '=') {
                match->value += readChar();
                match->type = MakeTokenType(c1, c2, c3);
                return;
            } else {
                match->type = MakeTokenType(c1, c1);
                return;
            }
        } else if (c2 == '=') {
            match->value += readChar();
            match->type = MakeTokenType(c1, c2);
            return;
        } else {
            match->type = MakeTokenType(c1);
            return;
        }

    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
        matchNumberLiteralToken(match);
        return;

    case 'A': case 'a':
    case 'B': case 'b':
    case 'C': case 'c':
    case 'D': case 'd':
    case 'E': case 'e':
    case 'F': case 'f':
    case 'G': case 'g':
    case 'H': case 'h':
    case 'I': case 'i':
    case 'J': case 'j':
    case 'K': case 'k':
    case 'L': case 'l':
    case 'M': case 'm':
    case 'N': case 'n':
    case 'O': case 'o':
    case 'P': case 'p':
    case 'Q': case 'q':
    case 'R': case 'r':
    case 'S': case 's':
    case 'T': case 't':
    case 'U': case 'u':
    case 'V': case 'v':
    case 'W': case 'w':
    case 'X': case 'x':
    case 'Y': case 'y':
    case 'Z': case 'z':
    case '_':
        matchNameToken(match);
        return;

    default:
        throw Error::IllegalToken(*match);
    }
}


void
Scanner::matchWhiteSpaceToken(Token *match)
{
    match->value += readChar();
    int c = peekChar(1);

    while (std::isspace(c)) {
        match->value += readChar();
        c = peekChar(1);
    }

    match->type = TokenType::WhiteSpace;
    return;
}


void
Scanner::matchCommentToken(Token *match)
{
    match->value += readChar();
    int c = peekChar(1);

    if (c == '*') {
        match->value += readChar();
        c = peekChar(1);

        for (;;) {
            if (c == '*') {
                match->value += readChar();
                c = peekChar(1);

                if (c == '/') {
                    match->value += readChar();
                    match->type = TokenType::Comment;
                    return;
                }
            } else {
                if (c < 0) {
                    throw Error::IllegalToken(*match);
                }

                match->value += readChar();
                c = peekChar(1);
            }
        }
    } else {
        match->value += readChar();
        c = peekChar(1);

        while (c >= 0 && c != '\n') {
            match->value += readChar();
            c = peekChar(1);
        }

        match->type = TokenType::Comment;
        return;
    }
}


void
Scanner::matchNumberLiteralToken(Token *match)
{
    if (peekChar(1) == '0' && peekChar(2) == 'x') {
        matchNumberLiteralToken16(match);
        return;
    } else {
        matchNumberLiteralToken10(match);
        return;
    }
}


void
Scanner::matchNumberLiteralToken10(Token *match)
{
    int c = peekChar(1);

    while (std::isdigit(c)) {
        match->value += readChar();
        c = peekChar(1);
    }

    bool floatingPointFlag = false;

    if (c == '.') {
        floatingPointFlag = true;
        match->value += readChar();
        c = peekChar(1);

        while (std::isdigit(c)) {
            match->value += readChar();
            c = peekChar(1);
        }
    }

    if (c == 'E' || c == 'e') {
        floatingPointFlag = true;
        match->value += readChar();
        c = peekChar(1);

        if (c == '+' || c == '-') {
            match->value += readChar();
            c = peekChar(1);
        }

        if (!std::isdigit(c)) {
            if (c >= 0 && c != '\n') {
                match->value += readChar();
            }

            throw Error::IllegalToken(*match);
        }

        match->value += readChar();
        c = peekChar(1);

        while (std::isdigit(c)) {
            match->value += readChar();
            c = peekChar(1);
        }
    }

    if (std::isalpha(c) || c == '_') {
        match->value += readChar();
        throw Error::IllegalToken(*match);
    }

    match->type = floatingPointFlag ? TokenType::FloatingPointLiteral : TokenType::IntegerLiteral;
    return;
}


void
Scanner::matchNumberLiteralToken16(Token *match)
{
    match->value += readChar();
    match->value += readChar();
    int c = peekChar(1);

    if (!std::isxdigit(c)) {
        if (c >= 0 && c != '\n') {
            match->value += readChar();
        }

        throw Error::IllegalToken(*match);
    }

    match->value += readChar();
    c = peekChar(1);

    while (std::isxdigit(c)) {
        match->value += readChar();
        c = peekChar(1);
    }

    if (std::isalpha(c) || c == '_') {
        match->value += readChar();
        throw Error::IllegalToken(*match);
    }

    match->type = TokenType::IntegerLiteral;
    return;
}


void
Scanner::matchStringLiteralToken(Token *match)
{
    match->value += readChar();
    int c = peekChar(1);

    for (;;) {
        if (c == '\"') {
            match->value += readChar();
            match->type = TokenType::StringLiteral;
            return;
        } else {
            if (c == '\\') {
                if (!matchEscapeChar(&match->value)) {
                    throw Error::IllegalToken(*match);
                }
            } else {
                if (c < 0 || c == '\n') {
                    throw Error::IllegalToken(*match);
                }

                match->value += readChar();
            }

            c = peekChar(1);
        }
    }
}


void
Scanner::matchNameToken(Token *match)
{
    match->value += readChar();
    int c = peekChar(1);

    while (std::isalnum(c) || c == '_') {
        match->value += readChar();
        c = peekChar(1);
    }

    std::unordered_map<std::string, TokenType>::const_iterator it = KeywordToTokenType
                                                                    .find(match->value);
    match->type = it == KeywordToTokenType.end() ? TokenType::Identifier : it->second;
    return;
}


bool
Scanner::matchEscapeChar(std::string *match)
{
    *match += readChar();
    int c = peekChar(1);

    switch (c) {
    case '\"':
    case '\'':
    case '\?':
    case '\\':
    case 'a':
    case 'b':
    case 'f':
    case 'n':
    case 'r':
    case 't':
    case 'v':
        *match += readChar();
        return true;

    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
        *match += readChar();
        c = peekChar(1);

        if (isodigit(c)) {
            *match += readChar();
            c = peekChar(1);

            if (isodigit(c)) {
                *match += readChar();
            }
        }

        return true;

    case 'x':
        *match += readChar();
        c = peekChar(1);

        if (!std::isxdigit(c)) {
            break;
        }

        *match += readChar();
        c = peekChar(1);

        if (std::isxdigit(c)) {
            *match += readChar();
        }

        return true;
    }

    if (c >= 0 && c != '\n') {
        *match += readChar();
    }

    return false;
}


namespace {

bool
isodigit(int c)
{
    return c >= '0' && c <= '7';
}

} // namespace

} // namespace OYC

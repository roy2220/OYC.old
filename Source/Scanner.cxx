#include "Scanner.h"

#include <cctype>
#include <cstring>
#include <iterator>
#include <type_traits>
#include <unordered_map>

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
        break;

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
        } else {
            match->type = MakeTokenType(c1);
        }

        break;

    case '\"':
        matchStringLiteralToken(match);
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
        match->value += readChar();
        match->type = MakeTokenType(c1);
        break;

    case '+':
    case '-':
        match->value += readChar();
        c2 = peekChar(1);

        if (c2 == c1 || c2 == '=') {
            match->value += readChar();
            match->type = MakeTokenType(c1, c2);
        } else {
            match->type = MakeTokenType(c1);
        }

        break;

    case '.':
        c2 = peekChar(2);

        if (std::isdigit(c2)) {
            matchNumberLiteralToken(match);
        } else {
            match->value += readChar();

            if (c2 == '.' && (c3 = peekChar(2)) == '.') {
                match->value += readChar();
                match->value += readChar();
                match->type = MakeTokenType(c1, c2, c3);
            } else {
                match->type = MakeTokenType(c1);
            }
        }

        break;

    case '/':
        c2 = peekChar(2);

        if (c2 == '*' || c2 == '/') {
            matchCommentToken(match);
        } else {
            match->value += readChar();

            if (c2 == '=') {
                match->value += readChar();
                match->type = MakeTokenType(c1, c2);
            } else {
                match->type = MakeTokenType(c1);
            }
        }

        break;

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
            } else {
                match->type = MakeTokenType(c1, c1);
            }
        } else if (c2 == '=') {
            match->value += readChar();
            match->type = MakeTokenType(c1, c2);
        } else {
            match->type = MakeTokenType(c1);
        }

        break;

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
        break;

    case 'A':
    case 'B':
    case 'C':
    case 'D':
    case 'E':
    case 'F':
    case 'G':
    case 'H':
    case 'I':
    case 'J':
    case 'K':
    case 'L':
    case 'M':
    case 'N':
    case 'O':
    case 'P':
    case 'Q':
    case 'R':
    case 'S':
    case 'T':
    case 'U':
    case 'V':
    case 'W':
    case 'X':
    case 'Y':
    case 'Z':
    case '_':
    case 'a':
    case 'b':
    case 'c':
    case 'd':
    case 'e':
    case 'f':
    case 'g':
    case 'h':
    case 'i':
    case 'j':
    case 'k':
    case 'l':
    case 'm':
    case 'n':
    case 'o':
    case 'p':
    case 'q':
    case 'r':
    case 's':
    case 't':
    case 'u':
    case 'v':
    case 'w':
    case 'x':
    case 'y':
    case 'z':
        matchNameToken(match);
        break;

    default:
        match->value += readChar();
        match->type = TokenType::Illegal;
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
                    match->type = TokenType::Illegal;
                    return;
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
    }
}


void
Scanner::matchNumberLiteralToken(Token *match)
{
    if (peekChar(1) == '0' && peekChar(2) == 'x') {
        matchNumberLiteralToken16(match);
    } else {
        matchNumberLiteralToken10(match);
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

            match->type = TokenType::Illegal;
            return;
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
        match->type = TokenType::Illegal;
        return;
    }

    match->type = floatingPointFlag ? TokenType::FloatingPointLiteral : TokenType::IntegerLiteral;
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

        match->type = TokenType::Illegal;
        return;
    }

    match->value += readChar();
    c = peekChar(1);

    while (std::isxdigit(c)) {
        match->value += readChar();
        c = peekChar(1);
    }

    if (std::isalpha(c) || c == '_') {
        match->value += readChar();
        match->type = TokenType::Illegal;
        return;
    }

    match->type = TokenType::IntegerLiteral;
}


void
Scanner::matchStringLiteralToken(Token *match)
{
    match->value += readChar();
    int c = peekChar(1);

    for (;;) {
        if (c == '\\') {
            match->value += readChar();
            c = peekChar(1);
            bool missFlag = false;

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
                match->value += readChar();
                c = peekChar(1);
                break;

            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
                match->value += readChar();
                c = peekChar(1);

                if (isodigit(c)) {
                    match->value += readChar();
                    c = peekChar(1);

                    if (isodigit(c)) {
                        match->value += readChar();
                        c = peekChar(1);
                    }
                }

                break;

            case 'x':
                match->value += readChar();
                c = peekChar(1);

                if (!std::isxdigit(c)) {
                    missFlag = true;
                    break;
                }

                match->value += readChar();
                c = peekChar(1);

                if (std::isxdigit(c)) {
                    match->value += readChar();
                    c = peekChar(1);
                }

                break;

            default:
                missFlag = true;
            }

            if (missFlag) {
                if (c >= 0 && c != '\n') {
                    match->value += readChar();
                }

                match->type = TokenType::Illegal;
                return;
            }
        } else {
            if (c == '\"') {
                match->value += readChar();
                match->type = TokenType::StringLiteral;
                return;
            } else {
                if (c < 0 || c == '\n') {
                    match->type = TokenType::Illegal;
                    return;
                }

                match->value += readChar();
                c = peekChar(1);
            }
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
}


namespace {

bool
isodigit(int c)
{
    return c >= '0' && c <= '7';
}

} // namespace

} // namespace OYC

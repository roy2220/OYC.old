#pragma once


#include <functional>
#include <list>
#include <utility>


namespace OYC {

struct Token;


class Scanner final
{
    Scanner(Scanner &) = delete;
    Scanner &operator=(Scanner &) = delete;

public:
    inline explicit Scanner(const std::function<int ()> &);
    inline explicit Scanner(std::function<int ()> &&);

    Token readToken();

private:
    std::function<int ()> charReader_;
    std::list<int> prereadChars_;
    int lineNumber_ = 1;
    int columnNumber_ = 1;

    int doReadChar();
    int peekChar(int);
    int readChar();

    void matchToken(Token *);
    void matchWhiteSpaceToken(Token *);
    void matchCommentToken(Token *);
    void matchNumberLiteralToken(Token *);
    void matchNumberLiteralToken10(Token *);
    void matchNumberLiteralToken16(Token *);
    void matchStringLiteralToken(Token *);
    void matchNameToken(Token *);
};


Scanner::Scanner(const std::function<int ()> &charReader)
    : charReader_(charReader)
{
}


Scanner::Scanner(std::function<int ()> &&charReader)
    : charReader_(std::move(charReader))
{
}

} // namespace OYC

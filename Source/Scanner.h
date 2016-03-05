#pragma once


#include <functional>
#include <list>
#include <utility>


namespace OYC {

struct Token;


class Scanner final
{
    Scanner(const Scanner &) = delete;
    Scanner &operator=(const Scanner &) = delete;

public:
    inline explicit Scanner();

    inline void setInput(const std::function<int ()> &);
    inline void setInput(std::function<int ()> &&);

    Token readToken();

private:
    std::function<int ()> input_;
    std::list<int> prereadChars_;
    int lineNumber_;
    int columnNumber_;

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


Scanner::Scanner()
  : input_([] () -> int {
        return -1;
    }),
    lineNumber_(1),
    columnNumber_(1)
{
}


void
Scanner::setInput(const std::function<int ()> &input)
{
    input_ = input;
}


void
Scanner::setInput(std::function<int ()> &&input)
{
    input_ = std::move(input);
}

} // namespace OYC

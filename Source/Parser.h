#pragma once


#include <functional>
#include <memory>
#include <list>
#include <string>
#include <utility>

#include "Token.h"


namespace OYC {

struct Program;
struct ProgramData;
struct Expression;
struct ArrayLiteral;
struct DictionaryLiteral;
struct FunctionLiteral;
struct ArrayInitializer;
struct DictionaryInitializer;


class Parser final
{
    Parser(Parser &) = delete;
    Parser &operator=(Parser &) = delete;

public:
    inline explicit Parser(const std::function<Token ()> &);
    inline explicit Parser(std::function<Token ()> &&);

    Program readProgram();

private:
    inline std::unique_ptr<Expression> matchExpression();

    std::function<Token ()> tokenReader_;
    std::list<Token> prereadTokens_;

    ProgramData *programData_;

    Token doReadToken();
    const Token &peekToken(int);
    Token readToken();

    void matchBody(std::vector<std::unique_ptr<Statement>> *, TokenType);

    std::unique_ptr<Statement> matchStatement();
    std::unique_ptr<Statement> matchAutoStatement();
    std::unique_ptr<Statement> matchBreakStatement();
    std::unique_ptr<Statement> matchContinueStatement();
    std::unique_ptr<Statement> matchReturnStatement();
    std::unique_ptr<Statement> matchExpressionStatement();
    std::unique_ptr<Statement> matchIfStatement();
    std::unique_ptr<Statement> matchSwitchStatement();
    std::unique_ptr<Statement> matchWhileStatement();
    std::unique_ptr<Statement> matchDoWhileStatement();
    std::unique_ptr<Statement> matchForStatement();
    std::unique_ptr<Statement> matchForeachStatement();

    std::unique_ptr<Expression> matchExpression1();
    std::unique_ptr<Expression> matchExpression2();
    std::unique_ptr<Expression> matchExpression3(int);
    std::unique_ptr<Expression> matchExpression4();
    std::unique_ptr<Expression> matchExpression5();
    std::unique_ptr<Expression> matchExpression6();
    std::unique_ptr<Expression> matchExpression7();

    bool getBoolean();
    unsigned long getInteger();
    double getFloatingPoint();
    const std::string *getString();
    const std::string *getIdentifier();

    const ArrayLiteral *matchArrayLiteral();
    const DictionaryLiteral *matchDictionaryLiteral();
    const FunctionLiteral *matchFunctionLiteral();
};


Parser::Parser(const std::function<Token ()> &tokenReader)
    : tokenReader_(tokenReader)
{
}


Parser::Parser(std::function<Token ()> &&tokenReader)
    : tokenReader_(std::move(tokenReader))
{
}


std::unique_ptr<Expression>
Parser::matchExpression()
{
    return matchExpression1();
}

} // namespace OYC

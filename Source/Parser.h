#pragma once


#include <functional>
#include <list>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "Token.h"


namespace OYC {

struct Program;
struct ProgramData;
struct Statement;
struct VariableDeclarator;
struct CaseClause;
struct Expression;
struct ArrayLiteral;
struct DictionaryLiteral;
struct FunctionLiteral;


class Parser final
{
    Parser(Parser &) = delete;
    Parser &operator=(Parser &) = delete;

public:
    inline explicit Parser();

    inline void setInput(const std::function<Token ()> &);
    inline void setInput(std::function<Token ()> &&);

    Program readProgram();

private:
    std::function<Token ()> input_;
    std::list<Token> prereadTokens_;

    ProgramData *programData_;

    Token doReadToken();
    const Token &peekToken(int);
    Token readToken();

    void matchProgram(Program *);
    void matchStatements(TokenType, std::vector<std::unique_ptr<Statement>> *);

    std::unique_ptr<Statement> matchStatement();
    std::unique_ptr<Statement> matchExpressionStatement();
    std::unique_ptr<Statement> matchAutoStatement();
    std::unique_ptr<Statement> matchBreakStatement();
    std::unique_ptr<Statement> matchContinueStatement();
    std::unique_ptr<Statement> matchReturnStatement();
    std::unique_ptr<Statement> matchIfStatement();
    std::unique_ptr<Statement> matchSwitchStatement();
    std::unique_ptr<Statement> matchWhileStatement();
    std::unique_ptr<Statement> matchDoWhileStatement();
    std::unique_ptr<Statement> matchForStatement();
    std::unique_ptr<Statement> matchForeachStatement();

    void matchVariableDeclarator(VariableDeclarator *);
    void matchCaseClause(CaseClause *);

    std::unique_ptr<Expression> matchExpression1();
    std::unique_ptr<Expression> matchExpression2();
    std::unique_ptr<Expression> matchExpression3(int *);
    std::unique_ptr<Expression> matchExpression4();
    std::unique_ptr<Expression> matchExpression5();
    std::unique_ptr<Expression> matchExpression6();
    std::unique_ptr<Expression> matchElementSelector();

    bool getBoolean();
    unsigned long getInteger();
    double getFloatingPoint();
    const std::string *getString();
    const std::string *getIdentifier();

    const ArrayLiteral *matchArrayLiteral();
    const DictionaryLiteral *matchDictionaryLiteral();
    const FunctionLiteral *matchFunctionLiteral();

    std::unique_ptr<Expression> matchArrayElement();
    std::pair<std::unique_ptr<Expression>, std::unique_ptr<Expression>> matchDictionaryElement();
};


Parser::Parser()
    : input_([] () -> Token { return {TokenType::EndOfFile, {}, 1, 1}; }), programData_(nullptr)
{
}


void
Parser::setInput(const std::function<Token ()> &input)
{
    input_ = input;
}


void
Parser::setInput(std::function<Token ()> &&input)
{
    input_ = std::move(input);
}

} // namespace OYC

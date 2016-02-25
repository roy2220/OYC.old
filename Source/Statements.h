#pragma once


#include <string>
#include <memory>
#include <vector>

#include "StatementVisitor.h"


namespace OYC {

struct VariableDeclarator
{
    const std::string *name;
    std::unique_ptr<Expression> initializer;
};


struct AutoStatement: Statement
{
    std::vector<VariableDeclarator> variableDeclarators;

    OYC_STATEMENT_VISIT_ACCEPTOR(AutoStatement)
};


struct BreakStatement: Statement
{
    OYC_STATEMENT_VISIT_ACCEPTOR(BreakStatement)
};


struct ContinueStatement: Statement
{
    OYC_STATEMENT_VISIT_ACCEPTOR(ContinueStatement)
};


struct ReturnStatement: Statement
{
    std::unique_ptr<Expression> result;

    OYC_STATEMENT_VISIT_ACCEPTOR(ReturnStatement)
};


struct ExpressionStatement: Statement
{
    std::unique_ptr<Expression> expression;

    OYC_STATEMENT_VISIT_ACCEPTOR(ExpressionStatement)
};


struct IfStatement: Statement
{
    std::unique_ptr<Expression> condition;
    std::vector<std::unique_ptr<Statement>> thenBody;
    std::vector<std::unique_ptr<Statement>> elseBody;

    OYC_STATEMENT_VISIT_ACCEPTOR(IfStatement)
};


struct WhileStatement: Statement
{
    std::unique_ptr<Expression> condition;
    std::vector<std::unique_ptr<Statement>> body;

    OYC_STATEMENT_VISIT_ACCEPTOR(WhileStatement)
};


struct DoWhileStatement: Statement
{
    std::unique_ptr<Expression> condition;
    std::vector<std::unique_ptr<Statement>> body;

    OYC_STATEMENT_VISIT_ACCEPTOR(DoWhileStatement)
};


struct ForStatement: Statement
{
    std::unique_ptr<Statement> initialization;
    std::unique_ptr<Expression> condition;
    std::unique_ptr<Expression> iteration;
    std::vector<std::unique_ptr<Statement>> body;

    OYC_STATEMENT_VISIT_ACCEPTOR(ForStatement)
};


struct ForeachStatement: Statement
{
    const std::string *variableName1;
    const std::string *variableName2;
    std::unique_ptr<Expression> collection;

    OYC_STATEMENT_VISIT_ACCEPTOR(ForeachStatement)
};

} // namespace OYC

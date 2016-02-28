#pragma once


#include <memory>
#include <string>
#include <vector>


namespace OYC {

struct Expression;

class StatementVisitor;


struct Statement
{
    int lineNumber = 0;
    int columnNumber = 0;

    virtual ~Statement() = default;

    virtual void acceptVisit(StatementVisitor *) const = 0;
};


struct ExpressionStatement : Statement
{
    std::unique_ptr<Expression> expression;

    void acceptVisit(StatementVisitor *) const override;
};


struct VariableDeclarator
{
    const std::string *name;
    std::unique_ptr<Expression> initializer;
};


struct AutoStatement : Statement
{
    std::vector<VariableDeclarator> variableDeclarators;

    void acceptVisit(StatementVisitor *) const override;
};


struct BreakStatement : Statement
{
    void acceptVisit(StatementVisitor *) const override;
};


struct ContinueStatement : Statement
{
    void acceptVisit(StatementVisitor *) const override;
};


struct ReturnStatement : Statement
{
    std::unique_ptr<Expression> result;

    void acceptVisit(StatementVisitor *) const override;
};


struct IfStatement : Statement
{
    std::unique_ptr<Expression> condition;
    std::vector<std::unique_ptr<Statement>> thenBody;
    std::vector<std::unique_ptr<Statement>> elseBody;

    void acceptVisit(StatementVisitor *) const override;
};


struct WhileStatement : Statement
{
    std::unique_ptr<Expression> condition;
    std::vector<std::unique_ptr<Statement>> body;

    void acceptVisit(StatementVisitor *) const override;
};


struct DoWhileStatement : Statement
{
    std::unique_ptr<Expression> condition;
    std::vector<std::unique_ptr<Statement>> body;

    void acceptVisit(StatementVisitor *) const override;
};


struct ForStatement : Statement
{
    std::unique_ptr<Statement> initialization;
    std::unique_ptr<Expression> condition;
    std::unique_ptr<Expression> iteration;
    std::vector<std::unique_ptr<Statement>> body;

    void acceptVisit(StatementVisitor *) const override;
};


struct ForeachStatement : Statement
{
    const std::string *variableName1;
    const std::string *variableName2;
    std::unique_ptr<Expression> collection;
    std::vector<std::unique_ptr<Statement>> body;

    void acceptVisit(StatementVisitor *) const override;
};

} // namespace OYC

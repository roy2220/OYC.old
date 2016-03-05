#pragma once


namespace OYC {

struct ExpressionStatement;
struct AutoStatement;
struct BreakStatement;
struct ContinueStatement;
struct ReturnStatement;
struct IfStatement;
struct SwitchStatement;
struct WhileStatement;
struct DoWhileStatement;
struct ForStatement;
struct ForeachStatement;


class StatementVisitor
{
    StatementVisitor(const StatementVisitor &) = delete;
    StatementVisitor &operator=(const StatementVisitor &) = delete;

public:
    virtual void visitExpressionStatement(const ExpressionStatement &) = 0;
    virtual void visitAutoStatement(const AutoStatement &) = 0;
    virtual void visitBreakStatement(const BreakStatement &) = 0;
    virtual void visitContinueStatement(const ContinueStatement &) = 0;
    virtual void visitReturnStatement(const ReturnStatement &) = 0;
    virtual void visitIfStatement(const IfStatement &) = 0;
    virtual void visitSwitchStatement(const SwitchStatement &) = 0;
    virtual void visitWhileStatement(const WhileStatement &) = 0;
    virtual void visitDoWhileStatement(const DoWhileStatement &) = 0;
    virtual void visitForStatement(const ForStatement &) = 0;
    virtual void visitForeachStatement(const ForeachStatement &) = 0;

protected:
    explicit StatementVisitor() = default;
    ~StatementVisitor() = default;
};

} // namespace OYC

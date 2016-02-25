#pragma once


#define OYC_STATEMENT_VISIT_ACCEPTOR(statement)                \
private:                                                       \
    void acceptVisit(StatementVisitor *visitor) const override \
    {                                                          \
        visitor->visit##statement(*this);                      \
    }


namespace OYC {

class StatementVisitor;


struct Statement
{
    virtual ~Statement() = default;

    int lineNumber = 0;
    int columnNumber = 0;

private:
    virtual void acceptVisitor(StatementVisitor *) const = 0;

    friend StatementVisitor;
};


class StatementVisitor
{
    StatementVisitor(StatementVisitor &) = delete;
    StatementVisitor &operator=(StatementVisitor &) = delete;

public:
    inline void visitStatement(const Statement &);

protected:
    explicit StatementVisitor() = default;
    ~StatementVisitor() = default;
};


void
StatementVisitor::visitStatement(const Statement &visitee)
{
    visitee.acceptVisit(this);
}

} // namespace OYC

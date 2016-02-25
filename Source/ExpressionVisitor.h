#pragma once


#define OYC_EXPRESSION_VISIT_ACCEPTOR(expression)               \
private:                                                        \
    void acceptVisit(ExpressionVisitor *visitor) const override \
    {                                                           \
        visitor->visit##expression(*this);                      \
    }


namespace OYC {

class ExpressionVisitor;

struct PrimaryExpression;
struct UnaryExpression;
struct BinaryExpression;
struct TernaryExpression;
struct RetrievalExpression;
struct InvocationExpression;


struct Expression
{
    virtual ~Expression() = default;

private:
    virtual void acceptVisitor(ExpressionVisitor *) const = 0;

    friend ExpressionVisitor;
};


class ExpressionVisitor
{
    ExpressionVisitor(ExpressionVisitor &) = delete;
    ExpressionVisitor &operator=(ExpressionVisitor &) = delete;

public:
    inline void visitExpression(const Expression &);

    virtual void visitPrimaryExpression(const PrimaryExpression &) = 0;
    virtual void visitUnaryExpression(const UnaryExpression &) = 0;
    virtual void visitBinaryExpression(const BinaryExpression &) = 0;
    virtual void visitTernaryExpression(const TernaryExpression &) = 0;
    virtual void visitRetrievalExpression(const RetrievalExpression &) = 0;
    virtual void visitInvocationExpression(const InvocationExpression &) = 0;

protected:
    explicit ExpressionVisitor() = default;
    ~ExpressionVisitor() = default;
};


void
ExpressionVisitor::visitExpression(const Expression &visitee)
{
    visitee.acceptVisit(this);
}

} // namespace OYC

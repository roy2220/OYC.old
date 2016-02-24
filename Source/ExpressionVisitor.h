#pragma once


#define OYC_EXPRESSION_VISITOR_ACCEPTOR                                               \
    void acceptVisitor(ExpressionVisitor *visitor) const override                     \
    {                                                                                 \
        static_cast<void>(static_cast<void (ExpressionVisitor::*)(decltype(*this) &)> \
                                     (&ExpressionVisitor::visitExpression));          \
        visitor->visitExpression(*this);                                              \
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

    virtual void acceptVisitor(ExpressionVisitor *visitor) const = 0;
};


class ExpressionVisitor
{
    ExpressionVisitor(ExpressionVisitor &) = delete;
    ExpressionVisitor &operator=(ExpressionVisitor &) = delete;

public:
    inline void visitExpression(const Expression &);

    virtual void visitExpression(const PrimaryExpression &) = 0;
    virtual void visitExpression(const UnaryExpression &) = 0;
    virtual void visitExpression(const BinaryExpression &) = 0;
    virtual void visitExpression(const TernaryExpression &) = 0;
    virtual void visitExpression(const RetrievalExpression &) = 0;
    virtual void visitExpression(const InvocationExpression &) = 0;

protected:
    explicit ExpressionVisitor() = default;
    ~ExpressionVisitor() = default;
};


void
ExpressionVisitor::visitExpression(const Expression &expression)
{
    expression.acceptVisitor(this);
}

} // namespace OYC

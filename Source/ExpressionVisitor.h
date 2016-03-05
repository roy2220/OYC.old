#pragma once


namespace OYC {

struct PrimaryExpression;
struct UnaryExpression;
struct BinaryExpression;
struct TernaryExpression;
struct RetrievalExpression;
struct InvocationExpression;


class ExpressionVisitor
{
    ExpressionVisitor(const ExpressionVisitor &) = delete;
    ExpressionVisitor &operator=(const ExpressionVisitor &) = delete;

public:
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

} // namespace OYC

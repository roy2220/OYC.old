#pragma once


#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "Token.h"


namespace OYC {

struct ArrayLiteral;
struct DictionaryLiteral;
struct FunctionLiteral;

class ExpressionVisitor;


struct Expression
{
    virtual ~Expression() = default;

    virtual void acceptVisit(ExpressionVisitor *) const = 0;
};


enum class PrimaryExpressionType : std::uint8_t
{
    No = 0,
    Null,
    Boolean,
    Integer,
    FloatingPoint,
    String,
    Identifier,
    ArrayLiteral,
    DictionaryLiteral,
    FunctionLiteral
};


struct PrimaryExpression : Expression
{
    PrimaryExpressionType type = PrimaryExpressionType::No;

    union {
        bool boolean;
        unsigned long integer;
        double floatingPoint;
        const std::string *string;
        const std::string *identifier;
        const ArrayLiteral *arrayLiteral;
        const DictionaryLiteral *dictionaryLiteral;
        const FunctionLiteral *functionLiteral;
    };

    void acceptVisit(ExpressionVisitor *) const override;
};


enum class UnaryExpressionType : std::uint8_t
{
    No = 0,
    Prefix,
    Postfix
};


struct UnaryExpression : Expression
{
    UnaryExpressionType type = UnaryExpressionType::No;
    TokenType op = TokenType::No;
    std::unique_ptr<Expression> operand;

    void acceptVisit(ExpressionVisitor *) const override;
};


struct BinaryExpression : Expression
{
    TokenType op = TokenType::No;
    std::unique_ptr<Expression> operand1;
    std::unique_ptr<Expression> operand2;

    void acceptVisit(ExpressionVisitor *) const override;
};


struct TernaryExpression : Expression
{
    TokenType op[2] = {TokenType::No, TokenType::No};
    std::unique_ptr<Expression> operand1;
    std::unique_ptr<Expression> operand2;
    std::unique_ptr<Expression> operand3;

    void acceptVisit(ExpressionVisitor *) const override;
};


struct RetrievalExpression : Expression
{
    std::unique_ptr<Expression> retrievee;
    std::unique_ptr<Expression> key;

    void acceptVisit(ExpressionVisitor *) const override;
};


struct InvocationExpression : Expression
{
    std::unique_ptr<Expression> invokee;
    std::vector<std::unique_ptr<Expression>> arguments;

    void acceptVisit(ExpressionVisitor *) const override;
};

} // namespace OYC

#pragma once


#include <cstdint>
#include <memory>
#include <vector>
#include <utility>
#include <string>

#include "ExpressionVisitor.h"
#include "Token.h"


namespace OYC {

enum class PrimaryExpressionType: std::uint8_t
{
    No = 0,
    Nil,
    Boolean,
    Integer,
    FloatingPoint,
    String,
    Identifier,
    ArrayLiteral,
    DictionaryLiteral,
    FunctionLiteral
};


struct ArrayLiteral
{
    std::vector<std::unique_ptr<Expression>> elements;
};


struct DictionaryLiteral
{
    std::vector<std::pair<std::unique_ptr<Expression>, std::unique_ptr<Expression>>> elements;
};


struct FunctionLiteral
{
    std::vector<const std::string *> parameters;
};


struct PrimaryExpression: Expression
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

    OYC_EXPRESSION_VISIT_ACCEPTOR(PrimaryExpression)
};


enum class UnaryExpressionType: std::uint8_t
{
    No = 0,
    Prefix,
    Postfix
};


struct UnaryExpression: Expression
{
    UnaryExpressionType type = UnaryExpressionType::No;
    TokenType op = TokenType::No;
    std::unique_ptr<Expression> operand;

    OYC_EXPRESSION_VISIT_ACCEPTOR(UnaryExpression)
};


struct BinaryExpression: Expression
{
    TokenType op = TokenType::No;
    std::unique_ptr<Expression> operand1;
    std::unique_ptr<Expression> operand2;

    OYC_EXPRESSION_VISIT_ACCEPTOR(BinaryExpression)
};


struct TernaryExpression: Expression
{
    TokenType op[2] = {TokenType::No, TokenType::No};
    std::unique_ptr<Expression> operand1;
    std::unique_ptr<Expression> operand2;
    std::unique_ptr<Expression> operand3;

    OYC_EXPRESSION_VISIT_ACCEPTOR(TernaryExpression)
};


struct RetrievalExpression: Expression
{
    std::unique_ptr<Expression> retrievee;
    std::unique_ptr<Expression> key;

    OYC_EXPRESSION_VISIT_ACCEPTOR(RetrievalExpression)
};


struct InvocationExpression: Expression
{
    std::unique_ptr<Expression> invokee;
    std::vector<std::unique_ptr<Expression>> arguments;

    OYC_EXPRESSION_VISIT_ACCEPTOR(InvocationExpression)
};

} // namespace OYC

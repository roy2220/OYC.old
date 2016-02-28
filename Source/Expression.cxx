#include "Expression.h"
#include "ExpressionVisitor.h"


namespace OYC {

#define EXPRESSION_VISIT_ACCEPTOR(expressionType)                 \
    void                                                          \
    expressionType::acceptVisit(ExpressionVisitor *visitor) const \
    {                                                             \
        visitor->visit##expressionType(*this);                    \
    }


EXPRESSION_VISIT_ACCEPTOR(PrimaryExpression)
EXPRESSION_VISIT_ACCEPTOR(UnaryExpression)
EXPRESSION_VISIT_ACCEPTOR(BinaryExpression)
EXPRESSION_VISIT_ACCEPTOR(TernaryExpression)
EXPRESSION_VISIT_ACCEPTOR(RetrievalExpression)
EXPRESSION_VISIT_ACCEPTOR(InvocationExpression)


#undef EXPRESSION_VISIT_ACCEPTOR

} // namespace OYC

#include "Expression.h"
#include "Statement.h"
#include "StatementVisitor.h"


namespace OYC {

#define STATEMENT_VISIT_ACCEPTOR(statementType)                 \
    void                                                        \
    statementType::acceptVisit(StatementVisitor *visitor) const \
    {                                                           \
        visitor->visit##statementType(*this);                   \
    }


STATEMENT_VISIT_ACCEPTOR(ExpressionStatement)
STATEMENT_VISIT_ACCEPTOR(AutoStatement)
STATEMENT_VISIT_ACCEPTOR(BreakStatement)
STATEMENT_VISIT_ACCEPTOR(ContinueStatement)
STATEMENT_VISIT_ACCEPTOR(ReturnStatement)
STATEMENT_VISIT_ACCEPTOR(IfStatement)
STATEMENT_VISIT_ACCEPTOR(SwitchStatement)
STATEMENT_VISIT_ACCEPTOR(WhileStatement)
STATEMENT_VISIT_ACCEPTOR(DoWhileStatement)
STATEMENT_VISIT_ACCEPTOR(ForStatement)
STATEMENT_VISIT_ACCEPTOR(ForeachStatement)


#undef STATEMENT_VISIT_ACCEPTOR

} // namespace OYC


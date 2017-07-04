#include "Declarations.hxx"

#include "src/ASTVisitors/IAstVisitor.hxx"

OperatorExpression::~OperatorExpression() = default;

void OperatorExpression::accept(IAstVisitor &visitor)
{
    visitor.visit(shared_from_this());
}

#include "Declarations.hxx"

#include "src/ASTVisitors/IAstVisitor.hxx"

LiteralExpression::~LiteralExpression() = default;

void LiteralExpression::accept(IAstVisitor &visitor)
{
    visitor.visit(shared_from_this());
}

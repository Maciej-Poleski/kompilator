#include "Declarations.hxx"

#include "src/ASTVisitors/IAstVisitor.hxx"

TermDefinitionExpression::~TermDefinitionExpression() = default;

void TermDefinitionExpression::accept(IAstVisitor &visitor)
{
    visitor.visit(shared_from_this());
}

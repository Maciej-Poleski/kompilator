#include "Declarations.hxx"

#include <ostream>

#include "src/ASTVisitors/IAstVisitor.hxx"

void DereferenceExpression::prettyPrint(int indentation, std::ostream &out) const
{
    out << "*";
    expression->prettyPrint(indentation, out);
}

void DereferenceExpression::accept(IAstVisitor &visitor)
{
    visitor.visit(shared_from_this());
}

bool DereferenceExpression::isLvalue() const
{
    return true;
}

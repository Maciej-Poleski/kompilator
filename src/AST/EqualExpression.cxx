#include "Declarations.hxx"

#include <ostream>

#include "src/ASTVisitors/IAstVisitor.hxx"

void EqualExpression::prettyPrint(int indentation, std::ostream &out) const
{
    leftExpression->prettyPrint(indentation, out);
    out << " == ";
    rightExpression->prettyPrint(indentation, out);
}

void EqualExpression::accept(IAstVisitor &visitor)
{
    visitor.visit(shared_from_this());
}

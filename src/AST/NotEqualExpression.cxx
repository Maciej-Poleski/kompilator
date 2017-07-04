#include "Declarations.hxx"

#include <ostream>

#include "src/ASTVisitors/IAstVisitor.hxx"

void NotEqualExpression::prettyPrint(int indentation, std::ostream &out) const
{
    leftExpression->prettyPrint(indentation, out);
    out << " != ";
    rightExpression->prettyPrint(indentation, out);
}

void NotEqualExpression::accept(IAstVisitor &visitor)
{
    visitor.visit(shared_from_this());
}

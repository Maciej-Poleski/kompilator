#include "Declarations.hxx"

#include <ostream>

#include "src/ASTVisitors/IAstVisitor.hxx"

void OrExpression::prettyPrint(int indentation, std::ostream &out) const
{
    leftExpression->prettyPrint(indentation, out);
    out << " || ";
    rightExpression->prettyPrint(indentation, out);
}

void OrExpression::accept(IAstVisitor &visitor)
{
    visitor.visit(shared_from_this());
}

#include "Declarations.hxx"

#include <ostream>

#include "src/ASTVisitors/IAstVisitor.hxx"

void UnifyExpression::prettyPrint(int indentation, std::ostream &out) const
{
    leftExpression->prettyPrint(indentation, out);
    out << " === ";
    rightExpression->prettyPrint(indentation, out);
}

void UnifyExpression::accept(IAstVisitor &visitor)
{
    visitor.visit(shared_from_this());
}

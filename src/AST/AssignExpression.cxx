#include "Declarations.hxx"

#include <ostream>

#include "src/ASTVisitors/IAstVisitor.hxx"

void AssignExpression::prettyPrint(int indentation, std::ostream &out) const
{
    leftExpression->prettyPrint(indentation, out);
    out << " = ";
    rightExpression->prettyPrint(indentation, out);
}

void AssignExpression::accept(IAstVisitor &visitor)
{
    visitor.visit(shared_from_this());
}

bool AssignExpression::isLvalue() const
{
    return true;
}

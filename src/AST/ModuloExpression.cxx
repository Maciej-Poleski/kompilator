#include "Declarations.hxx"

#include <ostream>

#include "src/ASTVisitors/IAstVisitor.hxx"

void ModuloExpression::prettyPrint(int indentation, std::ostream &out) const
{
    out << "(";
    leftExpression->prettyPrint(indentation, out);
    out << " % ";
    rightExpression->prettyPrint(indentation, out);
    out << ")";
}

void ModuloExpression::accept(IAstVisitor &visitor)
{
    visitor.visit(shared_from_this());
}

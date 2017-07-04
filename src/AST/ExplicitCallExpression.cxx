#include "Declarations.hxx"

#include <ostream>

#include "src/ASTVisitors/IAstVisitor.hxx"

void ExplicitCallExpression::prettyPrint(int indentation, std::ostream &out) const
{
    out << "$";
    expression->prettyPrint(indentation, out);
}

void ExplicitCallExpression::accept(IAstVisitor &visitor)
{
    visitor.visit(shared_from_this());
}

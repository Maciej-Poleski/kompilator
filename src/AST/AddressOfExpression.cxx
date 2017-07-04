#include "Declarations.hxx"

#include <ostream>

#include "src/ASTVisitors/IAstVisitor.hxx"

void AddressOfExpression::prettyPrint(int indentation, std::ostream &out) const
{
    out << "&";
    expression->prettyPrint(indentation, out);
}

void AddressOfExpression::accept(IAstVisitor &visitor)
{
    visitor.visit(shared_from_this());
}

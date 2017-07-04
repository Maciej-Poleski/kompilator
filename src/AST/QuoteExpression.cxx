#include "Declarations.hxx"

#include <ostream>

#include "src/ASTVisitors/IAstVisitor.hxx"

void QuoteExpression::prettyPrint(int indentation, std::ostream &out) const
{
    out << "`";
    expression->prettyPrint(indentation, out);
}

void QuoteExpression::accept(IAstVisitor &visitor)
{
    visitor.visit(shared_from_this());
}

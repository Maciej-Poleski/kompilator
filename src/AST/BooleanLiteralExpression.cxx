#include "Declarations.hxx"

#include <ostream>

#include "src/ASTVisitors/IAstVisitor.hxx"

BooleanLiteralExpression::BooleanLiteralExpression(bool value)
        : value(value)
{}

BooleanLiteralExpression::~BooleanLiteralExpression() = default;

void BooleanLiteralExpression::prettyPrint(int indentation, std::ostream &out) const
{
    (void) indentation;
    out << (value ? "true" : "false");
}

void BooleanLiteralExpression::accept(IAstVisitor &visitor)
{
    visitor.visit(shared_from_this());
}

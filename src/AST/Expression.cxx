#include "Declarations.hxx"

#include "src/ASTVisitors/IAstVisitor.hxx"

Expression::~Expression() = default;

void Expression::accept(IAstVisitor &visitor)
{
    visitor.visit(shared_from_this());
}

bool Expression::isLvalue() const
{
    return false;
}

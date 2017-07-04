#include "Declarations.hxx"

#include "src/ASTVisitors/IAstVisitor.hxx"

Type::~Type() = default;

void Type::accept(IAstVisitor &visitor)
{
    visitor.visit(shared_from_this());
}

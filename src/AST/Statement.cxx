#include "Declarations.hxx"

#include "src/ASTVisitors/IAstVisitor.hxx"

Statement::~Statement() = default;

void Statement::accept(IAstVisitor &visitor)
{
    visitor.visit(shared_from_this());
}

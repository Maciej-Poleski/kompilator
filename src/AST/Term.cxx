#include "Declarations.hxx"

#include "src/ASTVisitors/IAstVisitor.hxx"

Term::~Term() = default;

void Term::accept(IAstVisitor &visitor)
{
    visitor.visit(shared_from_this());
}

#include "Declarations.hxx"

#include "src/ASTVisitors/IAstVisitor.hxx"

FunctionBody::~FunctionBody() = default;

void FunctionBody::accept(IAstVisitor &visitor)
{
    visitor.visit(shared_from_this());
}

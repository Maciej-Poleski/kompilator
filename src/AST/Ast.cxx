#include "Declarations.hxx"

#include <ostream>

#include "src/ASTVisitors/IAstVisitor.hxx"

Ast::~Ast() = default;

void Ast::dump(std::ostream &out) const
{
    prettyPrint(0, out);
}

void Ast::accept(IAstVisitor &visitor)
{
    visitor.visit(shared_from_this());
}

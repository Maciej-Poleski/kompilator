#include "Declarations.hxx"

#include "src/ASTVisitors/IAstVisitor.hxx"

ReferenceTerm::ReferenceTerm(const std::string &name) : name(name)
{}

ReferenceTerm::~ReferenceTerm() = default;

void ReferenceTerm::prettyPrint(int indentation, std::ostream &out) const
{
    (void) indentation;
    out << name;
}

void ReferenceTerm::accept(IAstVisitor &visitor)
{
    visitor.visit(shared_from_this());
}

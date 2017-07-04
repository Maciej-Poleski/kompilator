#include "Declarations.hxx"

#include "src/ASTVisitors/IAstVisitor.hxx"

NamedType::NamedType(const std::string &name) : name(name)
{}

NamedType::~NamedType() = default;

void NamedType::prettyPrint(int indentation, std::ostream &out) const
{
    (void) indentation;
    out << name;
}

void NamedType::accept(IAstVisitor &visitor)
{
    visitor.visit(shared_from_this());
}

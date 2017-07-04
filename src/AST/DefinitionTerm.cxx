#include "Declarations.hxx"

#include <ostream>

#include "src/ASTVisitors/IAstVisitor.hxx"

DefinitionTerm::DefinitionTerm(std::shared_ptr<Type> type, const std::string &name) : type(type), name(name)
{}

DefinitionTerm::~DefinitionTerm() = default;

void DefinitionTerm::prettyPrint(int indentation, std::ostream &out) const
{
    type->prettyPrint(indentation, out);
    out << ' ' << name;
}

void DefinitionTerm::accept(IAstVisitor &visitor)
{
    visitor.visit(shared_from_this());
}

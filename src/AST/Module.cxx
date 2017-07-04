#include "Declarations.hxx"

#include <ostream>

#include "src/ASTVisitors/IAstVisitor.hxx"

Module::~Module() = default;

void Module::prettyPrint(int indentation, std::ostream &out) const
{
    for (auto r : recordDefinitions) {
        r->prettyPrint(indentation, out) ;
        out<<'\n';
    }
    for (auto r : functionDefinitions) {
        r->prettyPrint(indentation, out) ;
        out<<'\n';
    }
}

void Module::accept(IAstVisitor &visitor)
{
    visitor.visit(shared_from_this());
}

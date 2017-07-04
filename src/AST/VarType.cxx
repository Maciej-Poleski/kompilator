#include "Declarations.hxx"

#include <ostream>

#include "src/ASTVisitors/IAstVisitor.hxx"

VarType::VarType()
        : BuiltinType("var")
{}

VarType::~VarType() = default;

void VarType::prettyPrint(int indentation, std::ostream &out) const
{
    (void) indentation;
    out << "(builtin type) var";
}

void VarType::accept(IAstVisitor &visitor)
{
    visitor.visit(shared_from_this());
}

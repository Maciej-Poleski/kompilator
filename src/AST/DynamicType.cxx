#include "Declarations.hxx"

#include <ostream>

#include "src/ASTVisitors/IAstVisitor.hxx"

DynamicType::DynamicType()
        : BuiltinType("dynamic")
{}

DynamicType::~DynamicType() = default;

void DynamicType::prettyPrint(int indentation, std::ostream &out) const
{
    (void) indentation;
    out << "(builtin type) dynamic";
}

void DynamicType::accept(IAstVisitor &visitor)
{
    visitor.visit(shared_from_this());
}

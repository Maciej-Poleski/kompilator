#include "Declarations.hxx"

#include <ostream>

#include "src/ASTVisitors/IAstVisitor.hxx"

BoolType::BoolType()
        : BuiltinType("bool")
{}

BoolType::~BoolType() = default;

void BoolType::prettyPrint(int indentation, std::ostream &out) const
{
    (void) indentation;
    out << "(builtin type) bool";
}

void BoolType::accept(IAstVisitor &visitor)
{
    visitor.visit(shared_from_this());
}

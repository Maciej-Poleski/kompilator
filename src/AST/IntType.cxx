#include "Declarations.hxx"

#include <ostream>

#include "src/ASTVisitors/IAstVisitor.hxx"

IntType::IntType()
        : BuiltinType("int")
{}

IntType::~IntType() = default;

void IntType::prettyPrint(int indentation, std::ostream &out) const
{
    (void) indentation;
    out << "(builtin type) int";
}

void IntType::accept(IAstVisitor &visitor)
{
    visitor.visit(shared_from_this());
}

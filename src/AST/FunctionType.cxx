#include "Declarations.hxx"

#include <ostream>

#include "src/ASTVisitors/IAstVisitor.hxx"

FunctionType::FunctionType()
        : BuiltinType("function")
{}

FunctionType::~FunctionType() = default;

void FunctionType::prettyPrint(int indentation, std::ostream &out) const
{
    (void) indentation;
    out << "// (builtin type) function";
}

void FunctionType::accept(IAstVisitor &visitor)
{
    visitor.visit(shared_from_this());
}

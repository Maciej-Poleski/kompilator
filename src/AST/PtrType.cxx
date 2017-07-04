#include "Declarations.hxx"

#include <ostream>

#include "src/ASTVisitors/IAstVisitor.hxx"

PtrType::PtrType(std::shared_ptr<Type> type) : type(type)
{}

PtrType::~PtrType() = default;

void PtrType::prettyPrint(int indentation, std::ostream &out) const
{
    out << '*';
    type->prettyPrint(indentation, out);
}

void PtrType::accept(IAstVisitor &visitor)
{
    visitor.visit(shared_from_this());
}

#include "Declarations.hxx"

#include <ostream>

#include "src/ASTVisitors/IAstVisitor.hxx"

AnnotatedType::AnnotatedType(std::shared_ptr<Type> type) : type(type)
{}

AnnotatedType::~AnnotatedType() = default;

void AnnotatedType::prettyPrint(int indentation, std::ostream &out) const
{
    out << '+';
    type->prettyPrint(indentation, out);
}

void AnnotatedType::accept(IAstVisitor &visitor)
{
    visitor.visit(shared_from_this());
}

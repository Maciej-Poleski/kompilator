#include "Declarations.hxx"

#include <ostream>

#include "src/ASTVisitors/IAstVisitor.hxx"

AtomicTermDefinitionExpression::AtomicTermDefinitionExpression(std::shared_ptr<Type> type, const std::string &name)
        : type(type), name(name)
{}

AtomicTermDefinitionExpression::~AtomicTermDefinitionExpression() = default;

void AtomicTermDefinitionExpression::prettyPrint(int indentation, std::ostream &out) const
{
    (void) indentation;
    type->prettyPrint(indentation, out);
    out << " " << name;
}

void AtomicTermDefinitionExpression::accept(IAstVisitor &visitor)
{
    visitor.visit(shared_from_this());
}

bool AtomicTermDefinitionExpression::isLvalue() const
{
    return true;
}

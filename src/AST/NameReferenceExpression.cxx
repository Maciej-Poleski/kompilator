#include "Declarations.hxx"

#include <ostream>

#include "src/ASTVisitors/IAstVisitor.hxx"

NameReferenceExpression::NameReferenceExpression(const std::string &name)
        : name(name)
{}

NameReferenceExpression::~NameReferenceExpression() = default;

void NameReferenceExpression::prettyPrint(int indentation, std::ostream &out) const
{
    (void) indentation;
    out << name;
}

void NameReferenceExpression::accept(IAstVisitor &visitor)
{
    visitor.visit(shared_from_this());
}

bool NameReferenceExpression::isLvalue() const
{
    return true;
}

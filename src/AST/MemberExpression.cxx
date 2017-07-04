#include "Declarations.hxx"

#include <ostream>

#include "src/ASTVisitors/IAstVisitor.hxx"

MemberExpression::MemberExpression(std::shared_ptr<Expression> recordExpression, const std::string &memberName)
        : UnaryOperatorExpression(recordExpression), memberName(memberName)
{}

void MemberExpression::prettyPrint(int indentation, std::ostream &out) const
{
    out << "(";
    expression->prettyPrint(indentation, out);
    out << ").";
    out << memberName;
}

void MemberExpression::accept(IAstVisitor &visitor)
{
    visitor.visit(shared_from_this());
}

bool MemberExpression::isLvalue() const
{
    return true;
}

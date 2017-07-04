#include "Declarations.hxx"

#include "src/ASTVisitors/IAstVisitor.hxx"

DecimalLiteralExpression::DecimalLiteralExpression(const std::string &rawValue)
        : rawValue(rawValue)
{}

DecimalLiteralExpression::~DecimalLiteralExpression() = default;

void DecimalLiteralExpression::prettyPrint(int indentation, std::ostream &out) const
{
    (void) indentation;
    out << rawValue;
}

void DecimalLiteralExpression::accept(IAstVisitor &visitor)
{
    visitor.visit(shared_from_this());
}

#include "Declarations.hxx"

#include "src/ASTVisitors/IAstVisitor.hxx"

UnaryOperatorExpression::UnaryOperatorExpression(std::shared_ptr<Expression> expression)
        : expression(expression)
{}

UnaryOperatorExpression::~UnaryOperatorExpression() = default;

void UnaryOperatorExpression::accept(IAstVisitor &visitor)
{
    visitor.visit(shared_from_this());
}

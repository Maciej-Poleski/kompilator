#include "Declarations.hxx"

#include "src/ASTVisitors/IAstVisitor.hxx"

BinaryOperatorExpression::BinaryOperatorExpression(std::shared_ptr<Expression> leftExpression,
                                                   std::shared_ptr<Expression> rightExpression)
        : leftExpression(leftExpression), rightExpression(rightExpression)
{}

BinaryOperatorExpression::~BinaryOperatorExpression() = default;

void BinaryOperatorExpression::accept(IAstVisitor &visitor)
{
    visitor.visit(shared_from_this());
}

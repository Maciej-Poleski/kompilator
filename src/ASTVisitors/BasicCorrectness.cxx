#include "BasicCorrectness.hxx"

#include "src/AST/Declarations.hxx"

void BasicCorrectness::visit(std::shared_ptr<AssignExpression> node)
{
    AstVisitorBase::visit(node);
    if (!node->leftExpression->isLvalue()) {
        reportError(node->leftExpression, "Can assign only to lvalue");
    }
}

void BasicCorrectness::visit(std::shared_ptr<FunctionDefinitionDeclarative> node)
{
    if (auto body = std::dynamic_pointer_cast<FunctionBodyExtern>(node->functionBody)) {
        body->setParent(node);
    }
}

void BasicCorrectness::visit(std::shared_ptr<FunctionDefinitionImperative> node)
{
    if (auto body = std::dynamic_pointer_cast<FunctionBodyExtern>(node->functionBody)) {
        body->setParent(node);
    }
}

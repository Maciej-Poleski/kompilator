#include <src/AST/Declarations.hxx>
#include "QuoteCallResolution.hxx"

void QuoteCallResolution::visit(std::shared_ptr<FunctionBodyDeclarative> node)
{
    bool oldMode = isCallMode;
    isCallMode = false;
    AstVisitorBase::visit(node);
    isCallMode = oldMode;
}

void QuoteCallResolution::visit(std::shared_ptr<ExplicitCallExpression> node)
{
    bool oldMode = isCallMode;
    if (!std::dynamic_pointer_cast<FunctionTermExpression>(node->expression)) {
        reportError(node->expression, "Can call only function symbol");
    } else {
        isCallMode = true;
    }
    AstVisitorBase::visit(node);
    isCallMode = oldMode;
}

void QuoteCallResolution::visit(std::shared_ptr<QuoteExpression> node)
{
    bool oldMode = isCallMode;
    if (!std::dynamic_pointer_cast<FunctionTermExpression>(node->expression)) {
        reportError(node->expression, "Can quote only function symbol");
    } else {
        isCallMode = false;
    }
    AstVisitorBase::visit(node);
    isCallMode = oldMode;
}

void QuoteCallResolution::visit(std::shared_ptr<FunctionBodyImperative> node)
{
    bool oldMode = isCallMode;
    isCallMode = true;
    AstVisitorBase::visit(node);
    isCallMode = oldMode;
}

void QuoteCallResolution::visit(std::shared_ptr<FunctionTermExpression> node)
{
    node->isCall = isCallMode;
    AstVisitorBase::visit(node);
}

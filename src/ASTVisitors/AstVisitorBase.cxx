#include "AstVisitorBase.hxx"

#include "src/AST/Declarations.hxx"

void AstVisitorBase::visit(std::shared_ptr<Ast> node)
{

}

void AstVisitorBase::visit(std::shared_ptr<Module> node)
{
    visit(std::static_pointer_cast<Ast>(node));
    for (auto &n : node->recordDefinitions) {
        n->accept(*this);
    }
    for (auto &n : node->functionDefinitions) {
        n->accept(*this);
    }
}

void AstVisitorBase::visit(std::shared_ptr<FunctionDefinition> node)
{
    visit(std::static_pointer_cast<Ast>(node));
}

void AstVisitorBase::visit(std::shared_ptr<Type> node)
{
    visit(std::static_pointer_cast<Ast>(node));
}

void AstVisitorBase::visit(std::shared_ptr<RecordDefinition> node)
{
    visit(std::static_pointer_cast<Type>(node));
    for (auto &p : node->fields) {
        p.first->accept(*this);
    }
}

void AstVisitorBase::visit(std::shared_ptr<BuiltinType> node)
{
    visit(std::static_pointer_cast<Type>(node));
}

void AstVisitorBase::visit(std::shared_ptr<IntType> node)
{
    visit(std::static_pointer_cast<BuiltinType>(node));
}

void AstVisitorBase::visit(std::shared_ptr<BoolType> node)
{
    visit(std::static_pointer_cast<BuiltinType>(node));
}

void AstVisitorBase::visit(std::shared_ptr<VarType> node)
{
    visit(std::static_pointer_cast<BuiltinType>(node));
}

void AstVisitorBase::visit(std::shared_ptr<DynamicType> node)
{
    visit(std::static_pointer_cast<BuiltinType>(node));
}

void AstVisitorBase::visit(std::shared_ptr<FunctionType> node)
{
    visit(std::static_pointer_cast<BuiltinType>(node));
}

void AstVisitorBase::visit(std::shared_ptr<NamedType> node)
{
    visit(std::static_pointer_cast<Type>(node));
}

void AstVisitorBase::visit(std::shared_ptr<AnnotatedType> node)
{
    visit(std::static_pointer_cast<Type>(node));
    node->type->accept(*this);
}

void AstVisitorBase::visit(std::shared_ptr<PtrType> node)
{
    visit(std::static_pointer_cast<Type>(node));
    node->type->accept(*this);
}

void AstVisitorBase::visit(std::shared_ptr<FunctionDefinitionDeclarative> node)
{
    visit(std::static_pointer_cast<FunctionDefinition>(node));
    for (auto &n : node->terms) {
        n->accept(*this);
    }
    node->functionBody->accept(*this);
}

void AstVisitorBase::visit(std::shared_ptr<Term> node)
{
    visit(std::static_pointer_cast<Ast>(node));
}

void AstVisitorBase::visit(std::shared_ptr<ReferenceTerm> node)
{
    visit(std::static_pointer_cast<Term>(node));
}

void AstVisitorBase::visit(std::shared_ptr<DefinitionTerm> node)
{
    visit(std::static_pointer_cast<Term>(node));
    node->type->accept(*this);
}

void AstVisitorBase::visit(std::shared_ptr<FunctionTerm> node)
{
    visit(std::static_pointer_cast<Term>(node));
    for (auto &n : node->subterms) {
        n->accept(*this);
    }
}

void AstVisitorBase::visit(std::shared_ptr<FunctionDefinitionImperative> node)
{
    visit(std::static_pointer_cast<FunctionDefinition>(node));
    if (node->result) {
        node->result->accept(*this);
    }
    for (auto &n : node->terms) {
        n->accept(*this);
    }
    node->functionBody->accept(*this);
}

void AstVisitorBase::visit(std::shared_ptr<FunctionBody> node)
{
    visit(std::static_pointer_cast<Ast>(node));
}

void AstVisitorBase::visit(std::shared_ptr<FunctionBodyDeclarative> node)
{
    visit(std::static_pointer_cast<FunctionBody>(node));
    for (auto &n : node->body) {
        n->accept(*this);
    }
}

void AstVisitorBase::visit(std::shared_ptr<Expression> node)
{
    visit(std::static_pointer_cast<Ast>(node));
}

void AstVisitorBase::visit(std::shared_ptr<LiteralExpression> node)
{
    visit(std::static_pointer_cast<Expression>(node));
}

void AstVisitorBase::visit(std::shared_ptr<DecimalLiteralExpression> node)
{
    visit(std::static_pointer_cast<LiteralExpression>(node));
}

void AstVisitorBase::visit(std::shared_ptr<BooleanLiteralExpression> node)
{
    visit(std::static_pointer_cast<LiteralExpression>(node));
}

void AstVisitorBase::visit(std::shared_ptr<NameReferenceExpression> node)
{
    visit(std::static_pointer_cast<Expression>(node));
}

void AstVisitorBase::visit(std::shared_ptr<TermDefinitionExpression> node)
{
    visit(std::static_pointer_cast<Expression>(node));
}

void AstVisitorBase::visit(std::shared_ptr<AtomicTermDefinitionExpression> node)
{
    visit(std::static_pointer_cast<TermDefinitionExpression>(node));
    node->type->accept(*this);
}

void AstVisitorBase::visit(std::shared_ptr<FunctionTermExpression> node)
{
    visit(std::static_pointer_cast<TermDefinitionExpression>(node));
    for (auto &n : node->subterms) {
        n->accept(*this);
    }
}

void AstVisitorBase::visit(std::shared_ptr<OperatorExpression> node)
{
    visit(std::static_pointer_cast<Expression>(node));
}

void AstVisitorBase::visit(std::shared_ptr<UnaryOperatorExpression> node)
{
    visit(std::static_pointer_cast<OperatorExpression>(node));
    node->expression->accept(*this);
}

void AstVisitorBase::visit(std::shared_ptr<BinaryOperatorExpression> node)
{
    visit(std::static_pointer_cast<OperatorExpression>(node));
    node->leftExpression->accept(*this);
    node->rightExpression->accept(*this);
}

void AstVisitorBase::visit(std::shared_ptr<UnifyExpression> node)
{
    visit(std::static_pointer_cast<BinaryOperatorExpression>(node));
}

void AstVisitorBase::visit(std::shared_ptr<AssignExpression> node)
{
    visit(std::static_pointer_cast<BinaryOperatorExpression>(node));
}

void AstVisitorBase::visit(std::shared_ptr<ExplicitCallExpression> node)
{
    visit(std::static_pointer_cast<UnaryOperatorExpression>(node));
}

void AstVisitorBase::visit(std::shared_ptr<QuoteExpression> node)
{
    visit(std::static_pointer_cast<UnaryOperatorExpression>(node));
}

void AstVisitorBase::visit(std::shared_ptr<OrExpression> node)
{
    visit(std::static_pointer_cast<BinaryOperatorExpression>(node));
}

void AstVisitorBase::visit(std::shared_ptr<AndExpression> node)
{
    visit(std::static_pointer_cast<BinaryOperatorExpression>(node));
}

void AstVisitorBase::visit(std::shared_ptr<EqualExpression> node)
{
    visit(std::static_pointer_cast<BinaryOperatorExpression>(node));
}

void AstVisitorBase::visit(std::shared_ptr<NotEqualExpression> node)
{
    visit(std::static_pointer_cast<BinaryOperatorExpression>(node));
}

void AstVisitorBase::visit(std::shared_ptr<LessExpression> node)
{
    visit(std::static_pointer_cast<BinaryOperatorExpression>(node));
}

void AstVisitorBase::visit(std::shared_ptr<GreaterExpression> node)
{
    visit(std::static_pointer_cast<BinaryOperatorExpression>(node));
}

void AstVisitorBase::visit(std::shared_ptr<LessOrEqualExpression> node)
{
    visit(std::static_pointer_cast<BinaryOperatorExpression>(node));
}

void AstVisitorBase::visit(std::shared_ptr<GreaterOrEqualExpression> node)
{
    visit(std::static_pointer_cast<BinaryOperatorExpression>(node));
}

void AstVisitorBase::visit(std::shared_ptr<AddExpression> node)
{
    visit(std::static_pointer_cast<BinaryOperatorExpression>(node));
}

void AstVisitorBase::visit(std::shared_ptr<SubtractExpression> node)
{
    visit(std::static_pointer_cast<BinaryOperatorExpression>(node));
}

void AstVisitorBase::visit(std::shared_ptr<MultiplyExpression> node)
{
    visit(std::static_pointer_cast<BinaryOperatorExpression>(node));
}

void AstVisitorBase::visit(std::shared_ptr<DivideExpression> node)
{
    visit(std::static_pointer_cast<BinaryOperatorExpression>(node));
}

void AstVisitorBase::visit(std::shared_ptr<ModuloExpression> node)
{
    visit(std::static_pointer_cast<BinaryOperatorExpression>(node));
}

void AstVisitorBase::visit(std::shared_ptr<NotExpression> node)
{
    visit(std::static_pointer_cast<UnaryOperatorExpression>(node));
}

void AstVisitorBase::visit(std::shared_ptr<NegateExpression> node)
{
    visit(std::static_pointer_cast<UnaryOperatorExpression>(node));
}

void AstVisitorBase::visit(std::shared_ptr<DereferenceExpression> node)
{
    visit(std::static_pointer_cast<UnaryOperatorExpression>(node));
}

void AstVisitorBase::visit(std::shared_ptr<AddressOfExpression> node)
{
    visit(std::static_pointer_cast<UnaryOperatorExpression>(node));
}

void AstVisitorBase::visit(std::shared_ptr<MemberExpression> node)
{
    visit(std::static_pointer_cast<UnaryOperatorExpression>(node));
}

void AstVisitorBase::visit(std::shared_ptr<FunctionBodyImperative> node)
{
    visit(std::static_pointer_cast<FunctionBody>(node));
    for (auto &n : node->statements) {
        n->accept(*this);
    }
}

void AstVisitorBase::visit(std::shared_ptr<Statement> node)
{
    visit(std::static_pointer_cast<Ast>(node));
}

void AstVisitorBase::visit(std::shared_ptr<ExpressionStatement> node)
{
    visit(std::static_pointer_cast<Statement>(node));
    node->expression->accept(*this);
}

void AstVisitorBase::visit(std::shared_ptr<IfStatement> node)
{
    visit(std::static_pointer_cast<Statement>(node));
    node->condition->accept(*this);
    for (auto &n : node->thenBlock) {
        n->accept(*this);
    }
    for (auto &n : node->elseBlock) {
        n->accept(*this);
    }
}

void AstVisitorBase::visit(std::shared_ptr<ForStatement> node)
{
    visit(std::static_pointer_cast<Statement>(node));
    for (auto &n : node->initExpressions) {
        n->accept(*this);
    }
    if (node->conditionExpression) {
        node->conditionExpression->accept(*this);
    }
    for (auto &n : node->lastExpressions) {
        n->accept(*this);
    }
    for (auto &n : node->body) {
        n->accept(*this);
    }
}

void AstVisitorBase::visit(std::shared_ptr<BreakStatement> node)
{
    visit(std::static_pointer_cast<Statement>(node));
}

void AstVisitorBase::visit(std::shared_ptr<ContinueStatement> node)
{
    visit(std::static_pointer_cast<Statement>(node));
}

void AstVisitorBase::visit(std::shared_ptr<FunctionBodyExtern> node)
{
    visit(std::static_pointer_cast<FunctionBody>(node));
}

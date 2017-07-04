#ifndef KOMPILATOR_ASTVISITORBASE_HXX
#define KOMPILATOR_ASTVISITORBASE_HXX

#include "IAstVisitor.hxx"

#include <memory>

class AstVisitorBase : public IAstVisitor
{
public:
    void visit(std::shared_ptr<Ast> node) override;
    void visit(std::shared_ptr<Module> node) override;
    void visit(std::shared_ptr<FunctionDefinition> node) override;
    void visit(std::shared_ptr<Type> node) override;
    void visit(std::shared_ptr<RecordDefinition> node) override;
    void visit(std::shared_ptr<BuiltinType> node) override;
    void visit(std::shared_ptr<IntType> node) override;
    void visit(std::shared_ptr<BoolType> node) override;
    void visit(std::shared_ptr<VarType> node) override;
    void visit(std::shared_ptr<DynamicType> node) override;
    void visit(std::shared_ptr<FunctionType> node) override;
    void visit(std::shared_ptr<NamedType> node) override;
    void visit(std::shared_ptr<AnnotatedType> node) override;
    void visit(std::shared_ptr<PtrType> node) override;
    void visit(std::shared_ptr<FunctionDefinitionDeclarative> node) override;
    void visit(std::shared_ptr<Term> node) override;
    void visit(std::shared_ptr<ReferenceTerm> node) override;
    void visit(std::shared_ptr<DefinitionTerm> node) override;
    void visit(std::shared_ptr<FunctionTerm> node) override;
    void visit(std::shared_ptr<FunctionDefinitionImperative> node) override;
    void visit(std::shared_ptr<FunctionBody> node) override;
    void visit(std::shared_ptr<FunctionBodyDeclarative> node) override;
    void visit(std::shared_ptr<Expression> node) override;
    void visit(std::shared_ptr<LiteralExpression> node) override;
    void visit(std::shared_ptr<DecimalLiteralExpression> node) override;
    void visit(std::shared_ptr<BooleanLiteralExpression> node) override;
    void visit(std::shared_ptr<NameReferenceExpression> node) override;
    void visit(std::shared_ptr<TermDefinitionExpression> node) override;
    void visit(std::shared_ptr<AtomicTermDefinitionExpression> node) override;
    void visit(std::shared_ptr<FunctionTermExpression> node) override;
    void visit(std::shared_ptr<OperatorExpression> node) override;
    void visit(std::shared_ptr<UnaryOperatorExpression> node) override;
    void visit(std::shared_ptr<BinaryOperatorExpression> node) override;
    void visit(std::shared_ptr<UnifyExpression> node) override;
    void visit(std::shared_ptr<AssignExpression> node) override;
    void visit(std::shared_ptr<ExplicitCallExpression> node) override;
    void visit(std::shared_ptr<QuoteExpression> node) override;
    void visit(std::shared_ptr<OrExpression> node) override;
    void visit(std::shared_ptr<AndExpression> node) override;
    void visit(std::shared_ptr<EqualExpression> node) override;
    void visit(std::shared_ptr<NotEqualExpression> node) override;
    void visit(std::shared_ptr<LessExpression> node) override;
    void visit(std::shared_ptr<GreaterExpression> node) override;
    void visit(std::shared_ptr<LessOrEqualExpression> node) override;
    void visit(std::shared_ptr<GreaterOrEqualExpression> node) override;
    void visit(std::shared_ptr<AddExpression> node) override;
    void visit(std::shared_ptr<SubtractExpression> node) override;
    void visit(std::shared_ptr<MultiplyExpression> node) override;
    void visit(std::shared_ptr<DivideExpression> node) override;
    void visit(std::shared_ptr<ModuloExpression> node) override;
    void visit(std::shared_ptr<NotExpression> node) override;
    void visit(std::shared_ptr<NegateExpression> node) override;
    void visit(std::shared_ptr<DereferenceExpression> node) override;
    void visit(std::shared_ptr<AddressOfExpression> node) override;
    void visit(std::shared_ptr<MemberExpression> node) override;
    void visit(std::shared_ptr<FunctionBodyImperative> node) override;
    void visit(std::shared_ptr<Statement> node) override;
    void visit(std::shared_ptr<ExpressionStatement> node) override;
    void visit(std::shared_ptr<IfStatement> node) override;
    void visit(std::shared_ptr<ForStatement> node) override;
    void visit(std::shared_ptr<BreakStatement> node) override;
    void visit(std::shared_ptr<ContinueStatement> node) override;
    void visit(std::shared_ptr<FunctionBodyExtern> node) override;
};


#endif //KOMPILATOR_ASTVISITORBASE_HXX

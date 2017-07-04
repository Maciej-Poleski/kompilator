#ifndef KOMPILATOR_IASTVISITOR_HXX
#define KOMPILATOR_IASTVISITOR_HXX

#include <memory>

#include "src/AST/DeclarationsFwd.hxx"

class IAstVisitor
{
public:
    virtual ~IAstVisitor();

    virtual void visit(std::shared_ptr<Ast> node) = 0;
    virtual void visit(std::shared_ptr<Module> node) = 0;
    virtual void visit(std::shared_ptr<FunctionDefinition> node) = 0;
    virtual void visit(std::shared_ptr<Type> node) = 0;
    virtual void visit(std::shared_ptr<RecordDefinition> node) = 0;
    virtual void visit(std::shared_ptr<BuiltinType> node) = 0;
    virtual void visit(std::shared_ptr<IntType> node) = 0;
    virtual void visit(std::shared_ptr<BoolType> node) = 0;
    virtual void visit(std::shared_ptr<VarType> node) = 0;
    virtual void visit(std::shared_ptr<DynamicType> node) = 0;
    virtual void visit(std::shared_ptr<FunctionType> node) = 0;
    virtual void visit(std::shared_ptr<NamedType> node) = 0;
    virtual void visit(std::shared_ptr<AnnotatedType> node) = 0;
    virtual void visit(std::shared_ptr<PtrType> node) = 0;
    virtual void visit(std::shared_ptr<FunctionDefinitionDeclarative> node) = 0;
    virtual void visit(std::shared_ptr<Term> node) = 0;
    virtual void visit(std::shared_ptr<ReferenceTerm> node) = 0;
    virtual void visit(std::shared_ptr<DefinitionTerm> node) = 0;
    virtual void visit(std::shared_ptr<FunctionTerm> node) = 0;
    virtual void visit(std::shared_ptr<FunctionDefinitionImperative> node) = 0;
    virtual void visit(std::shared_ptr<FunctionBody> node) = 0;
    virtual void visit(std::shared_ptr<FunctionBodyDeclarative> node) = 0;
    virtual void visit(std::shared_ptr<Expression> node) = 0;
    virtual void visit(std::shared_ptr<LiteralExpression> node) = 0;
    virtual void visit(std::shared_ptr<DecimalLiteralExpression> node) = 0;
    virtual void visit(std::shared_ptr<BooleanLiteralExpression> node) = 0;
    virtual void visit(std::shared_ptr<NameReferenceExpression> node) = 0;
    virtual void visit(std::shared_ptr<TermDefinitionExpression> node) = 0;
    virtual void visit(std::shared_ptr<AtomicTermDefinitionExpression> node) = 0;
    virtual void visit(std::shared_ptr<FunctionTermExpression> node) = 0;
    virtual void visit(std::shared_ptr<OperatorExpression> node) = 0;
    virtual void visit(std::shared_ptr<UnaryOperatorExpression> node) = 0;
    virtual void visit(std::shared_ptr<BinaryOperatorExpression> node) = 0;
    virtual void visit(std::shared_ptr<UnifyExpression> node) = 0;
    virtual void visit(std::shared_ptr<AssignExpression> node) = 0;
    virtual void visit(std::shared_ptr<ExplicitCallExpression> node) = 0;
    virtual void visit(std::shared_ptr<QuoteExpression> node) = 0;
    virtual void visit(std::shared_ptr<OrExpression> node) = 0;
    virtual void visit(std::shared_ptr<AndExpression> node) = 0;
    virtual void visit(std::shared_ptr<EqualExpression> node) = 0;
    virtual void visit(std::shared_ptr<NotEqualExpression> node) = 0;
    virtual void visit(std::shared_ptr<LessExpression> node) = 0;
    virtual void visit(std::shared_ptr<GreaterExpression> node) = 0;
    virtual void visit(std::shared_ptr<LessOrEqualExpression> node) = 0;
    virtual void visit(std::shared_ptr<GreaterOrEqualExpression> node) = 0;
    virtual void visit(std::shared_ptr<AddExpression> node) = 0;
    virtual void visit(std::shared_ptr<SubtractExpression> node) = 0;
    virtual void visit(std::shared_ptr<MultiplyExpression> node) = 0;
    virtual void visit(std::shared_ptr<DivideExpression> node) = 0;
    virtual void visit(std::shared_ptr<ModuloExpression> node) = 0;
    virtual void visit(std::shared_ptr<NotExpression> node) = 0;
    virtual void visit(std::shared_ptr<NegateExpression> node) = 0;
    virtual void visit(std::shared_ptr<DereferenceExpression> node) = 0;
    virtual void visit(std::shared_ptr<AddressOfExpression> node) = 0;
    virtual void visit(std::shared_ptr<MemberExpression> node) = 0;
    virtual void visit(std::shared_ptr<FunctionBodyImperative> node) = 0;
    virtual void visit(std::shared_ptr<Statement> node) = 0;
    virtual void visit(std::shared_ptr<ExpressionStatement> node) = 0;
    virtual void visit(std::shared_ptr<IfStatement> node) = 0;
    virtual void visit(std::shared_ptr<ForStatement> node) = 0;
    virtual void visit(std::shared_ptr<BreakStatement> node) = 0;
    virtual void visit(std::shared_ptr<ContinueStatement> node) = 0;
    virtual void visit(std::shared_ptr<FunctionBodyExtern> node) = 0;
};


#endif //KOMPILATOR_IASTVISITOR_HXX

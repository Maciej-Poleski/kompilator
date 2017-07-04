#ifndef KOMPILATOR_DECLARATIONS_HXX
#define KOMPILATOR_DECLARATIONS_HXX

#include <vector>
#include <memory>

#include "DeclarationsFwd.hxx"

class IAstVisitor;

template<class AstNode>
class EnableSharedAst
{
public:
    std::shared_ptr<AstNode> shared_from_this();
};

class Ast : public std::enable_shared_from_this<Ast>, public EnableSharedAst<Ast>
{
public:
    using EnableSharedAst<Ast>::shared_from_this;

    virtual ~Ast();

    void dump(std::ostream &out) const;

    virtual void prettyPrint(int indentation, std::ostream &out) const = 0;

    virtual void accept(IAstVisitor &visitor);

private:
};

class Module : public Ast, public EnableSharedAst<Module>
{
public:
    using EnableSharedAst<Module>::shared_from_this;

    virtual ~Module() override;

    std::vector<std::shared_ptr<RecordDefinition>> recordDefinitions;
    std::vector<std::shared_ptr<FunctionDefinition>> functionDefinitions;

    virtual void prettyPrint(int indentation, std::ostream &out) const override;

    virtual void accept(IAstVisitor &visitor) override;
};

class FunctionDefinition : public Ast, public EnableSharedAst<FunctionDefinition>
{
public:
    using EnableSharedAst<FunctionDefinition>::shared_from_this;
    explicit FunctionDefinition(const std::string &name);
    virtual ~FunctionDefinition() override;

    virtual void accept(IAstVisitor &visitor) override;

    std::string name;
};

class Type : public Ast, public EnableSharedAst<Type>
{
public:
    using EnableSharedAst<Type>::shared_from_this;
    virtual ~Type() override;

    virtual void accept(IAstVisitor &visitor) override;
};

class RecordDefinition : public Type, public EnableSharedAst<RecordDefinition>
{
public:
    using EnableSharedAst<RecordDefinition>::shared_from_this;
    RecordDefinition(const std::string &name, const std::vector<std::pair<std::shared_ptr<Type>, std::string>> &fields);

    virtual ~RecordDefinition() override;

    virtual void prettyPrint(int indentation, std::ostream &out) const override;

    virtual void accept(IAstVisitor &visitor) override;

    std::string name;
    std::vector<std::pair<std::shared_ptr<Type>, std::string>> fields;
};

class BuiltinType : public Type, public EnableSharedAst<BuiltinType>
{
public:
    using EnableSharedAst<BuiltinType>::shared_from_this;
    explicit BuiltinType(const std::string &name);

    virtual ~BuiltinType() override;

    virtual void accept(IAstVisitor &visitor) override;

    std::string name;
};

class IntType : public BuiltinType, public EnableSharedAst<IntType>
{
public:
    using EnableSharedAst<IntType>::shared_from_this;
    explicit IntType();

    virtual ~IntType() override;

    virtual void prettyPrint(int indentation, std::ostream &out) const override;

    virtual void accept(IAstVisitor &visitor) override;
};

class BoolType : public BuiltinType, public EnableSharedAst<BoolType>
{
public:
    using EnableSharedAst<BoolType>::shared_from_this;
    explicit BoolType();

    virtual ~BoolType() override;

    virtual void prettyPrint(int indentation, std::ostream &out) const override;

    virtual void accept(IAstVisitor &visitor) override;
};

class VarType : public BuiltinType, public EnableSharedAst<VarType>
{
public:
    using EnableSharedAst<VarType>::shared_from_this;
    explicit VarType();

    virtual ~VarType() override;

    virtual void prettyPrint(int indentation, std::ostream &out) const override;

    virtual void accept(IAstVisitor &visitor) override;
};

class DynamicType : public BuiltinType, public EnableSharedAst<DynamicType>
{
public:
    using EnableSharedAst<DynamicType>::shared_from_this;
    explicit DynamicType();

    virtual ~DynamicType() override;

    virtual void prettyPrint(int indentation, std::ostream &out) const override;

    virtual void accept(IAstVisitor &visitor) override;
};

class FunctionType : public BuiltinType, public EnableSharedAst<FunctionType>
{
public:
    using EnableSharedAst<FunctionType>::shared_from_this;
    explicit FunctionType();

    virtual ~FunctionType() override;

    virtual void prettyPrint(int indentation, std::ostream &out) const override;

    virtual void accept(IAstVisitor &visitor) override;
};

class NamedType : public Type, public EnableSharedAst<NamedType>
{
public:
    using EnableSharedAst<NamedType>::shared_from_this;
    explicit NamedType(const std::string &name);

    virtual ~NamedType() override;

    virtual void prettyPrint(int indentation, std::ostream &out) const override;

    virtual void accept(IAstVisitor &visitor) override;

    std::string name;

    // NameResolution
    std::weak_ptr<Type> typeDefinition;
};

class AnnotatedType : public Type, public EnableSharedAst<AnnotatedType>
{
public:
    using EnableSharedAst<AnnotatedType>::shared_from_this;
    explicit AnnotatedType(std::shared_ptr<Type> type);

    virtual ~AnnotatedType() override;

    virtual void prettyPrint(int indentation, std::ostream &out) const override;

    virtual void accept(IAstVisitor &visitor) override;

    std::shared_ptr<Type> type;
};

class PtrType : public Type, public EnableSharedAst<PtrType>
{
public:
    using EnableSharedAst<PtrType>::shared_from_this;
    explicit PtrType(std::shared_ptr<Type> type);

    virtual ~PtrType() override;

    virtual void prettyPrint(int indentation, std::ostream &out) const override;

    virtual void accept(IAstVisitor &visitor) override;

    std::shared_ptr<Type> type;
};

class FunctionDefinitionDeclarative : public FunctionDefinition, public EnableSharedAst<FunctionDefinitionDeclarative>
{
public:
    using EnableSharedAst<FunctionDefinitionDeclarative>::shared_from_this;
    FunctionDefinitionDeclarative(const std::string name, const std::vector<std::shared_ptr<Expression>> &terms,
                                  const std::shared_ptr<FunctionBody> &functionBody);

    virtual ~FunctionDefinitionDeclarative() override;

    virtual void prettyPrint(int indentation, std::ostream &out) const override;

    virtual void accept(IAstVisitor &visitor) override;

    std::vector<std::shared_ptr<Expression>> terms;
    std::shared_ptr<FunctionBody> functionBody;
};

class Term : public Ast, public EnableSharedAst<Term>
{
public:
    using EnableSharedAst<Term>::shared_from_this;
    virtual ~Term() override;

    virtual void accept(IAstVisitor &visitor) override;
};

class ReferenceTerm : public Term, public EnableSharedAst<ReferenceTerm>
{
public:
    using EnableSharedAst<ReferenceTerm>::shared_from_this;
    explicit ReferenceTerm(const std::string &name);

    virtual ~ReferenceTerm() override;

    virtual void prettyPrint(int indentation, std::ostream &out) const override;

    virtual void accept(IAstVisitor &visitor) override;

    std::string name;

    // NameResolution
    std::weak_ptr<DefinitionTerm> definitionTerm;
};

class DefinitionTerm : public Term, public EnableSharedAst<DefinitionTerm>
{
public:
    using EnableSharedAst<DefinitionTerm>::shared_from_this;
    DefinitionTerm(std::shared_ptr<Type> type, const std::string &name);

    virtual ~DefinitionTerm() override;

    virtual void prettyPrint(int indentation, std::ostream &out) const override;

    virtual void accept(IAstVisitor &visitor) override;

    std::shared_ptr<Type> type;
    std::string name;
};

class FunctionTerm : public Term, public EnableSharedAst<FunctionTerm>
{
public:
    using EnableSharedAst<FunctionTerm>::shared_from_this;
    FunctionTerm(const std::string &name, const std::vector<std::shared_ptr<Term>> &subterms);

    virtual ~FunctionTerm() override;

    virtual void prettyPrint(int indentation, std::ostream &out) const override;

    virtual void accept(IAstVisitor &visitor) override;

    std::string name;
    std::vector<std::shared_ptr<Term>> subterms;
};

class FunctionDefinitionImperative : public FunctionDefinition, public EnableSharedAst<FunctionDefinitionImperative>
{
public:
    using EnableSharedAst<FunctionDefinitionImperative>::shared_from_this;
    FunctionDefinitionImperative(const std::string &name, std::shared_ptr<Expression> result,
                                 const std::vector<std::shared_ptr<Expression>> &terms,
                                 std::shared_ptr<FunctionBody> functionBody);

    virtual ~FunctionDefinitionImperative() override;

    virtual void prettyPrint(int indentation, std::ostream &out) const override;

    virtual void accept(IAstVisitor &visitor) override;

    std::shared_ptr<Expression> result;   // nullptr means "void"
    std::vector<std::shared_ptr<Expression>> terms;
    std::shared_ptr<FunctionBody> functionBody;
};

class FunctionBody : public Ast, public EnableSharedAst<FunctionBody>
{
public:
    using EnableSharedAst<FunctionBody>::shared_from_this;
    virtual ~FunctionBody() override;

    virtual void accept(IAstVisitor &visitor) override;
};

class FunctionBodyDeclarative : public FunctionBody, public EnableSharedAst<FunctionBodyDeclarative>
{
public:
    using EnableSharedAst<FunctionBodyDeclarative>::shared_from_this;
    explicit FunctionBodyDeclarative(const std::vector<std::shared_ptr<Expression>> &body);

    virtual ~FunctionBodyDeclarative() override;

    virtual void prettyPrint(int indentation, std::ostream &out) const override;

    virtual void accept(IAstVisitor &visitor) override;

    std::vector<std::shared_ptr<Expression>> body;
};

class Expression : public Ast, public EnableSharedAst<Expression>
{
public:
    using EnableSharedAst<Expression>::shared_from_this;
    virtual ~Expression() override;

    virtual void accept(IAstVisitor &visitor) override;

    virtual bool isLvalue() const;

    // TypeChecking, non-tree
    std::shared_ptr<Type> type;
};

class LiteralExpression : public Expression, public EnableSharedAst<LiteralExpression>
{
public:
    using EnableSharedAst<LiteralExpression>::shared_from_this;
    virtual ~LiteralExpression() override;

    virtual void accept(IAstVisitor &visitor) override;
};

class DecimalLiteralExpression : public LiteralExpression, public EnableSharedAst<DecimalLiteralExpression>
{
public:
    using EnableSharedAst<DecimalLiteralExpression>::shared_from_this;
    explicit DecimalLiteralExpression(const std::string &rawValue);

    virtual ~DecimalLiteralExpression() override;

    virtual void prettyPrint(int indentation, std::ostream &out) const override;

    virtual void accept(IAstVisitor &visitor) override;

    std::string rawValue;
};

class BooleanLiteralExpression : public LiteralExpression, public EnableSharedAst<BooleanLiteralExpression>
{
public:
    using EnableSharedAst<BooleanLiteralExpression>::shared_from_this;
    explicit BooleanLiteralExpression(bool value);

    virtual ~BooleanLiteralExpression() override;

    virtual void prettyPrint(int indentation, std::ostream &out) const override;

    virtual void accept(IAstVisitor &visitor) override;

    bool value;
};

class NameReferenceExpression : public Expression, public EnableSharedAst<NameReferenceExpression>
{
public:
    using EnableSharedAst<NameReferenceExpression>::shared_from_this;
    explicit NameReferenceExpression(const std::string &name);

    virtual ~NameReferenceExpression() override;

    virtual void prettyPrint(int indentation, std::ostream &out) const override;

    virtual void accept(IAstVisitor &visitor) override;

    virtual bool isLvalue() const override;

    std::string name;

    // NameResolution
    std::weak_ptr<AtomicTermDefinitionExpression> atomicTermDefinitionExpression;
};

class TermDefinitionExpression : public Expression, public EnableSharedAst<TermDefinitionExpression>
{
public:
    using EnableSharedAst<TermDefinitionExpression>::shared_from_this;
    virtual ~TermDefinitionExpression() override;

    virtual void accept(IAstVisitor &visitor) override;
};

class AtomicTermDefinitionExpression
        : public TermDefinitionExpression, public EnableSharedAst<AtomicTermDefinitionExpression>
{
public:
    using EnableSharedAst<AtomicTermDefinitionExpression>::shared_from_this;
    AtomicTermDefinitionExpression(std::shared_ptr<Type> type, const std::string &name);

    virtual ~AtomicTermDefinitionExpression() override;

    virtual void prettyPrint(int indentation, std::ostream &out) const override;

    virtual void accept(IAstVisitor &visitor) override;

    virtual bool isLvalue() const override;

    std::shared_ptr<Type> type;
    std::string name;
};

class FunctionTermExpression : public TermDefinitionExpression, public EnableSharedAst<FunctionTermExpression>
{
public:
    using EnableSharedAst<FunctionTermExpression>::shared_from_this;
    FunctionTermExpression(const std::string &name, const std::vector<std::shared_ptr<Expression>> &subterms);

    virtual ~FunctionTermExpression() override;

    virtual void prettyPrint(int indentation, std::ostream &out) const override;

    virtual void accept(IAstVisitor &visitor) override;

    std::string name;
    std::vector<std::shared_ptr<Expression>> subterms;

    // QuoteCallResolution
    bool isCall;
};

class OperatorExpression : public Expression, public EnableSharedAst<OperatorExpression>
{
public:
    using EnableSharedAst<OperatorExpression>::shared_from_this;
    virtual ~OperatorExpression() override;

    virtual void accept(IAstVisitor &visitor) override;
};

class UnaryOperatorExpression : public OperatorExpression, public EnableSharedAst<UnaryOperatorExpression>
{
public:
    using EnableSharedAst<UnaryOperatorExpression>::shared_from_this;
    explicit UnaryOperatorExpression(std::shared_ptr<Expression> expression);

    virtual ~UnaryOperatorExpression() override;

    virtual void accept(IAstVisitor &visitor) override;

    std::shared_ptr<Expression> expression;
};

class BinaryOperatorExpression : public OperatorExpression, public EnableSharedAst<BinaryOperatorExpression>
{
public:
    using EnableSharedAst<BinaryOperatorExpression>::shared_from_this;
    BinaryOperatorExpression(std::shared_ptr<Expression> leftExpression, std::shared_ptr<Expression> rightExpression);

    virtual ~BinaryOperatorExpression() override;

    virtual void accept(IAstVisitor &visitor) override;

    std::shared_ptr<Expression> leftExpression;
    std::shared_ptr<Expression> rightExpression;
};

class UnifyExpression : public BinaryOperatorExpression, public EnableSharedAst<UnifyExpression>
{
public:
    using EnableSharedAst<UnifyExpression>::shared_from_this;
    using BinaryOperatorExpression::BinaryOperatorExpression;

    virtual void prettyPrint(int indentation, std::ostream &out) const override;

    virtual void accept(IAstVisitor &visitor) override;
};

class AssignExpression : public BinaryOperatorExpression, public EnableSharedAst<AssignExpression>
{
public:
    using EnableSharedAst<AssignExpression>::shared_from_this;
    using BinaryOperatorExpression::BinaryOperatorExpression;

    virtual void prettyPrint(int indentation, std::ostream &out) const override;

    virtual void accept(IAstVisitor &visitor) override;

    virtual bool isLvalue() const override;
};

class ExplicitCallExpression : public UnaryOperatorExpression, public EnableSharedAst<ExplicitCallExpression>
{
public:
    using EnableSharedAst<ExplicitCallExpression>::shared_from_this;
    using UnaryOperatorExpression::UnaryOperatorExpression;

    virtual void prettyPrint(int indentation, std::ostream &out) const override;

    virtual void accept(IAstVisitor &visitor) override;
};

class QuoteExpression : public UnaryOperatorExpression, public EnableSharedAst<QuoteExpression>
{
public:
    using EnableSharedAst<QuoteExpression>::shared_from_this;
    using UnaryOperatorExpression::UnaryOperatorExpression;

    virtual void prettyPrint(int indentation, std::ostream &out) const override;

    virtual void accept(IAstVisitor &visitor) override;
};

class OrExpression : public BinaryOperatorExpression, public EnableSharedAst<OrExpression>
{
public:
    using EnableSharedAst<OrExpression>::shared_from_this;
    using BinaryOperatorExpression::BinaryOperatorExpression;

    virtual void prettyPrint(int indentation, std::ostream &out) const override;

    virtual void accept(IAstVisitor &visitor) override;
};

class AndExpression : public BinaryOperatorExpression, public EnableSharedAst<AndExpression>
{
public:
    using EnableSharedAst<AndExpression>::shared_from_this;
    using BinaryOperatorExpression::BinaryOperatorExpression;

    virtual void prettyPrint(int indentation, std::ostream &out) const override;

    virtual void accept(IAstVisitor &visitor) override;
};

class EqualExpression : public BinaryOperatorExpression, public EnableSharedAst<EqualExpression>
{
public:
    using EnableSharedAst<EqualExpression>::shared_from_this;
    using BinaryOperatorExpression::BinaryOperatorExpression;

    virtual void prettyPrint(int indentation, std::ostream &out) const override;

    virtual void accept(IAstVisitor &visitor) override;
};

class NotEqualExpression : public BinaryOperatorExpression, public EnableSharedAst<NotEqualExpression>
{
public:
    using EnableSharedAst<NotEqualExpression>::shared_from_this;
    using BinaryOperatorExpression::BinaryOperatorExpression;

    virtual void prettyPrint(int indentation, std::ostream &out) const override;

    virtual void accept(IAstVisitor &visitor) override;
};

class LessExpression : public BinaryOperatorExpression, public EnableSharedAst<LessExpression>
{
public:
    using EnableSharedAst<LessExpression>::shared_from_this;
    using BinaryOperatorExpression::BinaryOperatorExpression;

    virtual void prettyPrint(int indentation, std::ostream &out) const override;

    virtual void accept(IAstVisitor &visitor) override;
};

class GreaterExpression : public BinaryOperatorExpression, public EnableSharedAst<GreaterExpression>
{
public:
    using EnableSharedAst<GreaterExpression>::shared_from_this;
    using BinaryOperatorExpression::BinaryOperatorExpression;

    virtual void prettyPrint(int indentation, std::ostream &out) const override;

    virtual void accept(IAstVisitor &visitor) override;
};

class LessOrEqualExpression : public BinaryOperatorExpression, public EnableSharedAst<LessOrEqualExpression>
{
public:
    using EnableSharedAst<LessOrEqualExpression>::shared_from_this;
    using BinaryOperatorExpression::BinaryOperatorExpression;

    virtual void prettyPrint(int indentation, std::ostream &out) const override;

    virtual void accept(IAstVisitor &visitor) override;
};

class GreaterOrEqualExpression : public BinaryOperatorExpression, public EnableSharedAst<GreaterOrEqualExpression>
{
public:
    using EnableSharedAst<GreaterOrEqualExpression>::shared_from_this;
    using BinaryOperatorExpression::BinaryOperatorExpression;

    virtual void prettyPrint(int indentation, std::ostream &out) const override;

    virtual void accept(IAstVisitor &visitor) override;
};

class AddExpression : public BinaryOperatorExpression, public EnableSharedAst<AddExpression>
{
public:
    using EnableSharedAst<AddExpression>::shared_from_this;
    using BinaryOperatorExpression::BinaryOperatorExpression;

    virtual void prettyPrint(int indentation, std::ostream &out) const override;

    virtual void accept(IAstVisitor &visitor) override;
};

class SubtractExpression : public BinaryOperatorExpression, public EnableSharedAst<SubtractExpression>
{
public:
    using EnableSharedAst<SubtractExpression>::shared_from_this;
    using BinaryOperatorExpression::BinaryOperatorExpression;

    virtual void prettyPrint(int indentation, std::ostream &out) const override;

    virtual void accept(IAstVisitor &visitor) override;
};

class MultiplyExpression : public BinaryOperatorExpression, public EnableSharedAst<MultiplyExpression>
{
public:
    using EnableSharedAst<MultiplyExpression>::shared_from_this;
    using BinaryOperatorExpression::BinaryOperatorExpression;

    virtual void prettyPrint(int indentation, std::ostream &out) const override;

    virtual void accept(IAstVisitor &visitor) override;
};

class DivideExpression : public BinaryOperatorExpression, public EnableSharedAst<DivideExpression>
{
public:
    using EnableSharedAst<DivideExpression>::shared_from_this;
    using BinaryOperatorExpression::BinaryOperatorExpression;

    virtual void prettyPrint(int indentation, std::ostream &out) const override;

    virtual void accept(IAstVisitor &visitor) override;
};

class ModuloExpression : public BinaryOperatorExpression, public EnableSharedAst<ModuloExpression>
{
public:
    using EnableSharedAst<ModuloExpression>::shared_from_this;
    using BinaryOperatorExpression::BinaryOperatorExpression;

    virtual void prettyPrint(int indentation, std::ostream &out) const override;

    virtual void accept(IAstVisitor &visitor) override;
};

class NotExpression : public UnaryOperatorExpression, public EnableSharedAst<NotExpression>
{
public:
    using EnableSharedAst<NotExpression>::shared_from_this;
    using UnaryOperatorExpression::UnaryOperatorExpression;

    virtual void prettyPrint(int indentation, std::ostream &out) const override;

    virtual void accept(IAstVisitor &visitor) override;
};

class NegateExpression : public UnaryOperatorExpression, public EnableSharedAst<NegateExpression>
{
public:
    using EnableSharedAst<NegateExpression>::shared_from_this;
    using UnaryOperatorExpression::UnaryOperatorExpression;

    virtual void prettyPrint(int indentation, std::ostream &out) const override;

    virtual void accept(IAstVisitor &visitor) override;
};

class DereferenceExpression : public UnaryOperatorExpression, public EnableSharedAst<DereferenceExpression>
{
public:
    using EnableSharedAst<DereferenceExpression>::shared_from_this;
    using UnaryOperatorExpression::UnaryOperatorExpression;

    virtual void prettyPrint(int indentation, std::ostream &out) const override;

    virtual void accept(IAstVisitor &visitor) override;

    bool isLvalue() const override;
};

class AddressOfExpression : public UnaryOperatorExpression, public EnableSharedAst<AddressOfExpression>
{
public:
    using EnableSharedAst<AddressOfExpression>::shared_from_this;
    using UnaryOperatorExpression::UnaryOperatorExpression;

    virtual void prettyPrint(int indentation, std::ostream &out) const override;

    virtual void accept(IAstVisitor &visitor) override;
};

class MemberExpression : public UnaryOperatorExpression, public EnableSharedAst<MemberExpression>
{
public:
    using EnableSharedAst<MemberExpression>::shared_from_this;
    MemberExpression(std::shared_ptr<Expression> recordExpression, const std::string &memberName);

    virtual void prettyPrint(int indentation, std::ostream &out) const override;

    virtual void accept(IAstVisitor &visitor) override;

    bool isLvalue() const override;

    std::string memberName;

    std::uint32_t index; // TypeChecking
};

// End of expressions

class FunctionBodyImperative : public FunctionBody, public EnableSharedAst<FunctionBodyImperative>
{
public:
    using EnableSharedAst<FunctionBodyImperative>::shared_from_this;
    explicit FunctionBodyImperative(const std::vector<std::shared_ptr<Statement>> &statements);

    virtual ~FunctionBodyImperative() override;

    virtual void prettyPrint(int indentation, std::ostream &out) const override;

    virtual void accept(IAstVisitor &visitor) override;

    std::vector<std::shared_ptr<Statement>> statements;
};

class Statement : public Ast, public EnableSharedAst<Statement>
{
public:
    using EnableSharedAst<Statement>::shared_from_this;
    virtual ~Statement() override;

    virtual void accept(IAstVisitor &visitor) override;
};

class ExpressionStatement : public Statement, public EnableSharedAst<ExpressionStatement>
{
public:
    using EnableSharedAst<ExpressionStatement>::shared_from_this;
    explicit ExpressionStatement(std::shared_ptr<Expression> expression);

    virtual ~ExpressionStatement() override;

    virtual void prettyPrint(int indentation, std::ostream &out) const override;

    virtual void accept(IAstVisitor &visitor) override;

    std::shared_ptr<Expression> expression;
};

class IfStatement : public Statement, public EnableSharedAst<IfStatement>
{
public:
    using EnableSharedAst<IfStatement>::shared_from_this;
    IfStatement(std::shared_ptr<Expression> condition, const std::vector<std::shared_ptr<Statement>> &thenBlock,
                const std::vector<std::shared_ptr<Statement>> &elseBlock = {});

    virtual ~IfStatement() override;

    virtual void prettyPrint(int indentation, std::ostream &out) const override;

    virtual void accept(IAstVisitor &visitor) override;

    std::shared_ptr<Expression> condition;
    std::vector<std::shared_ptr<Statement>> thenBlock;
    std::vector<std::shared_ptr<Statement>> elseBlock;
};

class ForStatement : public Statement, public EnableSharedAst<ForStatement>
{
public:
    using EnableSharedAst<ForStatement>::shared_from_this;

    ForStatement(std::vector<std::shared_ptr<Expression>> &&initExpressions,
                 std::shared_ptr<Expression> &&conditionExpression,
                 std::vector<std::shared_ptr<Expression>> &&lastExpressions,
                 std::vector<std::shared_ptr<Statement>> &&body);

    virtual ~ForStatement() override;

    virtual void prettyPrint(int indentation, std::ostream &out) const override;

    virtual void accept(IAstVisitor &visitor) override;

    std::vector<std::shared_ptr<Expression>> initExpressions;
    std::shared_ptr<Expression> conditionExpression;
    std::vector<std::shared_ptr<Expression>> lastExpressions;
    std::vector<std::shared_ptr<Statement>> body;

};

class BreakStatement : public Statement, public EnableSharedAst<BreakStatement>
{
public:
    using EnableSharedAst<BreakStatement>::shared_from_this;
    virtual ~BreakStatement() override;

    virtual void prettyPrint(int indentation, std::ostream &out) const override;

    virtual void accept(IAstVisitor &visitor) override;
};

class ContinueStatement : public Statement, public EnableSharedAst<ContinueStatement>
{
public:
    using EnableSharedAst<ContinueStatement>::shared_from_this;
    virtual ~ContinueStatement() override;

    virtual void prettyPrint(int indentation, std::ostream &out) const override;

    virtual void accept(IAstVisitor &visitor) override;
};

class FunctionBodyExtern : public FunctionBody, public EnableSharedAst<FunctionBodyExtern>
{
public:
    enum class Abi
    {
        DI, C,
    };

    using EnableSharedAst<FunctionBodyExtern>::shared_from_this;

    explicit FunctionBodyExtern(Abi abi);
    virtual ~FunctionBodyExtern() override;

    std::shared_ptr<FunctionDefinition> getParent() const;
    void setParent(std::shared_ptr<FunctionDefinition> definition);

    Abi getAbi() const;

    virtual void prettyPrint(int indentation, std::ostream &out) const override;

    virtual void accept(IAstVisitor &visitor) override;

private:
    Abi abi;
    std::weak_ptr<FunctionDefinition> parent;
};

template<typename AstNode>
std::shared_ptr<AstNode> EnableSharedAst<AstNode>::shared_from_this()
{
    return std::static_pointer_cast<AstNode>(
            static_cast<AstNode *>(this)->std::enable_shared_from_this<Ast>::shared_from_this());
}

#endif //KOMPILATOR_DECLARATIONS_HXX

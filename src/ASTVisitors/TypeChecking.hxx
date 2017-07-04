#ifndef KOMPILATOR_TYPECHECKING_HXX
#define KOMPILATOR_TYPECHECKING_HXX

#include "AstVisitorBase.hxx"
#include "ErrorCollector.hxx"

#include <unordered_map>

class TypeChecking : public AstVisitorBase, public ErrorCollector
{
public:
    explicit TypeChecking(const std::unordered_map<std::string, std::shared_ptr<BuiltinType>> &builtinTypes);

    void visit(std::shared_ptr<DecimalLiteralExpression> node) override;
    void visit(std::shared_ptr<BooleanLiteralExpression> node) override;
    void visit(std::shared_ptr<NameReferenceExpression> node) override;
    void visit(std::shared_ptr<AtomicTermDefinitionExpression> node) override;
    void visit(std::shared_ptr<FunctionTermExpression> node) override;
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

private:
    std::shared_ptr<BuiltinType> builtinType(const std::string &name) const;

    std::shared_ptr<Type> R(std::shared_ptr<Type> type);
    std::shared_ptr<Type> A(std::shared_ptr<Type> type);
    std::shared_ptr<Type> removeAnnotation(std::shared_ptr<Type> type);
    std::shared_ptr<Type> carryAnnotation(std::shared_ptr<Type> from , std::shared_ptr<Type> to);

    bool isAnnotated(std::shared_ptr<Type> type);
    bool equivalent(std::shared_ptr<Type> lhs, std::shared_ptr<Type> rhs);


    void equalityComparison(std::shared_ptr<BinaryOperatorExpression> node);
    void relationComparison(std::shared_ptr<BinaryOperatorExpression> node);
    void integerArithmetic(std::shared_ptr<BinaryOperatorExpression> node);

    const std::unordered_map<std::string, std::shared_ptr<BuiltinType>> &builtinTypes;
};


#endif //KOMPILATOR_TYPECHECKING_HXX

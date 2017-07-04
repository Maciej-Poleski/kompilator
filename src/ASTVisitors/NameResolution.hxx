#ifndef KOMPILATOR_NAMERESOLUTION_HXX
#define KOMPILATOR_NAMERESOLUTION_HXX

#include "AstVisitorBase.hxx"
#include "ErrorCollector.hxx"

#include <unordered_map>
#include <vector>

class NameResolution : public AstVisitorBase, public ErrorCollector
{
    class ScopeGuard
    {
    public:
        explicit ScopeGuard(NameResolution &that);
        ~ScopeGuard();
    private:
        NameResolution &that;
    };

    class Scope
    {
        friend ScopeGuard;
    public:
        std::shared_ptr<Type> getDefinition(std::shared_ptr<NamedType> namedType) const;
        std::shared_ptr<DefinitionTerm> getDefinition(std::shared_ptr<ReferenceTerm> referenceTerm) const;
        std::shared_ptr<AtomicTermDefinitionExpression> getDefinition(std::shared_ptr<NameReferenceExpression> nameReferenceExpression) const;

        bool hasDefinition(std::shared_ptr<RecordDefinition> recordDefinition) const;
        bool hasDefinition(std::shared_ptr<DefinitionTerm> definitionTerm) const;
        bool hasDefinition(std::shared_ptr<AtomicTermDefinitionExpression> atomicTermDefinitionExpression) const;
        bool hasTypeDefinitionNamed(const std::string &typeDefinitionName);

        void recordDefinition(std::shared_ptr<RecordDefinition> recordDefinition, int oldScope);
        void recordDefinition(std::shared_ptr<DefinitionTerm> definitionTerm, int oldScope);
        void recordDefinition(std::shared_ptr<AtomicTermDefinitionExpression> atomicTermDefinitionExpression, int oldScope);
        void recordDefinition(std::shared_ptr<BuiltinType> builtinType, int oldScope);

    private:
        std::unordered_map<std::string, std::pair<std::shared_ptr<Type>, int>> typeDefinitions;
        std::unordered_map<std::string, std::pair<std::shared_ptr<DefinitionTerm>, int>> definitionTerms;
        std::unordered_map<std::string, std::pair<std::shared_ptr<AtomicTermDefinitionExpression>, int>> atomicTermDefinitionExpressions;
    };

public:
    explicit NameResolution(const std::unordered_map<std::string, std::shared_ptr<BuiltinType>> &builtinTypes,
                            const std::unordered_map<std::string, std::shared_ptr<RecordDefinition>> &recordDefinitions);

    void visit(std::shared_ptr<NamedType> node) override;
    void visit(std::shared_ptr<FunctionDefinitionDeclarative> node) override;
    void visit(std::shared_ptr<ReferenceTerm> node) override;
    void visit(std::shared_ptr<DefinitionTerm> node) override;
    void visit(std::shared_ptr<FunctionDefinitionImperative> node) override;
    void visit(std::shared_ptr<FunctionBodyDeclarative> node) override;
    void visit(std::shared_ptr<NameReferenceExpression> node) override;
    void visit(std::shared_ptr<AtomicTermDefinitionExpression> node) override;
    void visit(std::shared_ptr<FunctionBodyImperative> node) override;
    void visit(std::shared_ptr<IfStatement> node) override;
    void visit(std::shared_ptr<ForStatement> node) override;

private:

    std::shared_ptr<Type> getDefinition(std::shared_ptr<NamedType> namedType) const;
    std::shared_ptr<DefinitionTerm> getDefinition(std::shared_ptr<ReferenceTerm> referenceTerm) const;
    std::shared_ptr<AtomicTermDefinitionExpression> getDefinition(std::shared_ptr<NameReferenceExpression> nameReferenceExpression) const;

    ScopeGuard makeNewScope();

    void recordDefinition(std::shared_ptr<DefinitionTerm> definitionTerm);
    void recordDefinition(std::shared_ptr<AtomicTermDefinitionExpression> atomicTermDefinitionExpression);
    void recordDefinition(std::shared_ptr<RecordDefinition> recordDefinition);
    void recordDefinition(std::shared_ptr<BuiltinType> builtinType);

    std::vector<Scope> scopes;
    std::unordered_map<std::string, int> scopeOf;
};


#endif //KOMPILATOR_NAMERESOLUTION_HXX

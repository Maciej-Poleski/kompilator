#ifndef KOMPILATOR_NAMEMANGLER_HXX
#define KOMPILATOR_NAMEMANGLER_HXX

#include "AstVisitorBase.hxx"

class NameMangler : public AstVisitorBase
{
public:
    void visit(std::shared_ptr<RecordDefinition> node) override;
    void visit(std::shared_ptr<IntType> node) override;
    void visit(std::shared_ptr<BoolType> node) override;
    void visit(std::shared_ptr<VarType> node) override;
    void visit(std::shared_ptr<NamedType> node) override;
    void visit(std::shared_ptr<AnnotatedType> node) override;
    void visit(std::shared_ptr<PtrType> node) override;
    void visit(std::shared_ptr<FunctionDefinitionDeclarative> node) override;
    void visit(std::shared_ptr<FunctionDefinitionImperative> node) override;
    void visit(std::shared_ptr<AtomicTermDefinitionExpression> node) override;
    void visit(std::shared_ptr<FunctionTermExpression> node) override;

    static std::string getMangledName(std::shared_ptr<Ast> node);

private:
    std::string mangledName;
};


#endif //KOMPILATOR_NAMEMANGLER_HXX

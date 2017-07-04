#ifndef KOMPILATOR_BASICCORRECTNESS_HXX
#define KOMPILATOR_BASICCORRECTNESS_HXX

#include "AstVisitorBase.hxx"
#include "ErrorCollector.hxx"

class BasicCorrectness : public AstVisitorBase, public ErrorCollector
{
public:
    void visit(std::shared_ptr<AssignExpression> node) override;

    void visit(std::shared_ptr<FunctionDefinitionDeclarative> node) override;

    void visit(std::shared_ptr<FunctionDefinitionImperative> node) override;
};


#endif //KOMPILATOR_BASICCORRECTNESS_HXX

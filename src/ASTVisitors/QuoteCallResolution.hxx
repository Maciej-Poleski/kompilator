#ifndef KOMPILATOR_QUOTECALLRESOLUTION_HXX
#define KOMPILATOR_QUOTECALLRESOLUTION_HXX

#include "AstVisitorBase.hxx"
#include "ErrorCollector.hxx"

class QuoteCallResolution : public AstVisitorBase, public ErrorCollector
{
public:
    void visit(std::shared_ptr<FunctionBodyDeclarative> node) override;
    void visit(std::shared_ptr<ExplicitCallExpression> node) override;
    void visit(std::shared_ptr<QuoteExpression> node) override;
    void visit(std::shared_ptr<FunctionBodyImperative> node) override;

    void visit(std::shared_ptr<FunctionTermExpression> node) override;
private:
    bool isCallMode;
};


#endif //KOMPILATOR_QUOTECALLRESOLUTION_HXX

#ifndef KOMPILATOR_ERRORCOLLECTOR_HXX
#define KOMPILATOR_ERRORCOLLECTOR_HXX

#include <vector>
#include <string>
#include <memory>

#include <src/AST/DeclarationsFwd.hxx>

class ErrorCollector
{
public:
    bool hasErrors() const;
    const std::vector<std::string>& getErrors() const;

protected:
    void reportError(std::shared_ptr<Ast> node, const std::string &message);
    void reportError(Ast *node, const std::string &message);

private:
    std::vector<std::string> errors;
};


#endif //KOMPILATOR_ERRORCOLLECTOR_HXX

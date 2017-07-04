#ifndef KOMPILATOR_COMPILER_HXX
#define KOMPILATOR_COMPILER_HXX

#include <memory>
#include <vector>
#include <unordered_map>

#include "AST/DeclarationsFwd.hxx"

class Compiler
{
public:
    explicit Compiler(std::shared_ptr<Module> module, const std::string &name);

    bool compile();

private:
    void printErrors(const std::vector<std::string> &errors) const;

    template<typename AstVisitor, typename... Args>
    bool runAstVisitor(Args... args);

private:
    std::shared_ptr<Module> module;
    std::unordered_map<std::string, std::shared_ptr<BuiltinType>> builtinTypes;
    std::string name;
};


#endif //KOMPILATOR_COMPILER_HXX

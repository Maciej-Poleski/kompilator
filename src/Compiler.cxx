#include "Compiler.hxx"

#include <iostream>

#include "src/AST/Declarations.hxx"
#include "src/ASTVisitors/BasicCorrectness.hxx"
#include "src/ASTVisitors/NameResolution.hxx"
#include "src/ASTVisitors/QuoteCallResolution.hxx"
#include "src/ASTVisitors/RecordDefinitionsCollector.hxx"
#include "src/ASTVisitors/TypeChecking.hxx"
#include "src/ASTVisitors/Codegen.hxx"

Compiler::Compiler(std::shared_ptr<Module> module, const std::string &name)
        : module(module), name(name)
{
    builtinTypes["var"] = std::make_shared<VarType>();
    builtinTypes["int"] = std::make_shared<IntType>();
    builtinTypes["bool"] = std::make_shared<BoolType>();
    builtinTypes["dynamic"] = std::make_shared<DynamicType>();
    builtinTypes["function"] = std::make_shared<FunctionType>();
}

bool Compiler::compile()
{
    module->dump(std::cout);
    if (runAstVisitor<BasicCorrectness>()) {
        return true;
    }
    if (runAstVisitor<QuoteCallResolution>()) {
        return true;
    }
    std::unordered_map<std::string, std::shared_ptr<RecordDefinition>> recordDefinitions;
    if (runAstVisitor<RecordDefinitionsCollector>(std::ref(recordDefinitions))) {
        return true;
    }
    if (runAstVisitor<NameResolution>(std::cref(builtinTypes), std::cref(recordDefinitions))) {
        return true;
    }
    if (runAstVisitor<TypeChecking>(std::cref(builtinTypes))) {
        return true;
    }
    if(runAstVisitor<Codegen>(name)) {
        return true;
    }
    return false;
}

void Compiler::printErrors(const std::vector<std::string> &errors) const
{
    for (auto &error : errors) {
        std::cerr << error << '\n';
    }
}

template<typename AstVisitor, typename... Args>
bool Compiler::runAstVisitor(Args... args)
{
    AstVisitor visitor(args...);
    module->accept(visitor);
    if (visitor.hasErrors()) {
        printErrors(visitor.getErrors());
        return true;
    }
    return false;
}

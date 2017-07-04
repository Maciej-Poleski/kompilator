#include "Declarations.hxx"

#include "src/ASTVisitors/IAstVisitor.hxx"

FunctionDefinition::FunctionDefinition(const std::string &name)
        : name(name)
{}

FunctionDefinition::~FunctionDefinition() = default;

void FunctionDefinition::accept(IAstVisitor &visitor)
{
    visitor.visit(shared_from_this());
}

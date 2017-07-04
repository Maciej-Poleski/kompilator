#include "Declarations.hxx"

#include "src/ASTVisitors/IAstVisitor.hxx"

BuiltinType::BuiltinType(const std::string &name) : name(name)
{}

BuiltinType::~BuiltinType() = default;

void BuiltinType::accept(IAstVisitor &visitor)
{
    visitor.visit(shared_from_this());
}

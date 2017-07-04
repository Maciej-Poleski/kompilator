#include "Declarations.hxx"

#include <ostream>

#include "src/ASTVisitors/IAstVisitor.hxx"

FunctionBodyExtern::FunctionBodyExtern(Abi abi)
        : abi(abi)
{}

FunctionBodyExtern::~FunctionBodyExtern() = default;

std::shared_ptr<FunctionDefinition> FunctionBodyExtern::getParent() const
{
    return parent.lock();
}

void FunctionBodyExtern::setParent(std::shared_ptr<FunctionDefinition> definition)
{
    parent = definition;
}

FunctionBodyExtern::Abi FunctionBodyExtern::getAbi() const
{
    return abi;
}

void FunctionBodyExtern::prettyPrint(int indentation, std::ostream &out) const
{
    (void) indentation;
    out << "extern";
    if (abi == Abi::C) {
        out << " \"C\"";
    }
}

void FunctionBodyExtern::accept(IAstVisitor &visitor)
{
    visitor.visit(shared_from_this());
}

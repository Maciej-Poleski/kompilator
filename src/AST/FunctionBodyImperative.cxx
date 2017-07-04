#include "Declarations.hxx"

#include <ostream>

#include "src/ASTVisitors/IAstVisitor.hxx"

FunctionBodyImperative::FunctionBodyImperative(const std::vector<std::shared_ptr<Statement>> &statements)
        : statements(statements)
{}

FunctionBodyImperative::~FunctionBodyImperative() = default;

void FunctionBodyImperative::prettyPrint(int indentation, std::ostream &out) const
{
    out << "{\n";
    for (auto &s : statements) {
        s->prettyPrint(indentation + 4, out);
    }
    out << '}';
}

void FunctionBodyImperative::accept(IAstVisitor &visitor)
{
    visitor.visit(shared_from_this());
}

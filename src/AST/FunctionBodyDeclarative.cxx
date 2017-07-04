#include "Declarations.hxx"

#include <ostream>

#include "src/ASTVisitors/IAstVisitor.hxx"

FunctionBodyDeclarative::FunctionBodyDeclarative(const std::vector<std::shared_ptr<Expression>> &body)
        : body(body)
{}

FunctionBodyDeclarative::~FunctionBodyDeclarative() = default;

void FunctionBodyDeclarative::prettyPrint(int indentation, std::ostream &out) const
{
    out << ":- ";
    for (int i = 0; i < body.size(); ++i) {
        body[i]->prettyPrint(indentation + 4, out);
        if (i < body.size() - 1) {
            out << ", ";
        }
    }
    out << ";";
}

void FunctionBodyDeclarative::accept(IAstVisitor &visitor)
{
    visitor.visit(shared_from_this());
}

#include "Declarations.hxx"

#include <ostream>

#include "src/ASTVisitors/IAstVisitor.hxx"

FunctionDefinitionDeclarative::FunctionDefinitionDeclarative(const std::string name,
                                                             const std::vector<std::shared_ptr<Expression>> &terms,
                                                             const std::shared_ptr<FunctionBody> &functionBody)
        : FunctionDefinition(name), terms(terms), functionBody(functionBody)
{}

FunctionDefinitionDeclarative::~FunctionDefinitionDeclarative() = default;

void FunctionDefinitionDeclarative::prettyPrint(int indentation, std::ostream &out) const
{
    out << name << '(';
    for (int i = 0; i < terms.size(); ++i) {
        terms[i]->prettyPrint(indentation + 4, out);
        if (i < terms.size() - 1) {
            out << ", ";
        }
    }
    out << ") ";
    functionBody->prettyPrint(indentation + 4, out);
    out << '\n';
}

void FunctionDefinitionDeclarative::accept(IAstVisitor &visitor)
{
    visitor.visit(shared_from_this());
}

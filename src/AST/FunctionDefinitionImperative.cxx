#include "Declarations.hxx"

#include <ostream>

#include "src/ASTVisitors/IAstVisitor.hxx"

FunctionDefinitionImperative::FunctionDefinitionImperative(const std::string &name, std::shared_ptr<Expression> result,
                                                           const std::vector<std::shared_ptr<Expression>> &terms,
                                                           std::shared_ptr<FunctionBody> functionBody)
        : FunctionDefinition(name), result(result), terms(terms), functionBody(functionBody)
{

}

FunctionDefinitionImperative::~FunctionDefinitionImperative() = default;

void FunctionDefinitionImperative::prettyPrint(int indentation, std::ostream &out) const
{
    if (result) {
        result->prettyPrint(indentation, out);
    } else {
        out << "void";
    }
    out << " = ";
    out << name << "(";
    for (int i = 0; i < terms.size(); ++i) {
        terms[i]->prettyPrint(indentation + 4, out);
        if (i < terms.size() - 1) {
            out << ", ";
        }
    }
    out << ") ";
    functionBody->prettyPrint(indentation, out);
    out << '\n';
}

void FunctionDefinitionImperative::accept(IAstVisitor &visitor)
{
    visitor.visit(shared_from_this());
}

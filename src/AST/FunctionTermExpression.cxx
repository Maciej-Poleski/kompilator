#include "Declarations.hxx"

#include <ostream>

#include "utils.hxx"
#include "src/ASTVisitors/IAstVisitor.hxx"

FunctionTermExpression::FunctionTermExpression(const std::string &name,
                                               const std::vector<std::shared_ptr<Expression>> &subterms)
        : name(name), subterms(subterms), isCall(false)
{}

FunctionTermExpression::~FunctionTermExpression() = default;

void FunctionTermExpression::prettyPrint(int indentation, std::ostream &out) const
{
    out << name << "(";
    prettyPrintAstContainer(subterms.begin(), subterms.end(), indentation + 4, out, ", ");
    out << ")";
}

void FunctionTermExpression::accept(IAstVisitor &visitor)
{
    visitor.visit(shared_from_this());
}

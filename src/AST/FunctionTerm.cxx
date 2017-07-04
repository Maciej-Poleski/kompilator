#include "Declarations.hxx"

#include <ostream>

#include "src/ASTVisitors/IAstVisitor.hxx"

FunctionTerm::FunctionTerm(const std::string &name, const std::vector<std::shared_ptr<Term>> &subterms) : name(name),
                                                                                                          subterms(
                                                                                                                  subterms)
{}

FunctionTerm::~FunctionTerm() = default;

void FunctionTerm::prettyPrint(int indentation, std::ostream &out) const
{
    out << name << '(';
    for (int i = 0; i < subterms.size(); ++i) {
        subterms[i]->prettyPrint(indentation + 4, out);
        if (i < subterms.size() - 1) {
            out << ", ";
        }
    }
    out << ')';
}

void FunctionTerm::accept(IAstVisitor &visitor)
{
    visitor.visit(shared_from_this());
}

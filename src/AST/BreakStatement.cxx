#include "Declarations.hxx"

#include <ostream>

#include "src/ASTVisitors/IAstVisitor.hxx"

BreakStatement::~BreakStatement() = default;

void BreakStatement::prettyPrint(int indentation, std::ostream &out) const
{
    out << std::string(indentation, ' ');
    out << "break;\n";
}

void BreakStatement::accept(IAstVisitor &visitor)
{
    visitor.visit(shared_from_this());
}

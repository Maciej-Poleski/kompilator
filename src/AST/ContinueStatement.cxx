#include "Declarations.hxx"

#include <ostream>

#include "src/ASTVisitors/IAstVisitor.hxx"

ContinueStatement::~ContinueStatement() = default;

void ContinueStatement::prettyPrint(int indentation, std::ostream &out) const
{
    out << std::string(indentation, ' ');
    out << "continue;\n";
}

void ContinueStatement::accept(IAstVisitor &visitor)
{
    visitor.visit(shared_from_this());
}

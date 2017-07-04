#include "Declarations.hxx"

#include <ostream>

#include "src/ASTVisitors/IAstVisitor.hxx"

ExpressionStatement::ExpressionStatement(std::shared_ptr<Expression> expression)
        : expression(expression)
{}

ExpressionStatement::~ExpressionStatement() = default;

void ExpressionStatement::prettyPrint(int indentation, std::ostream &out) const
{
    out << std::string(indentation, ' ');
    expression->prettyPrint(indentation, out);
    out << ";\n";
}

void ExpressionStatement::accept(IAstVisitor &visitor)
{
    visitor.visit(shared_from_this());
}

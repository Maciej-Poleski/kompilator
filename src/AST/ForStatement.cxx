#include "Declarations.hxx"

#include <ostream>

#include "utils.hxx"

#include "src/ASTVisitors/IAstVisitor.hxx"

ForStatement::ForStatement(std::vector<std::shared_ptr<Expression>> &&initExpressions,
                           std::shared_ptr<Expression> &&conditionExpression,
                           std::vector<std::shared_ptr<Expression>> &&lastExpressions,
                           std::vector<std::shared_ptr<Statement>> &&body)
        : initExpressions(std::move(initExpressions)), conditionExpression(std::move(conditionExpression)),
          lastExpressions(std::move(lastExpressions)), body(std::move(body))
{}

ForStatement::~ForStatement() = default;

void ForStatement::prettyPrint(int indentation, std::ostream &out) const
{
    out << std::string(indentation, ' ');
    out << "for(";
    prettyPrintAstContainer(initExpressions.begin(), initExpressions.end(), indentation + 4, out, ", ");
    out << ";";
    if (conditionExpression) {
        out << " ";
        conditionExpression->prettyPrint(indentation + 4, out);
    }
    out << ";";
    if (!lastExpressions.empty()) {
        out << " ";
        prettyPrintAstContainer(lastExpressions.begin(), lastExpressions.end(), indentation + 4, out, ", ");
    }
    out << ") {\n";
    for (auto &s : body) {
        s->prettyPrint(indentation + 4, out);
    }
    out << std::string(indentation, ' ');
    out << "}\n";
}

void ForStatement::accept(IAstVisitor &visitor)
{
    visitor.visit(shared_from_this());
}

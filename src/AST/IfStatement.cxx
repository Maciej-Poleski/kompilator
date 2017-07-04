#include "Declarations.hxx"

#include <ostream>

#include "src/ASTVisitors/IAstVisitor.hxx"

IfStatement::IfStatement(std::shared_ptr<Expression> condition,
                         const std::vector<std::shared_ptr<Statement>> &thenBlock,
                         const std::vector<std::shared_ptr<Statement>> &elseBlock)
        : condition(condition), thenBlock(thenBlock), elseBlock(elseBlock)
{}

IfStatement::~IfStatement() = default;

void IfStatement::prettyPrint(int indentation, std::ostream &out) const
{
    out << std::string(indentation, ' ');
    out << "if(";
    condition->prettyPrint(indentation + 4, out);
    out << ") {\n";
    for (auto &s : thenBlock) {
        s->prettyPrint(indentation + 4, out);
    }
    out << std::string(indentation, ' ');
    out << "}";
    if (!elseBlock.empty()) {
        out << " else {\n";
        for (auto &s : elseBlock) {
            s->prettyPrint(indentation + 4, out);
        }
        out << std::string(indentation, ' ');
        out << "}";
    }
    out << '\n';
}

void IfStatement::accept(IAstVisitor &visitor)
{
    visitor.visit(shared_from_this());
}

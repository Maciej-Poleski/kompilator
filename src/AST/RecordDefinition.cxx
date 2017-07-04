#include "Declarations.hxx"

#include <ostream>

#include "src/ASTVisitors/IAstVisitor.hxx"

RecordDefinition::RecordDefinition(const std::string &name,
                                   const std::vector<std::pair<std::shared_ptr<Type>, std::string>> &fields) : name(
        name), fields(fields)
{}

RecordDefinition::~RecordDefinition() = default;

void RecordDefinition::prettyPrint(int indentation, std::ostream &out) const
{
    out << "struct " << name << "\n{\n";
    for (auto &p : fields) {
        out << std::string(indentation + 4, ' ');
        p.first->prettyPrint(indentation + 4, out);
        out << " " << p.second << ";\n";
    }
    out << "}\n";
}

void RecordDefinition::accept(IAstVisitor &visitor)
{
    visitor.visit(shared_from_this());
}

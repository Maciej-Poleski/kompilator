#include <src/AST/Declarations.hxx>
#include "RecordDefinitionsCollector.hxx"

RecordDefinitionsCollector::RecordDefinitionsCollector(
        std::unordered_map<std::string, std::shared_ptr<RecordDefinition>> &result)
        : result(result)
{}

void RecordDefinitionsCollector::visit(std::shared_ptr<RecordDefinition> node)
{
    if (result.find(node->name) != result.end()) {
        reportError(node, "Record redefinition");
    } else {
        result[node->name] = node;
    }
    AstVisitorBase::visit(node);
}

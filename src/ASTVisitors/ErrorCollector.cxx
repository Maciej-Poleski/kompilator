#include "ErrorCollector.hxx"

#include <sstream>

#include "src/AST/Declarations.hxx"

void ErrorCollector::reportError(std::shared_ptr<Ast> node, const std::string &message)
{
    reportError(node.get(), message);
}

void ErrorCollector::reportError(Ast *node, const std::string &message)
{
    // FIXME: report location
    std::ostringstream nodeDump;
    if (node) {
        node->dump(nodeDump);
    }
    errors.push_back(nodeDump.str() + ": " + message);
}

const std::vector<std::string> &ErrorCollector::getErrors() const
{
    return errors;
}

bool ErrorCollector::hasErrors() const
{
    return !errors.empty();
}

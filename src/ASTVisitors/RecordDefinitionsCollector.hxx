#ifndef KOMPILATOR_RECORDDEFINITIONSCOLLECTOR_HXX
#define KOMPILATOR_RECORDDEFINITIONSCOLLECTOR_HXX

#include "AstVisitorBase.hxx"
#include "ErrorCollector.hxx"

#include <unordered_map>

class RecordDefinitionsCollector : public AstVisitorBase, public ErrorCollector
{
public:
    explicit RecordDefinitionsCollector(std::unordered_map<std::string, std::shared_ptr<RecordDefinition>> &result);

    void visit(std::shared_ptr<RecordDefinition> node) override;

private:
    std::unordered_map<std::string, std::shared_ptr<RecordDefinition>> &result;
};


#endif //KOMPILATOR_RECORDDEFINITIONSCOLLECTOR_HXX

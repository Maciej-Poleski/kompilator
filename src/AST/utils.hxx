#ifndef KOMPILATOR_UTILS_HXX
#define KOMPILATOR_UTILS_HXX

#include <ostream>

#include "Declarations.hxx"

class ForStatementBuilder
{
public:
    explicit ForStatementBuilder(const std::vector<std::shared_ptr<Statement>> &body);

    ForStatementBuilder &&withInit(const std::vector<std::shared_ptr<Expression>> &initExpressions) &&;

    ForStatementBuilder &&withCondition(std::shared_ptr<Expression> conditionExpression) &&;

    ForStatementBuilder &&withLast(const std::vector<std::shared_ptr<Expression>> &lastExpressions) &&;

    std::shared_ptr<ForStatement> build() &&;

private:
    std::vector<std::shared_ptr<Statement>> body;
    std::vector<std::shared_ptr<Expression>> initExpressions;
    std::shared_ptr<Expression> conditionExpression;
    std::vector<std::shared_ptr<Expression>> lastExpressions;
};

template<class AstIterator>
void prettyPrintAstContainer(AstIterator begin, AstIterator end, int indentation, std::ostream &out,
                             const std::string &separator)
{
    if (begin == end) {
        return;
    }
    for (;;) {
        (*begin)->prettyPrint(indentation, out);
        ++begin;
        if (begin != end) {
            out << separator;
        } else {
            break;
        }
    }
}

#endif //KOMPILATOR_UTILS_HXX

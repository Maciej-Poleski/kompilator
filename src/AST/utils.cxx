#include "utils.hxx"

ForStatementBuilder::ForStatementBuilder(const std::vector<std::shared_ptr<Statement>> &body)
        : body(body)
{
}

ForStatementBuilder &&ForStatementBuilder::withInit(const std::vector<std::shared_ptr<Expression>> &initExpressions) &&
{
    this->initExpressions = initExpressions;
    return std::move(*this);
}

ForStatementBuilder &&ForStatementBuilder::withCondition(std::shared_ptr<Expression> conditionExpression) &&
{
    this->conditionExpression = conditionExpression;
    return std::move(*this);
}

ForStatementBuilder &&ForStatementBuilder::withLast(const std::vector<std::shared_ptr<Expression>> &lastExpressions) &&
{
    this->lastExpressions = lastExpressions;
    return std::move(*this);
}

std::shared_ptr<ForStatement> ForStatementBuilder::build() &&
{
    return std::make_shared<ForStatement>(std::move(initExpressions), std::move(conditionExpression),
                                          std::move(lastExpressions), std::move(body));
}

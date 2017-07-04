#include <src/AST/Declarations.hxx>
#include <cassert>
#include <iostream>
#include "NameResolution.hxx"

NameResolution::NameResolution(const std::unordered_map<std::string, std::shared_ptr<BuiltinType>> &builtinTypes,
                               const std::unordered_map<std::string, std::shared_ptr<RecordDefinition>> &recordDefinitions)
{
    scopes.push_back({});
    for (auto &p :builtinTypes) {
        recordDefinition(p.second);
    }
    for (auto &p : recordDefinitions) {
        recordDefinition(p.second);
    }
}

void NameResolution::visit(std::shared_ptr<NamedType> node)
{
    auto definition = getDefinition(node);
    if (!definition) {
        reportError(node, "Undefined type");
    } else {
        node->typeDefinition = definition;
    }
}

void NameResolution::visit(std::shared_ptr<FunctionDefinitionDeclarative> node)
{
    auto S = makeNewScope();
    AstVisitorBase::visit(node);
}

void NameResolution::visit(std::shared_ptr<ReferenceTerm> node)
{
    auto definition = getDefinition(node);
    if (!definition) {
        reportError(node, "Undefined term");
    } else {
        node->definitionTerm = definition;
    }
}

void NameResolution::visit(std::shared_ptr<DefinitionTerm> node)
{
    AstVisitorBase::visit(node);
    recordDefinition(node);
}

void NameResolution::visit(std::shared_ptr<FunctionDefinitionImperative> node)
{
    auto S = makeNewScope();
    AstVisitorBase::visit(node);
}

void NameResolution::visit(std::shared_ptr<FunctionBodyDeclarative> node)
{
    auto S = makeNewScope();
    AstVisitorBase::visit(node);
}

void NameResolution::visit(std::shared_ptr<NameReferenceExpression> node)
{
    auto definition = getDefinition(node);
    if (!definition) {
        reportError(node, "Undefined term");
    } else {
        node->atomicTermDefinitionExpression = definition;
    }
}

void NameResolution::visit(std::shared_ptr<AtomicTermDefinitionExpression> node)
{
    AstVisitorBase::visit(node);
    recordDefinition(node);
}

void NameResolution::visit(std::shared_ptr<FunctionBodyImperative> node)
{
    auto S = makeNewScope();
    AstVisitorBase::visit(node);
}

void NameResolution::visit(std::shared_ptr<IfStatement> node)
{
    auto S = makeNewScope();
    node->condition->accept(*this);
    {
        auto S = makeNewScope();
        for (auto &n : node->thenBlock) {
            n->accept(*this);
        }
    }
    {
        auto S = makeNewScope();
        for (auto &n : node->elseBlock) {
            n->accept(*this);
        }
    }
}

void NameResolution::visit(std::shared_ptr<ForStatement> node)
{
    auto S = makeNewScope();
    for (auto &n : node->initExpressions) {
        n->accept(*this);
    }
    {
        auto S = makeNewScope();
        if (node->conditionExpression) {
            node->conditionExpression->accept(*this);
        }
        {
            auto S = makeNewScope();
            for (auto &n : node->lastExpressions) {
                n->accept(*this);
            }
        }
        {
            auto S = makeNewScope();
            for (auto &n : node->body) {
                n->accept(*this);
            }
        }
    }
}

std::shared_ptr<Type> NameResolution::getDefinition(std::shared_ptr<NamedType> namedType) const
{
    auto &name = namedType->name;
    auto scope = scopeOf.find(name);
    if (scope == scopeOf.end()) {
        return nullptr;
    } else {
        return scopes[scope->second].getDefinition(namedType);
    }
}

std::shared_ptr<DefinitionTerm> NameResolution::getDefinition(std::shared_ptr<ReferenceTerm> referenceTerm) const
{
    auto &name = referenceTerm->name;
    auto scope = scopeOf.find(name);
    if (scope == scopeOf.end()) {
        return nullptr;
    } else {
        return scopes[scope->second].getDefinition(referenceTerm);
    }
}

std::shared_ptr<AtomicTermDefinitionExpression>
NameResolution::getDefinition(std::shared_ptr<NameReferenceExpression> nameReferenceExpression) const
{
    auto &name = nameReferenceExpression->name;
    auto scope = scopeOf.find(name);
    if (scope == scopeOf.end()) {
        return nullptr;
    } else {
        return scopes[scope->second].getDefinition(nameReferenceExpression);
    }
}

NameResolution::ScopeGuard NameResolution::makeNewScope()
{
    return ScopeGuard(*this);
}

void NameResolution::recordDefinition(std::shared_ptr<DefinitionTerm> definitionTerm)
{
    if (scopes.back().hasDefinition(definitionTerm)) {
        reportError(definitionTerm, "Redefinition");
    } else {
        auto &name = definitionTerm->name;
        int oldScope = -1;
        if (scopeOf.find(name) != scopeOf.end()) {
            oldScope = scopeOf[name];
        }
        scopes.back().recordDefinition(definitionTerm, oldScope);
        scopeOf[name] = static_cast<int>(scopes.size() - 1);
    }
}

void NameResolution::recordDefinition(std::shared_ptr<AtomicTermDefinitionExpression> atomicTermDefinitionExpression)
{
    if (scopes.back().hasDefinition(atomicTermDefinitionExpression)) {
        reportError(atomicTermDefinitionExpression, "Redefinition");
    } else {
        auto &name = atomicTermDefinitionExpression->name;
        int oldScope = -1;
        if (scopeOf.find(name) != scopeOf.end()) {
            oldScope = scopeOf[name];
        }
        scopes.back().recordDefinition(atomicTermDefinitionExpression, oldScope);
        scopeOf[name] = static_cast<int>(scopes.size() - 1);
    }
}

void NameResolution::recordDefinition(std::shared_ptr<RecordDefinition> recordDefinition)
{
    if (scopes.back().hasDefinition(recordDefinition)) {
        reportError(recordDefinition, "Redefinition");
    } else {
        auto &name = recordDefinition->name;
        int oldScope = -1;
        if (scopeOf.find(name) != scopeOf.end()) {
            oldScope = scopeOf[name];
        }
        scopes.back().recordDefinition(recordDefinition, oldScope);
        scopeOf[name] = static_cast<int>(scopes.size() - 1);
    }
}

void NameResolution::recordDefinition(std::shared_ptr<BuiltinType> builtinType)
{
    auto &name = builtinType->name;
    assert(!scopes.back().hasTypeDefinitionNamed(name));
    int oldScope = -1;
    if (scopeOf.find(name) != scopeOf.end()) {
        oldScope = scopeOf[name];
    }
    scopes.back().recordDefinition(builtinType, oldScope);
    scopeOf[name] = static_cast<int>(scopes.size() - 1);
}

std::shared_ptr<Type> NameResolution::Scope::getDefinition(std::shared_ptr<NamedType> namedType) const
{
    return typeDefinitions.find(namedType->name)->second.first;
}

std::shared_ptr<DefinitionTerm> NameResolution::Scope::getDefinition(std::shared_ptr<ReferenceTerm> referenceTerm) const
{
    return definitionTerms.find(referenceTerm->name)->second.first;
}

std::shared_ptr<AtomicTermDefinitionExpression>
NameResolution::Scope::getDefinition(std::shared_ptr<NameReferenceExpression> nameReferenceExpression) const
{
    return atomicTermDefinitionExpressions.find(nameReferenceExpression->name)->second.first;
}

bool NameResolution::Scope::hasDefinition(std::shared_ptr<RecordDefinition> recordDefinition) const
{
    return typeDefinitions.find(recordDefinition->name) != typeDefinitions.end();
}

bool NameResolution::Scope::hasDefinition(std::shared_ptr<DefinitionTerm> definitionTerm) const
{
    return definitionTerms.find(definitionTerm->name) != definitionTerms.end();
}

bool NameResolution::Scope::hasDefinition(
        std::shared_ptr<AtomicTermDefinitionExpression> atomicTermDefinitionExpression) const
{
    return atomicTermDefinitionExpressions.find(atomicTermDefinitionExpression->name) !=
           atomicTermDefinitionExpressions.end();
}

bool NameResolution::Scope::hasTypeDefinitionNamed(const std::string &typeDefinitionName)
{
    return typeDefinitions.find(typeDefinitionName) != typeDefinitions.end();
}

void NameResolution::Scope::recordDefinition(std::shared_ptr<RecordDefinition> recordDefinition, int oldScope)
{
    typeDefinitions[recordDefinition->name] = {recordDefinition, oldScope};
}

void NameResolution::Scope::recordDefinition(std::shared_ptr<DefinitionTerm> definitionTerm, int oldScope)
{
    definitionTerms[definitionTerm->name] = {definitionTerm, oldScope};
}

void
NameResolution::Scope::recordDefinition(std::shared_ptr<AtomicTermDefinitionExpression> atomicTermDefinitionExpression,
                                        int oldScope)
{
    atomicTermDefinitionExpressions[atomicTermDefinitionExpression->name] = {atomicTermDefinitionExpression, oldScope};
}

void NameResolution::Scope::recordDefinition(std::shared_ptr<BuiltinType> builtinType, int oldScope)
{
    typeDefinitions[builtinType->name] = {builtinType, oldScope};
}

NameResolution::ScopeGuard::ScopeGuard(NameResolution &that)
        : that(that)
{
    that.scopes.push_back({});
}

NameResolution::ScopeGuard::~ScopeGuard()
{
    auto &scope = that.scopes.back();
    for (auto &p : scope.typeDefinitions) {
        that.scopeOf[p.first] = p.second.second;
    }
    for (auto &p : scope.definitionTerms) {
        that.scopeOf[p.first] = p.second.second;
    }
    for (auto &p : scope.atomicTermDefinitionExpressions) {
        that.scopeOf[p.first] = p.second.second;
    }
    that.scopes.pop_back();
}

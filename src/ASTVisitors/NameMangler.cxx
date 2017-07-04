#include "NameMangler.hxx"

#include "src/AST/Declarations.hxx"

void NameMangler::visit(std::shared_ptr<IntType> node)
{
    (void) node;
    mangledName += "I";
}

void NameMangler::visit(std::shared_ptr<BoolType> node)
{
    (void) node;
    mangledName += "B";
}

void NameMangler::visit(std::shared_ptr<VarType> node)
{
    (void) node;
    mangledName += "V";
}

void NameMangler::visit(std::shared_ptr<NamedType> node)
{
    node->typeDefinition.lock()->accept(*this);
}

void NameMangler::visit(std::shared_ptr<AnnotatedType> node)
{
    mangledName += "A";
    node->type->accept(*this);
}

void NameMangler::visit(std::shared_ptr<PtrType> node)
{
    mangledName += "P";
    node->type->accept(*this);
}

void NameMangler::visit(std::shared_ptr<FunctionDefinitionDeclarative> node)
{
    mangledName += "DI_D";
    mangledName += std::to_string(node->name.length());
    mangledName += node->name;
    mangledName += std::to_string(node->terms.size());
    for (auto term : node->terms) {
        term->accept(*this);
    }
}

void NameMangler::visit(std::shared_ptr<FunctionDefinitionImperative> node)
{
    mangledName += "DI_I";
    mangledName += std::to_string(node->name.length());
    mangledName += node->name;
    if (node->result) {
        node->result->accept(*this);
    } else {
        mangledName += "V";
    }
    mangledName += std::to_string(node->terms.size());
    for (auto term : node->terms) {
        term->accept(*this);
    }
}

void NameMangler::visit(std::shared_ptr<AtomicTermDefinitionExpression> node)
{
    node->type->accept(*this);
}

void NameMangler::visit(std::shared_ptr<FunctionTermExpression> node)
{
    mangledName += "S";
    mangledName += std::to_string(node->name.length());
    mangledName += node->name;
    mangledName += std::to_string(node->subterms.size());
    for (auto subterm : node->subterms) {
        subterm->accept(*this);
    }
}

void NameMangler::visit(std::shared_ptr<RecordDefinition> node)
{
    mangledName += "R";
    mangledName += std::to_string(node->name.length());
    mangledName += node->name;
    mangledName += std::to_string(node->fields.size());
    for (auto field : node->fields) {
        field.first->accept(*this);
    }
}

std::string NameMangler::getMangledName(std::shared_ptr<Ast> node)
{
    NameMangler m;
    node->accept(m);
    return std::move(m.mangledName);
}

#include "TypeChecking.hxx"

#include <cassert>
#include <sstream>
#include <iostream>

#include "src/AST/Declarations.hxx"

template<class NodeType>
static bool atomicTypeEquivalent(std::shared_ptr<NodeType> lhs, std::shared_ptr<Type> rhs);

TypeChecking::TypeChecking(const std::unordered_map<std::string, std::shared_ptr<BuiltinType>> &builtinTypes)
        : builtinTypes(builtinTypes)
{
}

void TypeChecking::visit(std::shared_ptr<DecimalLiteralExpression> node)
{
    AstVisitorBase::visit(node);
    node->type = A(builtinType("int"));
}

void TypeChecking::visit(std::shared_ptr<BooleanLiteralExpression> node)
{
    AstVisitorBase::visit(node);
    node->type = A(builtinType("bool"));
}

void TypeChecking::visit(std::shared_ptr<NameReferenceExpression> node)
{
    AstVisitorBase::visit(node);
    node->type = node->atomicTermDefinitionExpression.lock()->Expression::type;
}

void TypeChecking::visit(std::shared_ptr<AtomicTermDefinitionExpression> node)
{
    AstVisitorBase::visit(node);
    node->Expression::type = R(node->type);
}

void TypeChecking::visit(std::shared_ptr<FunctionTermExpression> node)
{
    AstVisitorBase::visit(node);
    if (node->isCall) {
        node->type = builtinType("dynamic");
    } else {
        node->type = builtinType("function");
    }
}

void TypeChecking::visit(std::shared_ptr<UnifyExpression> node)
{
    AstVisitorBase::visit(node);
    node->type = builtinType("dynamic");
    if (node->leftExpression->type != builtinType("dynamic") && node->rightExpression->type != builtinType("dynamic") &&
        !equivalent(removeAnnotation(node->leftExpression->type), removeAnnotation(node->rightExpression->type))) {
        std::ostringstream nodeDump;
        node->dump(nodeDump);
        std::cerr << "Warning: " + nodeDump.str() + " will fail\n";
    }
}

void TypeChecking::visit(std::shared_ptr<AssignExpression> node)
{
    AstVisitorBase::visit(node);
    if (node->rightExpression->type == builtinType("function")) {
        reportError(node, "Cannot assign function term");
        node->type = node->leftExpression->type;
    } else if (node->leftExpression->type == builtinType("function")) {
        reportError(node, "Cannot assign to function term");
        node->type = node->leftExpression->type;
    } else if (node->rightExpression->type == builtinType("dynamic")) {
        // runtime type checking
        node->type = node->leftExpression->type;
    } else if (node->leftExpression->type == builtinType("dynamic")) {
        node->type = builtinType("dynamic");
    } else {
        assert(node->leftExpression->type != builtinType("function"));
        assert(node->rightExpression->type != builtinType("function"));
        assert(node->leftExpression->type != builtinType("dynamic"));
        assert(node->rightExpression->type != builtinType("dynamic"));
        if (!equivalent(node->leftExpression->type, node->rightExpression->type) &&
            !equivalent(node->leftExpression->type, removeAnnotation(node->rightExpression->type))) {
            std::ostringstream leftExpressionType;
            std::ostringstream rightExpressionType;
            node->leftExpression->type->dump(leftExpressionType);
            node->rightExpression->type->dump(rightExpressionType);
            reportError(node,
                        "Cannot assign term of type " + rightExpressionType.str() + " to " + leftExpressionType.str());
            node->type = node->leftExpression->type;
//            node->type = builtinType("dynamic");
        } else {
            if (!isAnnotated(node->leftExpression->type) && isAnnotated(node->rightExpression->type)) {
                std::cerr << "[TypeChecking] May want to forbid assignment from annotated to non-annotated type.\n";
                std::ostringstream nodeDump;
                node->dump(nodeDump);
                std::cerr << "Warning: assignment looses type annotation: " + nodeDump.str() + "\n";
            }
            node->type = node->leftExpression->type;
        }
    }
}

void TypeChecking::visit(std::shared_ptr<ExplicitCallExpression> node)
{
    AstVisitorBase::visit(node);
    node->type = node->expression->type;
}

void TypeChecking::visit(std::shared_ptr<QuoteExpression> node)
{
    AstVisitorBase::visit(node);
    node->type = node->expression->type;
}

void TypeChecking::visit(std::shared_ptr<OrExpression> node)
{
    AstVisitorBase::visit(node);
    if (!equivalent(node->leftExpression->type, A(builtinType("bool"))) &&
        node->leftExpression->type != builtinType("dynamic")) {
        reportError(node->leftExpression, "Must be +bool value");
    } else if (!equivalent(node->rightExpression->type, A(builtinType("bool"))) &&
               node->rightExpression->type != builtinType("dynamic")) {
        reportError(node->rightExpression, "Must be +bool value");
    }
    node->type = A(builtinType("bool"));
}

void TypeChecking::visit(std::shared_ptr<AndExpression> node)
{
    AstVisitorBase::visit(node);
    if (!equivalent(node->leftExpression->type, A(builtinType("bool"))) &&
        node->leftExpression->type != builtinType("dynamic")) {
        reportError(node->leftExpression, "Must be +bool value");
    } else if (!equivalent(node->rightExpression->type, A(builtinType("bool"))) &&
               node->rightExpression->type != builtinType("dynamic")) {
        reportError(node->rightExpression, "Must be +bool value");
    }
    node->type = A(builtinType("bool"));
}

void TypeChecking::visit(std::shared_ptr<EqualExpression> node)
{
    AstVisitorBase::visit(node);
    equalityComparison(node);
}

void TypeChecking::visit(std::shared_ptr<NotEqualExpression> node)
{
    AstVisitorBase::visit(node);
    equalityComparison(node);
}

void TypeChecking::equalityComparison(std::shared_ptr<BinaryOperatorExpression> node)
{
    if (node->rightExpression->type == builtinType("function")) {
        reportError(node, "Cannot compare function term");
    } else if (node->leftExpression->type == builtinType("function")) {
        reportError(node, "Cannot compare function term");
    } else if (node->rightExpression->type == builtinType("dynamic") ||
               node->leftExpression->type == builtinType("dynamic")) {
        // runtime type checking
    } else {
        assert(node->leftExpression->type != builtinType("function"));
        assert(node->rightExpression->type != builtinType("function"));
        assert(node->leftExpression->type != builtinType("dynamic"));
        assert(node->rightExpression->type != builtinType("dynamic"));
        if (!equivalent(removeAnnotation(node->leftExpression->type), removeAnnotation(node->rightExpression->type))) {
            // FIXME var jak dynamic
            std::ostringstream leftExpressionType;
            std::ostringstream rightExpressionType;
            node->leftExpression->type->dump(leftExpressionType);
            node->rightExpression->type->dump(rightExpressionType);
            std::cerr << "Warning: comparison " << rightExpressionType.str() << " to " << leftExpressionType.str()
                      << " is always false\n";
        }
    }
    node->type = A(builtinType("bool"));
}

void TypeChecking::visit(std::shared_ptr<LessExpression> node)
{
    AstVisitorBase::visit(node);
    relationComparison(node);
}

void TypeChecking::visit(std::shared_ptr<GreaterExpression> node)
{
    AstVisitorBase::visit(node);
    relationComparison(node);
}

void TypeChecking::visit(std::shared_ptr<LessOrEqualExpression> node)
{
    AstVisitorBase::visit(node);
    relationComparison(node);
}

void TypeChecking::visit(std::shared_ptr<GreaterOrEqualExpression> node)
{
    AstVisitorBase::visit(node);
    relationComparison(node);
}

void TypeChecking::relationComparison(std::shared_ptr<BinaryOperatorExpression> node)
{
    if (!equivalent(node->leftExpression->type, A(builtinType("int"))) &&
        node->leftExpression->type != builtinType("dynamic")) {
        reportError(node->leftExpression, "Must be +int value");
    } else if (!equivalent(node->rightExpression->type, A(builtinType("int"))) &&
               node->rightExpression->type != builtinType("dynamic")) {
        reportError(node->rightExpression, "Must be +int value");
    }
    node->type = A(builtinType("bool"));
}

void TypeChecking::visit(std::shared_ptr<AddExpression> node)
{
    AstVisitorBase::visit(node);
    integerArithmetic(node);
}

void TypeChecking::visit(std::shared_ptr<SubtractExpression> node)
{
    AstVisitorBase::visit(node);
    integerArithmetic(node);
}

void TypeChecking::visit(std::shared_ptr<MultiplyExpression> node)
{
    AstVisitorBase::visit(node);
    integerArithmetic(node);
}

void TypeChecking::visit(std::shared_ptr<DivideExpression> node)
{
    AstVisitorBase::visit(node);
    integerArithmetic(node);
}

void TypeChecking::visit(std::shared_ptr<ModuloExpression> node)
{
    AstVisitorBase::visit(node);
    integerArithmetic(node);
}

void TypeChecking::integerArithmetic(std::shared_ptr<BinaryOperatorExpression> node)
{
    if (!equivalent(node->leftExpression->type, A(builtinType("int"))) &&
        node->leftExpression->type != builtinType("dynamic")) {
        reportError(node->leftExpression, "Must be +int value");
    } else if (!equivalent(node->rightExpression->type, A(builtinType("int"))) &&
               node->rightExpression->type != builtinType("dynamic")) {
        reportError(node->rightExpression, "Must be +int value");
    }
    node->type = A(builtinType("int"));
}

void TypeChecking::visit(std::shared_ptr<NotExpression> node)
{
    AstVisitorBase::visit(node);
    if (!equivalent(node->expression->type, A(builtinType("bool"))) &&
        node->expression->type != builtinType("dynamic")) {
        reportError(node->expression, "Must be +bool value");
    }
    node->type = A(builtinType("bool"));
}

void TypeChecking::visit(std::shared_ptr<NegateExpression> node)
{
    AstVisitorBase::visit(node);
    if (!equivalent(node->expression->type, A(builtinType("int"))) &&
        node->expression->type != builtinType("dynamic")) {
        reportError(node->expression, "Must be +int value");
    }
    node->type = A(builtinType("int"));
}

void TypeChecking::visit(std::shared_ptr<DereferenceExpression> node)
{
    AstVisitorBase::visit(node);
    if (node->expression->type == builtinType("function")) {
        reportError(node->expression, "Cannot dereference function term");
        node->type = builtinType("dynamic");
    } else if (node->expression->type == builtinType("dynamic")) {
        node->type = builtinType("dynamic");
    } else {
        std::shared_ptr<Type> strippedAnnotation;
        for (strippedAnnotation = node->expression->type; isAnnotated(
                strippedAnnotation); strippedAnnotation = strippedAnnotation = removeAnnotation(strippedAnnotation));
        assert(std::dynamic_pointer_cast<NamedType>(strippedAnnotation) == nullptr);
        if (std::shared_ptr<PtrType> ptrType = std::dynamic_pointer_cast<PtrType>(strippedAnnotation)) {
            node->type = ptrType->type;
        } else {
            std::ostringstream expressionType;
            node->expression->type->dump(expressionType);
            reportError(node, "Cannot dereference value of non-pointer type " + expressionType.str());
            node->type = builtinType("dynamic");
        }
    }
}

void TypeChecking::visit(std::shared_ptr<AddressOfExpression> node)
{
    AstVisitorBase::visit(node);
    if (node->expression->type == builtinType("function")) {
        reportError(node->expression, "Cannot take address of function term");
        node->type = builtinType("dynamic");
    } else if (node->expression->type == builtinType("dynamic")) {
        node->type = builtinType("dynamic");
    } else {
        node->type = A(std::make_shared<PtrType>(node->expression->type));
    }
}

void TypeChecking::visit(std::shared_ptr<MemberExpression> node)
{
    AstVisitorBase::visit(node);
    if (node->expression->type == builtinType("function")) {
        reportError(node->expression, "Function term doesn't have members");
        node->type = builtinType("dynamic");
    } else if (node->expression->type == builtinType("dynamic")) {
        reportError(node->expression, "Cannot reference members of term with unknown type");
        node->type = builtinType("dynamic");
    } else if (std::dynamic_pointer_cast<BuiltinType>(removeAnnotation(node->expression->type))) {
        reportError(node->expression, "Builtin types don't have members");
        node->type = builtinType("dynamic");
    } else if (std::dynamic_pointer_cast<PtrType>(removeAnnotation(node->expression->type))) {
        reportError(node->expression, "Pointer type doesn't have members");
        node->type = builtinType("dynamic");
    } else {
        std::shared_ptr<RecordDefinition> record = std::dynamic_pointer_cast<RecordDefinition>(
                removeAnnotation(node->expression->type));
        assert(record);
        std::shared_ptr<Type> memberType = nullptr;
        for (unsigned i = 0; i < record->fields.size(); ++i) {
            if (record->fields[i].second == node->memberName) {
                memberType = record->fields[i].first;
                // TODO: Bind field offset in record here?
                node->index = i;
            }
        }
        if (memberType == nullptr) {
            reportError(node, "No field named " + node->memberName + " in record " + record->name);
            node->type = builtinType("dynamic");
        } else {
            node->type = carryAnnotation(node->expression->type, R(memberType));
        }
    }
}

std::shared_ptr<BuiltinType> TypeChecking::builtinType(const std::string &name) const
{
    auto i = builtinTypes.find(name);
    assert(i != builtinTypes.end());
    return i->second;
}

std::shared_ptr<Type> TypeChecking::R(std::shared_ptr<Type> type)
{
    class TypeNameResolution : public AstVisitorBase
    {
    public:
        explicit TypeNameResolution(const TypeChecking &typeChecker, std::shared_ptr<Type> type)
                : typeChecker(typeChecker), type(type)
        {}

        void visit(std::shared_ptr<Ast> node) override
        {
            (void) node;
            assert(false);
        }

        void visit(std::shared_ptr<RecordDefinition> node) override
        {
            (void) node;
            result = type;
        }

        void visit(std::shared_ptr<IntType> node) override
        {
            (void) node;
            result = type;
        }

        void visit(std::shared_ptr<BoolType> node) override
        {
            (void) node;
            result = type;
        }

        void visit(std::shared_ptr<VarType> node) override
        {
            (void) node;
            result = type;
        }

        void visit(std::shared_ptr<DynamicType> node) override
        {
            (void) node;
            result = type;
        }

        void visit(std::shared_ptr<FunctionType> node) override
        {
            (void) node;
            result = type;
        }

        void visit(std::shared_ptr<NamedType> node) override
        {
            TypeNameResolution visitor(typeChecker, node->typeDefinition.lock());
            node->typeDefinition.lock()->accept(visitor);
            result = visitor.getResult();
        }

        void visit(std::shared_ptr<AnnotatedType> node) override
        {
            TypeNameResolution visitor(typeChecker, node->type);
            node->type->accept(visitor);
            result = std::make_shared<AnnotatedType>(visitor.getResult());
        }

        void visit(std::shared_ptr<PtrType> node) override
        {
            TypeNameResolution visitor(typeChecker, node->type);
            node->type->accept(visitor);
            result = std::make_shared<PtrType>(visitor.getResult());
        }

        std::shared_ptr<Type> getResult() const
        {
            return result;
        }

    private:
        const TypeChecking &typeChecker;
        std::shared_ptr<Type> type;
        std::shared_ptr<Type> result;
    };

    TypeNameResolution visitor(*this, type);
    type->accept(visitor);
    return visitor.getResult();
}

std::shared_ptr<Type> TypeChecking::removeAnnotation(std::shared_ptr<Type> type)
{
    if (auto annotatedType = std::dynamic_pointer_cast<AnnotatedType>(type)) {
        return annotatedType->type;
    } else {
        return type;
    }
}

std::shared_ptr<Type> TypeChecking::A(std::shared_ptr<Type> type)
{
    if (isAnnotated(type)) {
        return type;
    } else {
        assert(std::dynamic_pointer_cast<DynamicType>(type) == nullptr);
        assert(std::dynamic_pointer_cast<FunctionType>(type) == nullptr);
        return std::make_shared<AnnotatedType>(type);
    }
}

std::shared_ptr<Type> TypeChecking::carryAnnotation(std::shared_ptr<Type> from, std::shared_ptr<Type> to)
{
    if (isAnnotated(from)) {
        return A(to);
    } else {
        return to;
    }
}

bool TypeChecking::isAnnotated(std::shared_ptr<Type> type)
{
    if (std::dynamic_pointer_cast<AnnotatedType>(type)) {
        return true;
    } else {
        return false;
    }
}

bool TypeChecking::equivalent(std::shared_ptr<Type> lhs, std::shared_ptr<Type> rhs)
{
    class EquivalenceChecker : public AstVisitorBase
    {
    public:
        explicit EquivalenceChecker(std::shared_ptr<Type> to)
                : to(to)
        {}

        void visit(std::shared_ptr<RecordDefinition> node) override
        {
            result = atomicTypeEquivalent(node, to);
        }

        void visit(std::shared_ptr<IntType> node) override
        {
            result = atomicTypeEquivalent(node, to);
        }

        void visit(std::shared_ptr<BoolType> node) override
        {
            result = atomicTypeEquivalent(node, to);
        }

        void visit(std::shared_ptr<VarType> node) override
        {
            result = atomicTypeEquivalent(node, to);
        }

        void visit(std::shared_ptr<DynamicType> node) override
        {
            result = atomicTypeEquivalent(node, to);
        }

        void visit(std::shared_ptr<FunctionType> node) override
        {
            result = atomicTypeEquivalent(node, to);
        }

        void visit(std::shared_ptr<AnnotatedType> node) override
        {
            if (auto reified = std::dynamic_pointer_cast<AnnotatedType>(to)) {
                EquivalenceChecker equivalenceChecker(reified->type);
                node->type->accept(equivalenceChecker);
                result = equivalenceChecker.getResult();
            } else {
                result = false;
            }
        }

        void visit(std::shared_ptr<PtrType> node) override
        {
            if (auto reified = std::dynamic_pointer_cast<PtrType>(to)) {
                EquivalenceChecker equivalenceChecker(reified->type);
                node->type->accept(equivalenceChecker);
                result = equivalenceChecker.getResult();
            } else {
                result = false;
            }
        }

        bool getResult() const
        {
            return result;
        }

    private:


    private:
        std::shared_ptr<Type> to;
        bool result;
    };
    EquivalenceChecker equivalenceChecker(rhs);
    lhs->accept(equivalenceChecker);
    return equivalenceChecker.getResult();
}

template<class NodeType>
static bool atomicTypeEquivalent(std::shared_ptr<NodeType> lhs, std::shared_ptr<Type> rhs)
{
    if (auto reified = std::dynamic_pointer_cast<NodeType>(rhs)) {
        return lhs == reified;
    } else {
        return false;
    }
}
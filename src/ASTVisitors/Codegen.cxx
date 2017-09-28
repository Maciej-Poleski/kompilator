#include "Codegen.hxx"

#include <iostream>
#include <unordered_set>

#include "src/AST/Declarations.hxx"

#include "NameMangler.hxx"

#include "llvm/ADT/APSInt.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include "llvm/ADT/Optional.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"

using namespace llvm;

Codegen::State::State(const std::string &moduleId)
        : TheContext(), Builder(TheContext), TheModule(moduleId, TheContext)
{
}

Codegen::Codegen(const std::string &moduleId, bool dumpLlvm)
        : state(std::make_shared<State>(moduleId)), dumpLlvm(dumpLlvm)
{
    emitFunctionsListPrototype();

    emitRuntimeGetLogMarkerPrototype();
    emitRuntimeRollbackToMarkerPrototype();
    emitRuntimeAllocatePrototypes();
    emitRuntimeAssignRecordFieldPrototype();
    emitRuntimeAssignFunctionSymbolSubtermPrototype();
    emitRuntimeGetIntValuePrototype();
    emitRuntimeGetBoolValuePrototype();
    emitRuntimeGetPtrValuePrototype();
    emitRuntimeGetRecordMemberPrototype();
    emitRuntimeUnifyPrototype();
    emitRuntimeContinueSldResolutionPrototype();
    emitRuntimeInvokePrototype();
    emitRuntimeFailedPrototype();
    emitRuntimeEqualsPrototype();
    emitRuntimeDeepCopyRepresentationPrototype();
    emitRuntimeSetPtrValuePrototype();
    emitRuntimeAddGoalPrototype();
}

Codegen::Codegen(std::shared_ptr<Codegen::State> state, bool dumpLlvm)
        : state(state), dumpLlvm(dumpLlvm)
{

}

void Codegen::visit(std::shared_ptr<FunctionDefinitionDeclarative> node)
{
    if (std::dynamic_pointer_cast<FunctionBodyExtern>(node->functionBody)) {
        node->functionBody->accept(*this);
        return;
    }
    std::array<llvm::Type *, 2> params = {llvm::Type::getInt8PtrTy(state->TheContext),
                                          llvm::Type::getInt8PtrTy(state->TheContext)};
    llvm::FunctionType *functionType = llvm::FunctionType::get(llvm::Type::getInt1Ty(state->TheContext), params, false);
    Function *function = Function::Create(functionType, Function::PrivateLinkage, "", &state->TheModule);
    emitFunctionExport(function, node);
    currentFunction = function;
    std::vector<Value *> args;
    for (auto &arg : function->args()) {
        args.push_back(&arg);
    }
    auto basicBlock = BasicBlock::Create(state->TheContext, "entry", function);
    state->Builder.SetInsertPoint(basicBlock);

    Value *marker = emitGetSldLogMarker();

    Value *functionHeadTerm = emitTerm(function, std::make_shared<FunctionTermExpression>(node->name, node->terms));

    Value *headUnifyResult = state->Builder.CreateCall(runtimeUnifyFunction, {functionHeadTerm, args[0]},
                                                       "head_unification_result");
    BasicBlock *headUnifiedBlock = BasicBlock::Create(state->TheContext, "head_unified", function);
    failedBlock = BasicBlock::Create(state->TheContext, "function_failed");

    state->Builder.CreateCondBr(headUnifyResult, headUnifiedBlock, failedBlock);
    state->Builder.SetInsertPoint(headUnifiedBlock);
    result = function;
    afterForBlock = BasicBlock::Create(state->TheContext, "function_footer");
    node->functionBody->accept(*this);

    state->Builder.CreateBr(afterForBlock);
    function->getBasicBlockList().push_back(afterForBlock);
    state->Builder.SetInsertPoint(afterForBlock);
    CallInst *sldResolutionResult = state->Builder.CreateCall(runtimeContinueSldResolutionFunction, None,
                                                              "sld_resolution_result");
    BasicBlock *sldResolutionSucceeded = BasicBlock::Create(state->TheContext, "sld_resolution_succeeded", function);
    state->Builder.CreateCondBr(sldResolutionResult, sldResolutionSucceeded, failedBlock);
    state->Builder.SetInsertPoint(sldResolutionSucceeded);
    state->Builder.CreateRet(state->Builder.getTrue());

    // head unification failed
    function->getBasicBlockList().push_back(failedBlock);
    state->Builder.SetInsertPoint(failedBlock);
    state->Builder.CreateCall(runtimeRollbackToMarkerFunction, {marker});
    state->Builder.CreateRet(state->Builder.getFalse());
}

void Codegen::visit(std::shared_ptr<FunctionDefinitionImperative> node)
{
    bool emitExport = true;
    if (auto functionBody = std::dynamic_pointer_cast<FunctionBodyExtern>(node->functionBody)) {
        emitExport = false;
        if (functionBody->getAbi() == FunctionBodyExtern::Abi::DI) {
            node->functionBody->accept(*this);
            return;
        }
    }
    std::array<llvm::Type *, 2> params = {llvm::Type::getInt8PtrTy(state->TheContext),
                                          llvm::Type::getInt8PtrTy(state->TheContext)};
    llvm::FunctionType *functionType = llvm::FunctionType::get(llvm::Type::getInt1Ty(state->TheContext), params, false);
    Function *function = Function::Create(functionType, Function::PrivateLinkage, "", &state->TheModule);
    if (emitExport) {
        emitFunctionExport(function, node);
    }
    currentFunction = function;
    std::vector<Value *> args;
    for (auto &arg : function->args()) {
        args.push_back(&arg);
    }
    auto basicBlock = BasicBlock::Create(state->TheContext, "entry", function);
    state->Builder.SetInsertPoint(basicBlock);

    Value *marker = emitGetSldLogMarker();

    // Result part of term head can be used in the same head
    Value *functionResultTerm;
    if (node->result) {
        functionResultTerm = emitTerm(function, node->result);
    }

    Value *functionHeadTerm = emitTerm(function, std::make_shared<FunctionTermExpression>(node->name, node->terms));

    Value *headUnifyResult = state->Builder.CreateCall(runtimeUnifyFunction, {functionHeadTerm, args[0]},
                                                       "head_unification_result");
    BasicBlock *headUnifiedBlock = BasicBlock::Create(state->TheContext, "head_unified", function);
    failedBlock = BasicBlock::Create(state->TheContext, "function_failed");

    state->Builder.CreateCondBr(headUnifyResult, headUnifiedBlock, failedBlock);
    state->Builder.SetInsertPoint(headUnifiedBlock);
    if (node->result) {
        Value *resultUnifyResult = state->Builder.CreateCall(runtimeUnifyFunction, {functionResultTerm, args[1]},
                                                             "result_unification_result");
        BasicBlock *resultUnifiedBlock = BasicBlock::Create(state->TheContext, "result_unified", function);
        state->Builder.CreateCondBr(resultUnifyResult, resultUnifiedBlock, failedBlock);
        state->Builder.SetInsertPoint(resultUnifiedBlock);
    }

    result = function;
    afterForBlock = BasicBlock::Create(state->TheContext, "function_footer");
    node->functionBody->accept(*this);

    state->Builder.CreateBr(afterForBlock);
    function->getBasicBlockList().push_back(afterForBlock);
    state->Builder.SetInsertPoint(afterForBlock);
    CallInst *sldResolutionResult = state->Builder.CreateCall(runtimeContinueSldResolutionFunction, None,
                                                              "sld_resolution_result");
    BasicBlock *sldResolutionSucceeded = BasicBlock::Create(state->TheContext, "sld_resolution_succeeded", function);
    state->Builder.CreateCondBr(sldResolutionResult, sldResolutionSucceeded, failedBlock);
    state->Builder.SetInsertPoint(sldResolutionSucceeded);
    state->Builder.CreateRet(state->Builder.getTrue());

    // head unification failed
    function->getBasicBlockList().push_back(failedBlock);
    state->Builder.SetInsertPoint(failedBlock);
    state->Builder.CreateCall(runtimeRollbackToMarkerFunction, {marker});
    state->Builder.CreateRet(state->Builder.getFalse());
}

void Codegen::visit(std::shared_ptr<FunctionBodyDeclarative> node)
{
    for (auto expression : node->body) {
        expression->accept(*this);
        std::array<Value *, 2> args = {functionsListVariable, result};
        state->Builder.CreateCall(runtimeAddGoalFunction, args);
    }
}

void Codegen::visit(std::shared_ptr<Expression> node)
{
    result = emitTerm(currentFunction, node);
}

void Codegen::visit(std::shared_ptr<FunctionTermExpression> node)
{
    visit(std::static_pointer_cast<Expression>(node));
}

void Codegen::visit(std::shared_ptr<AtomicTermDefinitionExpression> node)
{
    visit(std::static_pointer_cast<Expression>(node));
}

void Codegen::visit(std::shared_ptr<UnaryOperatorExpression> node)
{
    visit(std::static_pointer_cast<Expression>(node));
}

void Codegen::visit(std::shared_ptr<BinaryOperatorExpression> node)
{
    visit(std::static_pointer_cast<Expression>(node));
}

void Codegen::visit(std::shared_ptr<FunctionBodyImperative> node)
{
    auto function = dyn_cast<Function>(result);
    if (!function) {
        result = nullptr;
        return;
    }

    for (auto statement : node->statements) {
        statement->accept(*this);
    }

    result = function;
}

void Codegen::visit(std::shared_ptr<ExpressionStatement> node)
{
    node->expression->accept(*this);
}

void Codegen::visit(std::shared_ptr<IfStatement> node)
{
    node->condition->accept(*this);
    Value *condition = state->Builder.CreateCall(runtimeGetBoolValueFunction, result, "condition_value");
    BasicBlock *branchTrue = BasicBlock::Create(state->TheContext, "branch_true_if", currentFunction);
    BasicBlock *branchFalse = BasicBlock::Create(state->TheContext, "branch_false_if");
    BasicBlock *afterIf = BasicBlock::Create(state->TheContext, "after_if");
    state->Builder.CreateCondBr(condition, branchTrue, branchFalse);
    state->Builder.SetInsertPoint(branchTrue);
    for (auto statement: node->thenBlock) {
        statement->accept(*this);
    }
    state->Builder.CreateBr(afterIf);
    currentFunction->getBasicBlockList().push_back(branchFalse);
    state->Builder.SetInsertPoint(branchFalse);
    for (auto statement: node->elseBlock) {
        statement->accept(*this);
    }
    state->Builder.CreateBr(afterIf);
    currentFunction->getBasicBlockList().push_back(afterIf);
    state->Builder.SetInsertPoint(afterIf);
}

void Codegen::visit(std::shared_ptr<ForStatement> node)
{
    for (auto expression : node->initExpressions) {
        expression->accept(*this);
    }
    auto oldAfterForBlock = afterForBlock;
    auto oldNextForBlock = nextForBlock;
    BasicBlock *conditionBlock = BasicBlock::Create(state->TheContext, "for_condition", currentFunction);
    state->Builder.CreateBr(conditionBlock);
    state->Builder.SetInsertPoint(conditionBlock);
    node->conditionExpression->accept(*this);
    Value *condition = state->Builder.CreateCall(runtimeGetBoolValueFunction, result, "condition_value");
    BasicBlock *bodyBlock = BasicBlock::Create(state->TheContext, "for_body", currentFunction);
    BasicBlock *afterFor = BasicBlock::Create(state->TheContext, "after_for");
    BasicBlock *lastStatements = BasicBlock::Create(state->TheContext, "for_next");
    afterForBlock = afterFor;
    nextForBlock = lastStatements;
    state->Builder.CreateCondBr(condition, bodyBlock, afterFor);
    state->Builder.SetInsertPoint(bodyBlock);
    for (auto statement : node->body) {
        statement->accept(*this);
    }
    currentFunction->getBasicBlockList().push_back(lastStatements);
    state->Builder.CreateBr(lastStatements);
    state->Builder.SetInsertPoint(lastStatements);
    for (auto expression : node->lastExpressions) {
        expression->accept(*this);
    }
    state->Builder.CreateBr(conditionBlock);
    currentFunction->getBasicBlockList().push_back(afterFor);
    state->Builder.SetInsertPoint(afterFor);
    afterForBlock = oldAfterForBlock;
    nextForBlock = oldNextForBlock;
}

void Codegen::visit(std::shared_ptr<BreakStatement>)
{
    state->Builder.CreateBr(afterForBlock);
    BasicBlock *deadcodeBlock = BasicBlock::Create(state->TheContext, "deadcode");
    currentFunction->getBasicBlockList().push_back(deadcodeBlock);
    state->Builder.SetInsertPoint(deadcodeBlock);
}

void Codegen::visit(std::shared_ptr<ContinueStatement> node)
{
    if (!nextForBlock) {
        reportError(node, "Outside of for loop");
        return;
    }
    state->Builder.CreateBr(nextForBlock);
    BasicBlock *deadcodeBlock = BasicBlock::Create(state->TheContext, "deadcode");
    currentFunction->getBasicBlockList().push_back(deadcodeBlock);
    state->Builder.SetInsertPoint(deadcodeBlock);
}

void Codegen::visit(std::shared_ptr<FunctionBodyExtern> node)
{
    switch (node->getAbi()) {
        case FunctionBodyExtern::Abi::DI:
            emitFunctionImport(node->getParent());
            break;
        case FunctionBodyExtern::Abi::C:
            emitFunctionCImport(node->getParent());
            break;
    }
}

std::vector<uint8_t> getTypeIds(std::shared_ptr<::Type> node)
{
    class TypeIdBuilder : public AstVisitorBase
    {
    public:
        void visit(std::shared_ptr<Ast>) override
        {
            assert(false);
        }

        void visit(std::shared_ptr<IntType> node) override
        {
            result.push_back(1);
        }

        void visit(std::shared_ptr<BoolType> node) override
        {
            result.push_back(2);
        }

        void visit(std::shared_ptr<VarType> node) override
        {
            result.push_back(6);
        }

        void visit(std::shared_ptr<::FunctionType> node) override
        {
            result.push_back(5);
        }

        void visit(std::shared_ptr<RecordDefinition> node) override
        {
            result.push_back(4);
            union
            {
                std::uint32_t nameLength;
                std::uint8_t nameLengthBytes[4];
            };
            nameLength = node->name.size();
            for (unsigned i = 0; i < sizeof(nameLengthBytes); ++i)
                result.push_back(nameLengthBytes[i]);
            std::copy(node->name.begin(), node->name.end(), std::back_inserter(result));
        }

        void visit(std::shared_ptr<NamedType> node) override
        {
            node->typeDefinition.lock()->accept(*this);
        }

        void visit(std::shared_ptr<AnnotatedType> node) override
        {
            auto index = result.size();
            node->type->accept(*this);
            assert(index < result.size());
            result[index] |= 0x40;
        }

        void visit(std::shared_ptr<PtrType> node) override
        {
            result.push_back(3);
            node->type->accept(*this);
        }

        std::vector<std::uint8_t> getResult() &&
        {
            return std::move(result);
        }

    private:
        std::vector<std::uint8_t> result;
    };
    TypeIdBuilder builder;
    node->accept(builder);
    return std::move(builder).getResult();
}

void Codegen::emitFunctionsListPrototype()
{
    auto functionType = llvm::FunctionType::get(llvm::Type::getInt1Ty(state->TheContext),
                                                {llvm::Type::getInt8PtrTy(state->TheContext),
                                                 llvm::Type::getInt8PtrTy(state->TheContext)}, false);
    auto functionPointerType = llvm::PointerType::get(functionType, 0);
    auto functionPointerPointerType = llvm::PointerType::get(functionPointerType, 0);
    auto functionPointerPointerPointerType = llvm::PointerType::get(functionPointerPointerType, 0);
    functionsListVariable = new GlobalVariable(functionPointerPointerType, true, GlobalVariable::PrivateLinkage,
                                               nullptr, "functions_list");
    state->TheModule.getGlobalList().push_back(functionsListVariable);
}

void Codegen::emitFunctionExport(llvm::Function *function, std::shared_ptr<FunctionDefinition> node)
{
    auto functionArrayStubType = ArrayType::get(function->getType(), 2);
    auto functionArray = ConstantArray::get(functionArrayStubType,
                                            {function, Constant::getNullValue(function->getType())});
    auto functionArrayVariable = new GlobalVariable(functionArray->getType(), true, GlobalVariable::PrivateLinkage,
                                                    functionArray);
    state->TheModule.getGlobalList().push_back(functionArrayVariable);
    functions.push_back(functionArrayVariable);

    functionsForName[NameMangler::getMangledName(node)].push_back(function);
    if (auto type = getCExportType(node)) {
        functionsToExportC[node->name].emplace_back(function, type, node);
    }
}

void Codegen::emitFunctionImport(std::shared_ptr<FunctionDefinition> node)
{
    auto mangledName = NameMangler::getMangledName(node);
    auto functionType = llvm::FunctionType::get(llvm::Type::getInt1Ty(state->TheContext),
                                                {llvm::Type::getInt8PtrTy(state->TheContext),
                                                 llvm::Type::getInt8PtrTy(state->TheContext)}, false);
    auto functionPointerType = llvm::PointerType::get(functionType, 0);
    auto functionPointerArrayType = llvm::ArrayType::get(functionPointerType, functions.size() + 1);
    auto functionsArray = new GlobalVariable(functionPointerArrayType, true, GlobalVariable::ExternalLinkage, nullptr,
                                             mangledName);
    state->TheModule.getGlobalList().push_back(functionsArray);
    functions.push_back(functionsArray);
}

void Codegen::emitFunctionCImport(std::shared_ptr<FunctionDefinition> node)
{
    class AnnotationChecker : public AstVisitorBase
    {
    public:
        explicit AnnotationChecker(Codegen &codegen)
                : codegen(codegen)
        {}

        void visit(std::shared_ptr<RecordDefinition> node) override
        {
            for (auto field : node->fields) {
                field.first->accept(*this);
                if (!result) {
                    break;
                }
            }
        }

        void visit(std::shared_ptr<IntType> node) override
        {
            result = false;
        }

        void visit(std::shared_ptr<BoolType> node) override
        {
            result = false;
        }

        void visit(std::shared_ptr<VarType> node) override
        {
            result = false;
        }

        void visit(std::shared_ptr<NamedType> node) override
        {
            node->typeDefinition.lock()->accept(*this);
        }

        void visit(std::shared_ptr<AnnotatedType> node) override
        {
            return;
        }

        void visit(std::shared_ptr<PtrType> node) override
        {
            // TODO Support pointers
            codegen.reportError(node, "Pointers in extern \"C\" imports are not supported");
        }

        void visit(std::shared_ptr<NameReferenceExpression> node) override
        {
            node->atomicTermDefinitionExpression.lock()->accept(*this);
        }

        void visit(std::shared_ptr<AtomicTermDefinitionExpression> node) override
        {
            node->type->accept(*this);
        }

        void visit(std::shared_ptr<FunctionTermExpression> node) override
        {
            codegen.reportError(node, "Cannot use function symbol arguments in extern \"C\" import");
        }

        void visit(std::shared_ptr<Expression>) override
        {
            assert(false);
        }

        bool getResult() const
        {
            return result;
        }

    private:
        Codegen &codegen;
        bool result = true;
    };
    std::vector<std::shared_ptr<Expression>> *args;
    std::shared_ptr<Expression> resultExpression = nullptr;
    if (auto definition = std::dynamic_pointer_cast<FunctionDefinitionImperative>(node)) {
        args = &definition->terms;
        resultExpression = definition->result;
    } else if (auto definition = std::dynamic_pointer_cast<FunctionDefinitionDeclarative>(node)) {
        args = &definition->terms;
    } else {
        assert(false);
    }
    for (auto arg : *args) {
        AnnotationChecker checker(*this);
        arg->accept(checker);
        if (!checker.getResult()) {
            reportError(arg, "Must be declared as fully defined");
        }
    }

    if (hasErrors()) {
        return;
    }

    llvm::FunctionType *functionType = getCExportType(node);
    if (!functionType) {
        reportError(resultExpression, "Cannot use function symbol result in extern \"C\" import");
    }
    llvm::Function *function = Function::Create(functionType, Function::ExternalLinkage, node->name, &state->TheModule);

    std::vector<Value *> argValues;
    for (auto arg : *args) {
        if (auto definition = std::dynamic_pointer_cast<AtomicTermDefinitionExpression>(arg)) {
            argValues.push_back(
                    emitValue(currentFunction, arg, state->Builder.CreateLoad(state->NamedValues[definition])));
        } else if (auto reference = std::dynamic_pointer_cast<NameReferenceExpression>(arg)) {
            argValues.push_back(emitValue(currentFunction, arg, state->Builder.CreateLoad(
                    state->NamedValues[reference->atomicTermDefinitionExpression.lock()])));
        } else {
            assert(false);
        }
    }

    Value *resultValue = state->Builder.CreateCall(function, argValues);
    if (resultExpression) {
        assert(resultValue->getType() != state->Builder.getVoidTy());
        auto resultDefinition = std::dynamic_pointer_cast<AtomicTermDefinitionExpression>(resultExpression);
        assert(resultDefinition);
        Value *resultTerm = emitTerm(currentFunction, resultDefinition->type, resultValue);

        std::array<Value *, 2> unifyArgs = {state->Builder.CreateLoad(state->NamedValues[resultDefinition]),
                                            resultTerm};
        Value *unifyResult = state->Builder.CreateCall(runtimeUnifyFunction, unifyArgs);
        BasicBlock *nextBlock = BasicBlock::Create(state->TheContext, "", currentFunction);
        state->Builder.CreateCondBr(unifyResult, nextBlock, failedBlock);
        state->Builder.SetInsertPoint(nextBlock);
    }

    // emit private function symbol and add to functions list
    auto functionArrayStubType = ArrayType::get(currentFunction->getType(), 2);
    auto functionArray = ConstantArray::get(functionArrayStubType,
                                            {currentFunction, Constant::getNullValue(currentFunction->getType())});
    auto functionArrayVariable = new GlobalVariable(functionArray->getType(), true, GlobalVariable::PrivateLinkage,
                                                    functionArray);
    state->TheModule.getGlobalList().push_back(functionArrayVariable);
    functions.push_back(functionArrayVariable);
}

void Codegen::emitFunctionsListInitializer()
{
    auto functionType = llvm::FunctionType::get(llvm::Type::getInt1Ty(state->TheContext),
                                                {llvm::Type::getInt8PtrTy(state->TheContext),
                                                 llvm::Type::getInt8PtrTy(state->TheContext)}, false);
    auto functionPointerType = llvm::PointerType::get(functionType, 0);
    auto functionPointerPointerType = llvm::PointerType::get(functionPointerType, 0);
    functions.push_back(Constant::getNullValue(functionPointerType));
    auto initializerType = ArrayType::get(functionPointerPointerType, functions.size());
    auto initializer = ConstantArray::get(initializerType, functions);
    functionsListVariable->setInitializer(initializer);
}

void Codegen::emitFunctionsExports()
{
    for (auto &pack : functionsForName) {
        auto &mangledName = pack.first;
        auto &functions = pack.second;
        auto functionType = llvm::FunctionType::get(llvm::Type::getInt1Ty(state->TheContext),
                                                    {llvm::Type::getInt8PtrTy(state->TheContext),
                                                     llvm::Type::getInt8PtrTy(state->TheContext)}, false);
        auto functionPointerType = llvm::PointerType::get(functionType, 0);
        auto functionPointerArrayType = llvm::ArrayType::get(functionPointerType, functions.size() + 1);
        std::vector<llvm::Constant *> functionPointers;
        for (auto function : functions) {
            functionPointers.push_back(function);
        }
        functionPointers.push_back(llvm::Constant::getNullValue(functionPointerType));
        llvm::Constant *functionPointersArray = llvm::ConstantArray::get(functionPointerArrayType, functionPointers);
        auto functionsArray = new GlobalVariable(functionPointerArrayType, true, GlobalVariable::ExternalLinkage,
                                                 functionPointersArray, mangledName);
        state->TheModule.getGlobalList().push_back(functionsArray);
    }
}

void Codegen::emitFunctionsCExports()
{
    for (auto symbol : functionsToExportC) {
        assert(!symbol.first.empty());
        assert(!symbol.second.empty());
        auto &name = symbol.first;
        bool canEmit = true;
        for (unsigned i = 1; i < symbol.second.size(); ++i) {
            if (std::get<1>(symbol.second[i - 1]) != std::get<1>(symbol.second[i])) {
                canEmit = false;
                break;
            }
        }
        if (!canEmit) {
            std::cerr << "Symbol " << name << " pominięty: brak uniwersalnego typu\n";
            continue;
        }
        Function *function = Function::Create(std::get<1>(symbol.second[0]), Function::ExternalLinkage, name,
                                              &state->TheModule);
        BasicBlock *entry = BasicBlock::Create(state->TheContext, "entry", function);
        state->Builder.SetInsertPoint(entry);

        std::vector<Value *> functionArgs;
        for (auto &arg : function->args()) {
            functionArgs.push_back(&arg);
        }

        Value *marker;
        marker = emitGetSldLogMarker();
        for (unsigned i = 0; i < symbol.second.size(); ++i) {
            auto &predicate = symbol.second[i];
            std::vector<Constant *> functionSymbolChars;
            for (auto byte : name) {
                functionSymbolChars.push_back(state->Builder.getInt8(reinterpret_cast<std::uint8_t &>(byte)));
            }
            Constant *functionSymbolName = ConstantArray::get(ArrayType::get(state->Builder.getInt8Ty(), name.size()),
                                                              functionSymbolChars);

            AllocaInst *functionSymbolNameLocation = state->Builder.CreateAlloca(functionSymbolName->getType(),
                                                                                 ConstantInt::get(
                                                                                         state->Builder.getInt32Ty(),
                                                                                         name.size()));
            state->Builder.CreateStore(functionSymbolName, functionSymbolNameLocation);

            std::array<Value *, 4> args = {state->Builder.getTrue(), state->Builder.getInt32(
                    static_cast<std::uint32_t>(std::get<1>(predicate)->getNumParams())), state->Builder.getInt32(
                    static_cast<std::uint32_t >(functionSymbolChars.size())), functionSymbolNameLocation};
            CallInst *allocateFunctionSymbolCall = state->Builder.CreateCall(runtimeAllocateFunctionSymbolFunction,
                                                                             args, "symbol_for_call");

            std::vector<std::shared_ptr<Expression>> *subterms;
            std::shared_ptr<Ast> result = std::make_shared<VarType>();
            if (auto declarative = std::dynamic_pointer_cast<FunctionDefinitionDeclarative>(std::get<2>(predicate))) {
                subterms = &declarative->terms;
            } else if (auto imperative = std::dynamic_pointer_cast<FunctionDefinitionImperative>(
                    std::get<2>(predicate))) {
                subterms = &imperative->terms;
                result = imperative->result;
            } else {
                assert(false);
            }
            for (unsigned i = 0; i < subterms->size(); ++i) {
                Value *arg = emitTerm(function, (*subterms)[i], functionArgs[i]);
                std::array<Value *, 3> args = {allocateFunctionSymbolCall, arg, state->Builder.getInt32(i)};
                state->Builder.CreateCall(runtimeAssignFunctionSymbolSubtermFunction, args);
            }
            Value *resultTerm = emitTerm(function, result);
            std::array<Value *, 2> invokeArgs = {allocateFunctionSymbolCall, resultTerm};
            Value *callResult = state->Builder.CreateCall(std::get<0>(predicate), invokeArgs, "call_result");
            BasicBlock *success = BasicBlock::Create(state->TheContext, "call_succesfull", function);
            BasicBlock *failure = BasicBlock::Create(state->TheContext, "call_failed");
            state->Builder.CreateCondBr(callResult, success, failure);
            state->Builder.SetInsertPoint(success);
            Value *resultValue = nullptr;
            if (!std::dynamic_pointer_cast<VarType>(result)) {
                // FIXME może być niezainicjalizowany (nie ustawiony przez funkcje)
                resultValue = emitValue(function, result, resultTerm);
            }
            state->Builder.CreateCall(runtimeRollbackToMarkerFunction, {marker});
            if (resultValue) {
                state->Builder.CreateRet(resultValue);
            } else {
                state->Builder.CreateRetVoid();
            }

            function->getBasicBlockList().push_back(failure);
            state->Builder.SetInsertPoint(failure);
            if (i + 1 < symbol.second.size()) {
                state->Builder.CreateCall(runtimeRollbackToMarkerFunction, {marker});
            }
        }
        state->Builder.CreateCall(runtimeFailedFunction);
        // doesn't return but basic block must end with ret or jmp
        auto returnType = function->getReturnType();
        if (!returnType->isVoidTy()) {
            state->Builder.CreateRet(llvm::UndefValue::get(returnType));
        } else {
            state->Builder.CreateRetVoid();
        }
    }
}

void Codegen::emitRuntimeGetLogMarkerPrototype()
{
    auto functionType = llvm::FunctionType::get(llvm::Type::getInt32Ty(state->TheContext), false);
    runtimeGetLogMarkerFunction = Function::Create(functionType, Function::ExternalLinkage, "_DI_getLogMarker",
                                                   &state->TheModule);
}

void Codegen::emitRuntimeRollbackToMarkerPrototype()
{
    std::array<llvm::Type *, 1> params = {llvm::Type::getInt32Ty(state->TheContext)};
    auto functionType = llvm::FunctionType::get(llvm::Type::getVoidTy(state->TheContext), params, false);
    runtimeRollbackToMarkerFunction = Function::Create(functionType, Function::ExternalLinkage, "_DI_rollbackToMarker",
                                                       &state->TheModule);
}

void Codegen::emitRuntimeAllocatePrototypes()
{
    emitRuntimeAllocateIntPrototype();
    emitRuntimeAllocateBoolPrototype();
    emitRuntimeAllocatePtrPrototype();
    emitRuntimeAllocateFunctionSymbolPrototype();
    emitRuntimeAllocateRecordPrototype();
    emitRuntimeAllocateVariablePrototype();
}

void Codegen::emitRuntimeAllocateIntPrototype()
{
    std::array<llvm::Type *, 3> params = {llvm::Type::getInt1Ty(state->TheContext),
                                          llvm::Type::getInt1Ty(state->TheContext),
                                          llvm::Type::getInt32Ty(state->TheContext)};
    auto functionType = llvm::FunctionType::get(llvm::Type::getInt8PtrTy(state->TheContext), params, false);
    runtimeAllocateIntFunction = Function::Create(functionType, Function::ExternalLinkage, "_DI_allocateInt",
                                                  &state->TheModule);
}

void Codegen::emitRuntimeAllocateBoolPrototype()
{
    std::array<llvm::Type *, 3> params = {llvm::Type::getInt1Ty(state->TheContext),
                                          llvm::Type::getInt1Ty(state->TheContext),
                                          llvm::Type::getInt1Ty(state->TheContext)};
    auto functionType = llvm::FunctionType::get(llvm::Type::getInt8PtrTy(state->TheContext), params, false);
    runtimeAllocateBoolFunction = Function::Create(functionType, Function::ExternalLinkage, "_DI_allocateBool",
                                                   &state->TheModule);
}

void Codegen::emitRuntimeAllocatePtrPrototype()
{
    std::array<llvm::Type *, 4> params = {llvm::Type::getInt1Ty(state->TheContext),
                                          llvm::Type::getInt1Ty(state->TheContext),
                                          llvm::Type::getInt8PtrTy(state->TheContext),
                                          llvm::Type::getInt8PtrTy(state->TheContext)};
    auto functionType = llvm::FunctionType::get(llvm::Type::getInt8PtrTy(state->TheContext), params, false);
    runtimeAllocatePtrFunction = Function::Create(functionType, Function::ExternalLinkage, "_DI_allocatePtr",
                                                  &state->TheModule);
}

void Codegen::emitRuntimeAllocateRecordPrototype()
{
    std::array<llvm::Type *, 4> params = {llvm::Type::getInt1Ty(state->TheContext),
                                          llvm::Type::getInt32Ty(state->TheContext),
                                          llvm::Type::getInt32Ty(state->TheContext),
                                          llvm::Type::getInt8PtrTy(state->TheContext)};
    auto functionType = llvm::FunctionType::get(llvm::Type::getInt8PtrTy(state->TheContext), params, false);
    runtimeAllocateRecordFunction = Function::Create(functionType, Function::ExternalLinkage, "_DI_allocateRecord",
                                                     &state->TheModule);
}

void Codegen::emitRuntimeAllocateFunctionSymbolPrototype()
{
    std::array<llvm::Type *, 4> params = {llvm::Type::getInt1Ty(state->TheContext),
                                          llvm::Type::getInt32Ty(state->TheContext),
                                          llvm::Type::getInt32Ty(state->TheContext),
                                          llvm::Type::getInt8PtrTy(state->TheContext)};
    auto functionType = llvm::FunctionType::get(llvm::Type::getInt8PtrTy(state->TheContext), params, false);
    runtimeAllocateFunctionSymbolFunction = Function::Create(functionType, Function::ExternalLinkage,
                                                             "_DI_allocateFunctionSymbol", &state->TheModule);
}

void Codegen::emitRuntimeAllocateVariablePrototype()
{
    std::array<llvm::Type *, 1> params = {llvm::Type::getInt1Ty(state->TheContext)};
    auto functionType = llvm::FunctionType::get(llvm::Type::getInt8PtrTy(state->TheContext), params, false);
    runtimeAllocateVariableFunction = Function::Create(functionType, Function::ExternalLinkage, "_DI_allocateVariable",
                                                       &state->TheModule);
}

void Codegen::emitRuntimeAssignRecordFieldPrototype()
{
    std::array<llvm::Type *, 3> params = {llvm::Type::getInt8PtrTy(state->TheContext),
                                          llvm::Type::getInt8PtrTy(state->TheContext),
                                          llvm::Type::getInt32Ty(state->TheContext)};
    auto functionType = llvm::FunctionType::get(llvm::Type::getVoidTy(state->TheContext), params, false);
    runtimeAssignRecordFieldFunction = Function::Create(functionType, Function::ExternalLinkage,
                                                        "_DI_assignRecordField", &state->TheModule);
}

void Codegen::emitRuntimeAssignFunctionSymbolSubtermPrototype()
{
    std::array<llvm::Type *, 3> params = {llvm::Type::getInt8PtrTy(state->TheContext),
                                          llvm::Type::getInt8PtrTy(state->TheContext),
                                          llvm::Type::getInt32Ty(state->TheContext)};
    auto functionType = llvm::FunctionType::get(llvm::Type::getVoidTy(state->TheContext), params, false);
    runtimeAssignFunctionSymbolSubtermFunction = Function::Create(functionType, Function::ExternalLinkage,
                                                                  "_DI_assignFunctionSymbolSubterm", &state->TheModule);
}

void Codegen::emitRuntimeGetIntValuePrototype()
{
    std::array<llvm::Type *, 1> params = {llvm::Type::getInt8PtrTy(state->TheContext)};
    auto functionType = llvm::FunctionType::get(llvm::Type::getInt32Ty(state->TheContext), params, false);
    runtimeGetIntValueFunction = Function::Create(functionType, Function::ExternalLinkage, "_DI_getIntValue",
                                                  &state->TheModule);
}

void Codegen::emitRuntimeGetBoolValuePrototype()
{
    std::array<llvm::Type *, 1> params = {llvm::Type::getInt8PtrTy(state->TheContext)};
    auto functionType = llvm::FunctionType::get(llvm::Type::getInt1Ty(state->TheContext), params, false);
    runtimeGetBoolValueFunction = Function::Create(functionType, Function::ExternalLinkage, "_DI_getBoolValue",
                                                   &state->TheModule);
}

void Codegen::emitRuntimeGetPtrValuePrototype()
{
    std::array<llvm::Type *, 1> params = {llvm::Type::getInt8PtrTy(state->TheContext)};
    auto functionType = llvm::FunctionType::get(llvm::Type::getInt8PtrTy(state->TheContext), params, false);
    runtimeGetPtrValueFunction = Function::Create(functionType, Function::ExternalLinkage, "_DI_getPtrValue",
                                                  &state->TheModule);
}

void Codegen::emitRuntimeGetRecordMemberPrototype()
{
    std::array<llvm::Type *, 2> params = {llvm::Type::getInt8PtrTy(state->TheContext),
                                          llvm::Type::getInt32Ty(state->TheContext)};
    auto functionType = llvm::FunctionType::get(llvm::Type::getInt8PtrTy(state->TheContext), params, false);
    runtimeGetRecordMemberFunction = Function::Create(functionType, Function::ExternalLinkage, "_DI_getRecordMember",
                                                      &state->TheModule);
}

void Codegen::emitRuntimeUnifyPrototype()
{
    std::array<llvm::Type *, 2> params = {llvm::Type::getInt8PtrTy(state->TheContext),
                                          llvm::Type::getInt8PtrTy(state->TheContext)};
    auto functionType = llvm::FunctionType::get(llvm::Type::getInt1Ty(state->TheContext), params, false);
    runtimeUnifyFunction = Function::Create(functionType, Function::ExternalLinkage, "_DI_unify", &state->TheModule);
}

void Codegen::emitRuntimeContinueSldResolutionPrototype()
{
    auto functionType = llvm::FunctionType::get(llvm::Type::getInt1Ty(state->TheContext), false);
    runtimeContinueSldResolutionFunction = Function::Create(functionType, Function::ExternalLinkage,
                                                            "_DI_continueSldResolution", &state->TheModule);
}

void Codegen::emitRuntimeInvokePrototype()
{
    std::vector<llvm::Type *> args = {PointerType::getUnqual(PointerType::getUnqual(PointerType::getUnqual(
            llvm::FunctionType::get(llvm::Type::getInt1Ty(state->TheContext),
                                    {llvm::Type::getInt8PtrTy(state->TheContext),
                                     llvm::Type::getInt8PtrTy(state->TheContext)}, false)))),
                                      llvm::Type::getInt8PtrTy(state->TheContext),
                                      llvm::Type::getInt8PtrTy(state->TheContext)};
    auto functionType = llvm::FunctionType::get(llvm::Type::getInt1Ty(state->TheContext), args, false);
    runtimeInvokeFunction = Function::Create(functionType, Function::ExternalLinkage, "_DI_invoke", &state->TheModule);
}

void Codegen::emitRuntimeFailedPrototype()
{
    auto functionType = llvm::FunctionType::get(llvm::Type::getVoidTy(state->TheContext), false);
    runtimeFailedFunction = Function::Create(functionType, Function::ExternalLinkage, "_DI_failed", &state->TheModule);
}

void Codegen::emitRuntimeEqualsPrototype()
{
    std::array<llvm::Type *, 2> args = {llvm::Type::getInt8PtrTy(state->TheContext),
                                        llvm::Type::getInt8PtrTy(state->TheContext)};
    auto functionType = llvm::FunctionType::get(llvm::Type::getInt1Ty(state->TheContext), args, false);
    runtimeEqualsFunction = Function::Create(functionType, Function::ExternalLinkage, "_DI_equals", &state->TheModule);
}

void Codegen::emitRuntimeDeepCopyRepresentationPrototype()
{
    std::array<llvm::Type *, 1> args = {llvm::Type::getInt8PtrTy(state->TheContext)};
    auto functionType = llvm::FunctionType::get(llvm::Type::getInt8PtrTy(state->TheContext), args, false);
    runtimeDeepCopyRepresentationFunction = Function::Create(functionType, Function::ExternalLinkage,
                                                             "_DI_deepCopyRepresentation", &state->TheModule);
}

void Codegen::emitRuntimeSetPtrValuePrototype()
{
    std::array<llvm::Type *, 2> args = {llvm::Type::getInt8PtrTy(state->TheContext),
                                        llvm::Type::getInt8PtrTy(state->TheContext)};
    auto functionType = llvm::FunctionType::get(llvm::Type::getVoidTy(state->TheContext), args, false);
    runtimeSetPtrValueFunction = Function::Create(functionType, Function::ExternalLinkage, "_DI_setPtrValue",
                                                  &state->TheModule);
}

void Codegen::emitRuntimeAddGoalPrototype()
{
    std::array<llvm::Type *, 2> args = {PointerType::getUnqual(PointerType::getUnqual(PointerType::getUnqual(
            llvm::FunctionType::get(llvm::Type::getInt1Ty(state->TheContext),
                                    {llvm::Type::getInt8PtrTy(state->TheContext),
                                     llvm::Type::getInt8PtrTy(state->TheContext)}, false)))),
                                        llvm::Type::getInt8PtrTy(state->TheContext)};
    auto functionType = llvm::FunctionType::get(llvm::Type::getVoidTy(state->TheContext), args, false);
    runtimeAddGoalFunction = Function::Create(functionType, Function::ExternalLinkage, "_DI_addGoal",
                                              &state->TheModule);
}

Codegen::~Codegen()
{
    if (hasErrors()) {
        return;
    }
    emitFunctionsListInitializer();
    emitFunctionsExports();
    emitFunctionsCExports();
    if(dumpLlvm) {
        state->TheModule.dump();
    }

    // Initialize the target registry etc.
    InitializeAllTargetInfos();
    InitializeAllTargets();
    InitializeAllTargetMCs();
    InitializeAllAsmParsers();
    InitializeAllAsmPrinters();

    auto TargetTriple = sys::getDefaultTargetTriple();
    state->TheModule.setTargetTriple(TargetTriple);

    std::string Error;
    auto Target = TargetRegistry::lookupTarget(TargetTriple, Error);

    // Print an error and exit if we couldn't find the requested target.
    // This generally occurs if we've forgotten to initialise the
    // TargetRegistry or we have a bogus target triple.
    if (!Target) {
        reportError(nullptr, Error);
    }

    auto CPU = "generic";
    auto Features = "";

    TargetOptions opt;
    auto RM = Optional<Reloc::Model>();
    auto TheTargetMachine = Target->createTargetMachine(TargetTriple, CPU, Features, opt, RM);

    state->TheModule.setDataLayout(TheTargetMachine->createDataLayout());

    auto Filename = state->TheModule.getModuleIdentifier() + ".o";
    std::error_code EC;
    raw_fd_ostream dest(Filename, EC, sys::fs::F_None);

    if (EC) {
        reportError(nullptr, "Could not open file: " + EC.message());
    }

    legacy::PassManager pass;
    auto FileType = TargetMachine::CGFT_ObjectFile;

    if (TheTargetMachine->addPassesToEmitFile(pass, dest, FileType)) {
        reportError(nullptr, "TheTargetMachine can't emit a file of this type");
    }

    pass.run(state->TheModule);
    dest.flush();

}

llvm::AllocaInst *Codegen::createNewVariable(llvm::Function *function, const std::string &name)
{
    IRBuilder<> TmpB(&function->getEntryBlock(), function->getEntryBlock().begin());
    return state->Builder.CreateAlloca(llvm::Type::getInt8PtrTy(state->TheContext), nullptr, name);
}

Value *Codegen::emitGetSldLogMarker()
{
    return state->Builder.CreateCall(runtimeGetLogMarkerFunction, None, "logMarker");
}

Value *Codegen::emitTerm(llvm::Function *function, std::shared_ptr<Ast> expression, Value *value)
{
    class TermEmiter : public AstVisitorBase
    {
    public:
        explicit TermEmiter(Codegen &codegen, llvm::Function *function, Value *value)
                : codegen(codegen), function(function), value(value)
        {}

        void visit(std::shared_ptr<RecordDefinition> node) override
        {
            static std::unordered_set<std::shared_ptr<RecordDefinition>> activeRecordDefinitions;
            if (activeRecordDefinitions.find(node) != activeRecordDefinitions.end()) {
                codegen.reportError(node, "Recursive definition of record");
                return;
            }
            activeRecordDefinitions.insert(node);
            bool oldSkipVariableNames = skipVariableNames;
            skipVariableNames = true;
            std::vector<Constant *> recordNameChars;
            for (auto byte : node->name) {
                recordNameChars.push_back(codegen.state->Builder.getInt8(reinterpret_cast<std::uint8_t &>(byte)));
            }
            Constant *recordName = ConstantArray::get(
                    ArrayType::get(codegen.state->Builder.getInt8Ty(), node->name.size()), recordNameChars);

            AllocaInst *recordNameLocation = codegen.state->Builder.CreateAlloca(recordName->getType(),
                                                                                 ConstantInt::get(
                                                                                         codegen.state->Builder.getInt32Ty(),
                                                                                         node->name.size()));
            codegen.state->Builder.CreateStore(recordName, recordNameLocation);

            std::array<Value *, 4> args = {codegen.state->Builder.getInt1(annotated), codegen.state->Builder.getInt32(
                    static_cast<std::uint32_t>(node->fields.size())), codegen.state->Builder.getInt32(
                    static_cast<std::uint32_t >(recordNameChars.size())), recordNameLocation};
            CallInst *allocateRecordCall = codegen.state->Builder.CreateCall(codegen.runtimeAllocateRecordFunction,
                                                                             args, "record");
            auto oldValue = value;
            for (unsigned i = 0; i < node->fields.size(); ++i) {
                if (oldValue) {
                    value = codegen.state->Builder.CreateExtractValue(oldValue, {i});
                }
                node->fields[i].first->accept(*this);
                std::array<Value *, 3> args = {allocateRecordCall, result, codegen.state->Builder.getInt32(i)};
                codegen.state->Builder.CreateCall(codegen.runtimeAssignRecordFieldFunction, args);
            }
            value = oldValue;
            result = allocateRecordCall;
            skipVariableNames = oldSkipVariableNames;
            activeRecordDefinitions.erase(node);
        }

        void visit(std::shared_ptr<IntType> node) override
        {
            std::array<Value *, 3> args;
            if (value) {
                args = {codegen.state->Builder.getInt1(annotated), codegen.state->Builder.getTrue(), value};
            } else {
                args = {codegen.state->Builder.getInt1(annotated), codegen.state->Builder.getFalse(),
                        llvm::UndefValue::get(codegen.state->Builder.getInt32Ty())};
            }
            result = codegen.state->Builder.CreateCall(codegen.runtimeAllocateIntFunction, args, "int");
        }

        void visit(std::shared_ptr<BoolType> node) override
        {
            std::array<Value *, 3> args;
            if (value) {
                args = {codegen.state->Builder.getInt1(annotated), codegen.state->Builder.getTrue(), value};
            } else {
                args = {codegen.state->Builder.getInt1(annotated), codegen.state->Builder.getFalse(),
                        llvm::UndefValue::get(codegen.state->Builder.getInt8Ty())};
            }
            result = codegen.state->Builder.CreateCall(codegen.runtimeAllocateBoolFunction, args, "bool");
        }

        void visit(std::shared_ptr<VarType> node) override
        {
            std::array<Value *, 1> args = {codegen.state->Builder.getInt1(annotated)};
            result = codegen.state->Builder.CreateCall(codegen.runtimeAllocateVariableFunction, args, "var");
        }

        void visit(std::shared_ptr<NamedType> node) override
        {
            node->typeDefinition.lock()->accept(*this);
        }

        void visit(std::shared_ptr<AnnotatedType> node) override
        {
            bool oldAnnotated = annotated;
            annotated = true;
            node->type->accept(*this);
            annotated = oldAnnotated;
        }

        void visit(std::shared_ptr<PtrType> node) override
        {
            std::vector<std::uint8_t> pointeeType = getTypeIds(node->type);
            std::vector<Constant *> pointeeTypeElements;
            for (auto byte : pointeeType) {
                pointeeTypeElements.push_back(codegen.state->Builder.getInt8(byte));
            }
            Constant *pointeeTypeArray = ConstantArray::get(
                    ArrayType::get(codegen.state->Builder.getInt8Ty(), pointeeType.size()), pointeeTypeElements);

            AllocaInst *pointeeTypeArrayLocation = codegen.state->Builder.CreateAlloca(pointeeTypeArray->getType(),
                                                                                       ConstantInt::get(
                                                                                               codegen.state->Builder.getInt32Ty(),
                                                                                               pointeeType.size()));
            codegen.state->Builder.CreateStore(pointeeTypeArray, pointeeTypeArrayLocation);

            std::array<Value *, 4> args = {codegen.state->Builder.getInt1(annotated), codegen.state->Builder.getFalse(),
                                           llvm::UndefValue::get(codegen.state->Builder.getInt8PtrTy()),
                                           pointeeTypeArrayLocation};
            result = codegen.state->Builder.CreateCall(codegen.runtimeAllocatePtrFunction, args, "ptr");
        }

        void visit(std::shared_ptr<DecimalLiteralExpression> node) override
        {
            std::array<Value *, 3> args = {codegen.state->Builder.getInt1(annotated), codegen.state->Builder.getTrue(),
                                           llvm::ConstantInt::get(codegen.state->Builder.getInt32Ty(), node->rawValue,
                                                                  10)};
            result = codegen.state->Builder.CreateCall(codegen.runtimeAllocateIntFunction, args, "int_literal");
        }

        void visit(std::shared_ptr<BooleanLiteralExpression> node) override
        {
            std::array<Value *, 3> args = {codegen.state->Builder.getInt1(annotated), codegen.state->Builder.getTrue(),
                                           codegen.state->Builder.getInt8(
                                                   static_cast<std::uint8_t>(node->value ? 1 : 0))};
            result = codegen.state->Builder.CreateCall(codegen.runtimeAllocateBoolFunction, args, "bool_literal");
        }

        void visit(std::shared_ptr<NameReferenceExpression> node) override
        {
            result = codegen.state->Builder.CreateLoad(
                    codegen.state->NamedValues[node->atomicTermDefinitionExpression.lock()], node->name);
        }

        void visit(std::shared_ptr<AtomicTermDefinitionExpression> node) override
        {
            node->type->accept(*this);
            if (!skipVariableNames) {
                AllocaInst *allocaInst = codegen.createNewVariable(function, node->name);
                codegen.state->Builder.CreateStore(result, allocaInst);
                codegen.state->NamedValues[node] = allocaInst;
            }
            // result from node->type
        }

        void visit(std::shared_ptr<FunctionTermExpression> node) override
        {
            if (node->isCall) {
                auto quoteThisNode = std::make_shared<FunctionTermExpression>(node->name, node->subterms);
                result = codegen.emitTerm(function, std::make_shared<VarType>());
                std::array<Value *, 3> args = {codegen.functionsListVariable, codegen.emitTerm(function, quoteThisNode),
                                               result};
                Value *callResult = codegen.state->Builder.CreateCall(codegen.runtimeInvokeFunction, args,
                                                                      "call_result");
                BasicBlock *success = BasicBlock::Create(codegen.state->TheContext, "call_succesfull", function);
                codegen.state->Builder.CreateCondBr(callResult, success, codegen.failedBlock);
                codegen.state->Builder.SetInsertPoint(success);
            } else {
                bool isAnnotated = annotated;
                annotated = false;
                std::vector<Constant *> functionSymbolChars;
                for (auto byte : node->name) {
                    functionSymbolChars.push_back(
                            codegen.state->Builder.getInt8(reinterpret_cast<std::uint8_t &>(byte)));
                }
                Constant *functionSymbolName = ConstantArray::get(
                        ArrayType::get(codegen.state->Builder.getInt8Ty(), node->name.size()), functionSymbolChars);

                AllocaInst *functionSymbolNameLocation = codegen.state->Builder.CreateAlloca(
                        functionSymbolName->getType(),
                        ConstantInt::get(codegen.state->Builder.getInt32Ty(), node->name.size()));
                codegen.state->Builder.CreateStore(functionSymbolName, functionSymbolNameLocation);

                std::array<Value *, 4> args = {codegen.state->Builder.getInt1(isAnnotated),
                                               codegen.state->Builder.getInt32(
                                                       static_cast<std::uint32_t>(node->subterms.size())),
                                               codegen.state->Builder.getInt32(
                                                       static_cast<std::uint32_t >(functionSymbolChars.size())),
                                               functionSymbolNameLocation};
                CallInst *allocateFunctionSymbolCall = codegen.state->Builder.CreateCall(
                        codegen.runtimeAllocateFunctionSymbolFunction, args, "symbol");

                for (unsigned i = 0; i < node->subterms.size(); ++i) {
                    node->subterms[i]->accept(*this);
                    std::array<Value *, 3> args = {allocateFunctionSymbolCall, result,
                                                   codegen.state->Builder.getInt32(i)};
                    codegen.state->Builder.CreateCall(codegen.runtimeAssignFunctionSymbolSubtermFunction, args);
                }
                result = allocateFunctionSymbolCall;
            }
        }

        void visit(std::shared_ptr<DereferenceExpression> node) override
        {
            node->expression->accept(*this);
            std::array<Value *, 1> args = {result};
            result = codegen.state->Builder.CreateCall(codegen.runtimeGetPtrValueFunction, args, "ptr_value");
        }

        void visit(std::shared_ptr<AddressOfExpression> node) override
        {
            node->expression->accept(*this);
            std::vector<std::uint8_t> pointeeType = getTypeIds(node->expression->type);
            std::vector<Constant *> pointeeTypeElements;
            for (auto byte : pointeeType) {
                pointeeTypeElements.push_back(codegen.state->Builder.getInt8(byte));
            }
            Constant *pointeeTypeArray = ConstantArray::get(
                    ArrayType::get(codegen.state->Builder.getInt8Ty(), pointeeType.size()), pointeeTypeElements);

            AllocaInst *pointeeTypeArrayLocation = codegen.state->Builder.CreateAlloca(pointeeTypeArray->getType(),
                                                                                       ConstantInt::get(
                                                                                               codegen.state->Builder.getInt32Ty(),
                                                                                               pointeeType.size()));
            codegen.state->Builder.CreateStore(pointeeTypeArray, pointeeTypeArrayLocation);

            std::array<Value *, 4> args = {codegen.state->Builder.getTrue(), codegen.state->Builder.getTrue(), result,
                                           pointeeTypeArrayLocation};
            result = codegen.state->Builder.CreateCall(codegen.runtimeAllocatePtrFunction, args, "ptr");
        }

        void visit(std::shared_ptr<MemberExpression> node) override
        {
            node->expression->accept(*this);
            std::array<Value *, 2> args = {result, codegen.state->Builder.getInt32(node->index)};
            result = codegen.state->Builder.CreateCall(codegen.runtimeGetRecordMemberFunction, args, node->memberName);
        }

        void visit(std::shared_ptr<UnifyExpression> node) override
        {
            std::array<Value *, 2> args;
            node->leftExpression->accept(*this);
            args[0] = result;
            node->rightExpression->accept(*this);
            args[1] = result;
            Value *unifyResult = codegen.state->Builder.CreateCall(codegen.runtimeUnifyFunction, args);
            BasicBlock *nextBlock = BasicBlock::Create(codegen.state->TheContext, "", codegen.currentFunction);
            codegen.state->Builder.CreateCondBr(unifyResult, nextBlock, codegen.failedBlock);
            codegen.state->Builder.SetInsertPoint(nextBlock);
        }

        void visit(std::shared_ptr<AssignExpression> node) override
        {
            node->rightExpression->accept(*this);
            Value *valueToAssign = result;

            // Resolve location to store
            class Location
            {
            public:
                virtual ~Location() = default;
                virtual void detach(Codegen &codegen) = 0;
            };
            class LocationResolver : public AstVisitorBase
            {
                class AddressOnStack : public Location
                {
                public:
                    explicit AddressOnStack(Value *value)
                            : value(value)
                    {
                        assert(value);
                    }

                    void detach(Codegen &codegen) override
                    {
                        Value *oldRepresentation = codegen.state->Builder.CreateLoad(value, "old_representation");
                        Value *newRepresentation = codegen.state->Builder.CreateCall(
                                codegen.runtimeDeepCopyRepresentationFunction, oldRepresentation, "new_representation");
                        codegen.state->Builder.CreateStore(newRepresentation, value);
                    }

                private:
                    Value *value;
                };

                class Pointer : public Location
                {
                public:
                    explicit Pointer(Value *value)
                            : value(value)
                    {}

                    void detach(Codegen &codegen) override
                    {
                        Value *oldRepresentation = codegen.state->Builder.CreateCall(codegen.runtimeGetPtrValueFunction,
                                                                                     value, "old_representation");
                        Value *newRepresentation = codegen.state->Builder.CreateCall(
                                codegen.runtimeDeepCopyRepresentationFunction, oldRepresentation, "new_representation");
                        std::array<Value *, 2> args = {value, newRepresentation};
                        codegen.state->Builder.CreateCall(codegen.runtimeSetPtrValueFunction, args);
                    }

                private:
                    Value *value;
                };

            public:
                explicit LocationResolver(TermEmiter &termEmiter)
                        : termEmiter(termEmiter)
                {}

                void visit(std::shared_ptr<Expression> node) override
                {
                    assert(false && "Unreachable");
                }

                void visit(std::shared_ptr<LiteralExpression> node) override
                {
                    result = nullptr;
                }

                void visit(std::shared_ptr<NameReferenceExpression> node) override
                {
                    result = std::make_shared<AddressOnStack>(
                            termEmiter.codegen.state->NamedValues[node->atomicTermDefinitionExpression.lock()]);
                }

                void visit(std::shared_ptr<AtomicTermDefinitionExpression> node) override
                {
                    node->accept(termEmiter);
                    result = std::make_shared<AddressOnStack>(termEmiter.codegen.state->NamedValues[node]);
                }

                void visit(std::shared_ptr<FunctionTermExpression> node) override
                {
                    result = nullptr;
                }

                void visit(std::shared_ptr<UnaryOperatorExpression> node) override
                {
                    result = nullptr;
                }

                void visit(std::shared_ptr<BinaryOperatorExpression> node) override
                {
                    result = nullptr;
                }

                void visit(std::shared_ptr<AssignExpression> node) override
                {
                    assert(false && "Unreachable");
                }

                void visit(std::shared_ptr<DereferenceExpression> node) override
                {
                    Value *ptrRepresentation = termEmiter.codegen.emitTerm(termEmiter.codegen.currentFunction,
                                                                           node->expression);
                    result = std::make_shared<Pointer>(ptrRepresentation);
                }

                void visit(std::shared_ptr<MemberExpression> node) override
                {
                    node->expression->accept(*this);
                }

                std::shared_ptr<Location> getResult() const
                {
                    return result;
                }

            private:
                TermEmiter &termEmiter;
                std::shared_ptr<Location> result;
            };
            LocationResolver resolver(*this);
            node->leftExpression->accept(resolver);
            std::shared_ptr<Location> location = resolver.getResult();
            if (!location) {
                codegen.reportError(node->leftExpression, "Cannot assign - not a lvalue");
                result = valueToAssign;
                return;
            }

            // Deep copy old representation starting from location
            location->detach(codegen);

            // Store value in sublocation specified by expression
            class ValueStorer : public AstVisitorBase
            {
            public:
                explicit ValueStorer(Codegen &codegen, Value *value)
                        : codegen(codegen), value(value)
                {}

                void visit(std::shared_ptr<Expression>) override
                {
                    assert(false);
                }

                void visit(std::shared_ptr<NameReferenceExpression> node) override
                {
                    node->atomicTermDefinitionExpression.lock()->accept(*this);
                }

                void visit(std::shared_ptr<AtomicTermDefinitionExpression> node) override
                {
                    codegen.state->Builder.CreateStore(value, codegen.state->NamedValues[node]);
                }

                void visit(std::shared_ptr<DereferenceExpression> node) override
                {
                    std::array<Value *, 2> args = {codegen.emitTerm(codegen.currentFunction, node->expression), value};
                    codegen.state->Builder.CreateCall(codegen.runtimeSetPtrValueFunction, args);
                }

                void visit(std::shared_ptr<MemberExpression> node) override
                {
                    std::array<Value *, 3> args = {codegen.emitTerm(codegen.currentFunction, node->expression), value,
                                                   codegen.state->Builder.getInt32(node->index)};
                    codegen.state->Builder.CreateCall(codegen.runtimeAssignRecordFieldFunction, args);
                }

            private:
                Codegen &codegen;
                Value *const value;
            };
            ValueStorer storer(codegen, valueToAssign);
            node->leftExpression->accept(storer);
            result = valueToAssign;
        }

        void visit(std::shared_ptr<OrExpression> node) override
        {
            node->leftExpression->accept(*this);
            Value *leftValue = codegen.state->Builder.CreateCall(codegen.runtimeGetBoolValueFunction, result, "or_lhs");
            node->rightExpression->accept(*this);
            Value *rightValue = codegen.state->Builder.CreateCall(codegen.runtimeGetBoolValueFunction, result,
                                                                  "or_rhs");
            Value *orValue = codegen.state->Builder.CreateOr(leftValue, rightValue, "or_value");
            std::array<Value *, 3> args = {codegen.state->Builder.getTrue(), codegen.state->Builder.getTrue(), orValue};
            result = codegen.state->Builder.CreateCall(codegen.runtimeAllocateBoolFunction, args);
        }

        void visit(std::shared_ptr<AndExpression> node) override
        {
            node->leftExpression->accept(*this);
            Value *leftValue = codegen.state->Builder.CreateCall(codegen.runtimeGetBoolValueFunction, result, "or_lhs");
            node->rightExpression->accept(*this);
            Value *rightValue = codegen.state->Builder.CreateCall(codegen.runtimeGetBoolValueFunction, result,
                                                                  "or_rhs");
            Value *andValue = codegen.state->Builder.CreateAnd(leftValue, rightValue, "and_value");
            std::array<Value *, 3> args = {codegen.state->Builder.getTrue(), codegen.state->Builder.getTrue(),
                                           andValue};
            result = codegen.state->Builder.CreateCall(codegen.runtimeAllocateBoolFunction, args);
        }

        void visit(std::shared_ptr<EqualExpression> node) override
        {
            std::array<Value *, 2> args;
            node->leftExpression->accept(*this);
            args[0] = result;
            node->rightExpression->accept(*this);
            args[1] = result;
            Value *equalsValue = codegen.state->Builder.CreateCall(codegen.runtimeEqualsFunction, args, "equals_value");
            std::array<Value *, 3> allocateArgs = {codegen.state->Builder.getTrue(), codegen.state->Builder.getTrue(),
                                                   equalsValue};
            result = codegen.state->Builder.CreateCall(codegen.runtimeAllocateBoolFunction, allocateArgs,
                                                       "equals_term");
        }

        void visit(std::shared_ptr<NotEqualExpression> node) override
        {
            std::array<Value *, 2> args;
            node->leftExpression->accept(*this);
            args[0] = result;
            node->rightExpression->accept(*this);
            args[1] = result;
            Value *equalsValue = codegen.state->Builder.CreateCall(codegen.runtimeEqualsFunction, args, "equals_value");
            Value *notEqualsValue = codegen.state->Builder.CreateNot(equalsValue, "not_equals_value");
            std::array<Value *, 3> allocateArgs = {codegen.state->Builder.getTrue(), codegen.state->Builder.getTrue(),
                                                   notEqualsValue};
            result = codegen.state->Builder.CreateCall(codegen.runtimeAllocateBoolFunction, allocateArgs,
                                                       "not_equals_term");
        }

        void visit(std::shared_ptr<LessExpression> node) override
        {
            node->leftExpression->accept(*this);
            Value *leftValue = codegen.state->Builder.CreateCall(codegen.runtimeGetIntValueFunction, result,
                                                                 "less_lhs");
            node->rightExpression->accept(*this);
            Value *rightValue = codegen.state->Builder.CreateCall(codegen.runtimeGetIntValueFunction, result,
                                                                  "less_rhs");
            Value *cmpResult = codegen.state->Builder.CreateICmpSLT(leftValue, rightValue, "less_value");
            std::array<Value *, 3> args = {codegen.state->Builder.getTrue(), codegen.state->Builder.getTrue(),
                                           cmpResult};
            result = codegen.state->Builder.CreateCall(codegen.runtimeAllocateBoolFunction, args);
        }

        void visit(std::shared_ptr<GreaterExpression> node) override
        {
            node->leftExpression->accept(*this);
            Value *leftValue = codegen.state->Builder.CreateCall(codegen.runtimeGetIntValueFunction, result,
                                                                 "greater_lhs");
            node->rightExpression->accept(*this);
            Value *rightValue = codegen.state->Builder.CreateCall(codegen.runtimeGetIntValueFunction, result,
                                                                  "greater_rhs");
            Value *cmpResult = codegen.state->Builder.CreateICmpSGT(leftValue, rightValue, "greater_value");
            std::array<Value *, 3> args = {codegen.state->Builder.getTrue(), codegen.state->Builder.getTrue(),
                                           cmpResult};
            result = codegen.state->Builder.CreateCall(codegen.runtimeAllocateBoolFunction, args);
        }

        void visit(std::shared_ptr<LessOrEqualExpression> node) override
        {
            node->leftExpression->accept(*this);
            Value *leftValue = codegen.state->Builder.CreateCall(codegen.runtimeGetIntValueFunction, result,
                                                                 "less_or_equal_lhs");
            node->rightExpression->accept(*this);
            Value *rightValue = codegen.state->Builder.CreateCall(codegen.runtimeGetIntValueFunction, result,
                                                                  "less_or_equal_rhs");
            Value *cmpResult = codegen.state->Builder.CreateICmpSLE(leftValue, rightValue, "less_or_equal_value");
            std::array<Value *, 3> args = {codegen.state->Builder.getTrue(), codegen.state->Builder.getTrue(),
                                           cmpResult};
            result = codegen.state->Builder.CreateCall(codegen.runtimeAllocateBoolFunction, args);
        }

        void visit(std::shared_ptr<GreaterOrEqualExpression> node) override
        {
            node->leftExpression->accept(*this);
            Value *leftValue = codegen.state->Builder.CreateCall(codegen.runtimeGetIntValueFunction, result,
                                                                 "greater_or_equal_lhs");
            node->rightExpression->accept(*this);
            Value *rightValue = codegen.state->Builder.CreateCall(codegen.runtimeGetIntValueFunction, result,
                                                                  "greater_or_equal_rhs");
            Value *cmpResult = codegen.state->Builder.CreateICmpSGE(leftValue, rightValue, "greater_or_equal_value");
            std::array<Value *, 3> args = {codegen.state->Builder.getTrue(), codegen.state->Builder.getTrue(),
                                           cmpResult};
            result = codegen.state->Builder.CreateCall(codegen.runtimeAllocateBoolFunction, args);
        }

        void visit(std::shared_ptr<AddExpression> node) override
        {
            node->leftExpression->accept(*this);
            Value *leftValue = codegen.state->Builder.CreateCall(codegen.runtimeGetIntValueFunction, result, "add_lhs");
            node->rightExpression->accept(*this);
            Value *rightValue = codegen.state->Builder.CreateCall(codegen.runtimeGetIntValueFunction, result,
                                                                  "add_rhs");
            Value *addResult = codegen.state->Builder.CreateAdd(leftValue, rightValue, "add_value");
            std::array<Value *, 3> args = {codegen.state->Builder.getTrue(), codegen.state->Builder.getTrue(),
                                           addResult};
            result = codegen.state->Builder.CreateCall(codegen.runtimeAllocateIntFunction, args);
        }

        void visit(std::shared_ptr<SubtractExpression> node) override
        {
            node->leftExpression->accept(*this);
            Value *leftValue = codegen.state->Builder.CreateCall(codegen.runtimeGetIntValueFunction, result, "sub_lhs");
            node->rightExpression->accept(*this);
            Value *rightValue = codegen.state->Builder.CreateCall(codegen.runtimeGetIntValueFunction, result,
                                                                  "sub_rhs");
            Value *subResult = codegen.state->Builder.CreateSub(leftValue, rightValue, "sub_value");
            std::array<Value *, 3> args = {codegen.state->Builder.getTrue(), codegen.state->Builder.getTrue(),
                                           subResult};
            result = codegen.state->Builder.CreateCall(codegen.runtimeAllocateIntFunction, args);
        }

        void visit(std::shared_ptr<MultiplyExpression> node) override
        {
            node->leftExpression->accept(*this);
            Value *leftValue = codegen.state->Builder.CreateCall(codegen.runtimeGetIntValueFunction, result, "mul_lhs");
            node->rightExpression->accept(*this);
            Value *rightValue = codegen.state->Builder.CreateCall(codegen.runtimeGetIntValueFunction, result,
                                                                  "mul_rhs");
            Value *mulResult = codegen.state->Builder.CreateMul(leftValue, rightValue, "mul_value");
            std::array<Value *, 3> args = {codegen.state->Builder.getTrue(), codegen.state->Builder.getTrue(),
                                           mulResult};
            result = codegen.state->Builder.CreateCall(codegen.runtimeAllocateIntFunction, args);
        }

        void visit(std::shared_ptr<DivideExpression> node) override
        {
            node->leftExpression->accept(*this);
            Value *leftValue = codegen.state->Builder.CreateCall(codegen.runtimeGetIntValueFunction, result, "div_lhs");
            node->rightExpression->accept(*this);
            Value *rightValue = codegen.state->Builder.CreateCall(codegen.runtimeGetIntValueFunction, result,
                                                                  "div_rhs");
            Value *divResult = codegen.state->Builder.CreateSDiv(leftValue, rightValue, "div_value");
            std::array<Value *, 3> args = {codegen.state->Builder.getTrue(), codegen.state->Builder.getTrue(),
                                           divResult};
            result = codegen.state->Builder.CreateCall(codegen.runtimeAllocateIntFunction, args);
        }

        void visit(std::shared_ptr<ModuloExpression> node) override
        {
            node->leftExpression->accept(*this);
            Value *leftValue = codegen.state->Builder.CreateCall(codegen.runtimeGetIntValueFunction, result, "rem_lhs");
            node->rightExpression->accept(*this);
            Value *rightValue = codegen.state->Builder.CreateCall(codegen.runtimeGetIntValueFunction, result,
                                                                  "rem_rhs");
            Value *remResult = codegen.state->Builder.CreateSRem(leftValue, rightValue, "rem_value");
            std::array<Value *, 3> args = {codegen.state->Builder.getTrue(), codegen.state->Builder.getTrue(),
                                           remResult};
            result = codegen.state->Builder.CreateCall(codegen.runtimeAllocateIntFunction, args);
        }

        void visit(std::shared_ptr<NotExpression> node) override
        {
            node->expression->accept(*this);
            Value *expressionValue = codegen.state->Builder.CreateCall(codegen.runtimeGetBoolValueFunction, result,
                                                                       "not_expression");
            Value *notResult = codegen.state->Builder.CreateNot(expressionValue, "not_value");
            std::array<Value *, 3> args = {codegen.state->Builder.getTrue(), codegen.state->Builder.getTrue(),
                                           notResult};
            result = codegen.state->Builder.CreateCall(codegen.runtimeAllocateBoolFunction, args);
        }

        void visit(std::shared_ptr<NegateExpression> node) override
        {
            node->expression->accept(*this);
            Value *expressionValue = codegen.state->Builder.CreateCall(codegen.runtimeGetIntValueFunction, result,
                                                                       "neg_expression");
            Value *negResult = codegen.state->Builder.CreateNeg(expressionValue, "neg_value");
            std::array<Value *, 3> args = {codegen.state->Builder.getTrue(), codegen.state->Builder.getTrue(),
                                           negResult};
            result = codegen.state->Builder.CreateCall(codegen.runtimeAllocateIntFunction, args);
        }

        Value *getResult() const
        {
            return result;
        }

    private:
        Codegen &codegen;
        llvm::Function *function;
        Value *value;
        bool annotated = false;
        bool skipVariableNames = false;
        Value *result;
    };
    TermEmiter emiter(*this, function, value);
    expression->accept(emiter);
    return emiter.getResult();
}

llvm::Type *Codegen::getLlvmType(std::shared_ptr<::Type> typeNode)
{
    class TypeMaker : public AstVisitorBase
    {
    public:
        explicit TypeMaker(Codegen &codegen)
                : codegen(codegen)
        {}

        void visit(std::shared_ptr<RecordDefinition> node) override
        {
            std::vector<llvm::Type *> fields;
            for (auto &field : node->fields) {
                field.first->accept(*this);
                if (!type) {
                    return;
                }
                fields.push_back(type);
            }
            type = StructType::get(codegen.state->TheContext, fields);
        }

        void visit(std::shared_ptr<IntType> node) override
        {
            type = llvm::Type::getInt32Ty(codegen.state->TheContext);
        }

        void visit(std::shared_ptr<BoolType> node) override
        {
            type = llvm::Type::getInt1Ty(codegen.state->TheContext);
        }

        void visit(std::shared_ptr<VarType> node) override
        {
            type = nullptr;
        }

        void visit(std::shared_ptr<NamedType> node) override
        {
            node->typeDefinition.lock()->accept(*this);
        }

        void visit(std::shared_ptr<AnnotatedType> node) override
        {
            hasAnnotation = true;
            node->type->accept(*this);
        }

        void visit(std::shared_ptr<PtrType> node) override
        {
            // TODO: support Ptr exports
#if 0
            node->type->accept(*this);
            type = type->getPointerTo();
#else
            type = nullptr;
#endif
        }

        llvm::Type *getResult() const
        {
            return type;
        }

    private:
        llvm::Type *type;
        bool hasAnnotation = false;
        Codegen &codegen;
    };
    TypeMaker maker(*this);
    typeNode->accept(maker);
    return maker.getResult();
}

llvm::FunctionType *Codegen::getCExportType(std::shared_ptr<FunctionDefinition> functionNode)
{
    class ExportMaker : public AstVisitorBase
    {
    public:
        explicit ExportMaker(Codegen &codegen)
                : codegen(codegen)
        {}

        void visit(std::shared_ptr<FunctionDefinitionDeclarative> node) override
        {
            std::vector<llvm::Type *> args;
            for (auto &arg : node->terms) {
                arg->accept(*this);
                if (!type) {
                    return;
                }
                args.push_back(type);
            }
            type = llvm::FunctionType::get(llvm::Type::getVoidTy(codegen.state->TheContext), args, false);
        }

        void visit(std::shared_ptr<FunctionDefinitionImperative> node) override
        {
            if (node->result) {
                node->result->accept(*this);
            } else {
                type = llvm::Type::getVoidTy(codegen.state->TheContext);
            }
            if (hasAnnotation) {
                type = nullptr;
            }
            if (!type) {
                return;
            }
            llvm::Type *result = type;
            std::vector<llvm::Type *> args;
            for (auto &arg : node->terms) {
                arg->accept(*this);
                if (!type) {
                    return;
                }
                args.push_back(type);
            }
            type = llvm::FunctionType::get(result, args, false);
        }

        void visit(std::shared_ptr<NameReferenceExpression> node) override
        {
            node->atomicTermDefinitionExpression.lock()->accept(*this);
        }

        void visit(std::shared_ptr<AtomicTermDefinitionExpression> node) override
        {
            type = codegen.getLlvmType(node->type);
        }

        void visit(std::shared_ptr<FunctionTermExpression> node) override
        {
            type = nullptr;
        }

        llvm::FunctionType *getResult() const
        {
            return static_cast<llvm::FunctionType *>(type);
        }

    private:
        llvm::Type *type;
        bool hasAnnotation = false;
        Codegen &codegen;
    };
    ExportMaker exporter(*this);
    functionNode->accept(exporter);
    return exporter.getResult();
}

llvm::Value *Codegen::emitValue(llvm::Function *function, std::shared_ptr<Ast> expression, Value *value)
{
    class ValueEmiter : public AstVisitorBase
    {
    public:
        ValueEmiter(Codegen &codegen, llvm::Function *function, Value *value)
                : codegen(codegen), function(function), term(value)
        {
        }

        void visit(std::shared_ptr<RecordDefinition> node) override
        {
            auto oldTerm = term;
            StructType *recordType = dyn_cast<StructType>(codegen.getLlvmType(node));
            Value *resultTerm = UndefValue::get(recordType);
            for (unsigned i = 0; i < node->fields.size(); ++i) {
                std::array<Value *, 2> args = {oldTerm, codegen.state->Builder.getInt32(i)};
                term = codegen.state->Builder.CreateCall(codegen.runtimeGetRecordMemberFunction, args);
                node->fields[i].first->accept(*this);
                resultTerm = codegen.state->Builder.CreateInsertValue(resultTerm, result, i);
            }
            result = resultTerm;
        }

        void visit(std::shared_ptr<IntType>) override
        {
            result = codegen.state->Builder.CreateCall(codegen.runtimeGetIntValueFunction, term);
        }

        void visit(std::shared_ptr<BoolType>) override
        {
            result = codegen.state->Builder.CreateCall(codegen.runtimeGetBoolValueFunction, term);
        }

        void visit(std::shared_ptr<PtrType>) override
        {
            assert(false);
        }

        void visit(std::shared_ptr<VarType>) override
        {
            assert(false);
        }

        void visit(std::shared_ptr<NamedType> node) override
        {
            node->typeDefinition.lock()->accept(*this);
        }

        void visit(std::shared_ptr<AnnotatedType> node) override
        {
            node->type->accept(*this);
        }

        void visit(std::shared_ptr<AtomicTermDefinitionExpression> node) override
        {
            node->type->accept(*this);
        }

        Value *getResult() const
        {
            return result;
        }

    private:
        Codegen &codegen;
        llvm::Function *function;
        Value *term;
        Value *result;
    };
    ValueEmiter emiter(*this, function, value);
    expression->accept(emiter);
    return emiter.getResult();
}

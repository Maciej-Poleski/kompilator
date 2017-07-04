#ifndef KOMPILATOR_CODEGEN_HXX
#define KOMPILATOR_CODEGEN_HXX

#include <unordered_map>
#include "AstVisitorBase.hxx"
#include "ErrorCollector.hxx"

#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"

class Codegen : public AstVisitorBase, public ErrorCollector
{
    class State
    {
    public:
        explicit State(const std::string &moduleId);
        State(const State &) = delete;

        llvm::LLVMContext TheContext;
        llvm::IRBuilder<> Builder;
        llvm::Module TheModule;
        std::map<std::shared_ptr<AtomicTermDefinitionExpression>, llvm::AllocaInst *> NamedValues;
    };

public:
    explicit Codegen(const std::string &moduleId);
    ~Codegen();

    void visit(std::shared_ptr<FunctionDefinitionDeclarative> node) override;
    void visit(std::shared_ptr<FunctionDefinitionImperative> node) override;
    void visit(std::shared_ptr<FunctionBodyDeclarative> node) override;
    void visit(std::shared_ptr<Expression> node) override;
    void visit(std::shared_ptr<FunctionTermExpression> node) override;
    void visit(std::shared_ptr<AtomicTermDefinitionExpression> node) override;
    void visit(std::shared_ptr<UnaryOperatorExpression> node) override;
    void visit(std::shared_ptr<BinaryOperatorExpression> node) override;
    void visit(std::shared_ptr<FunctionBodyImperative> node) override;
    void visit(std::shared_ptr<ExpressionStatement> node) override;
    void visit(std::shared_ptr<IfStatement> node) override;
    void visit(std::shared_ptr<ForStatement> node) override;
    void visit(std::shared_ptr<BreakStatement> node) override;
    void visit(std::shared_ptr<ContinueStatement> node) override;
    void visit(std::shared_ptr<FunctionBodyExtern> node) override;

//private:
    explicit Codegen(std::shared_ptr<State> state);

    void emitFunctionsListPrototype();
    void emitFunctionExport(llvm::Function *function, std::shared_ptr<FunctionDefinition> node);
    void emitFunctionImport(std::shared_ptr<FunctionDefinition> node);
    void emitFunctionCImport(std::shared_ptr<FunctionDefinition> node);
    void emitFunctionsListInitializer();
    void emitFunctionsExports();
    void emitFunctionsCExports();

    void emitRuntimeGetLogMarkerPrototype();
    void emitRuntimeRollbackToMarkerPrototype();
    void emitRuntimeAllocatePrototypes();
    void emitRuntimeAllocateIntPrototype();
    void emitRuntimeAllocateBoolPrototype();
    void emitRuntimeAllocatePtrPrototype();
    void emitRuntimeAllocateFunctionSymbolPrototype();
    void emitRuntimeAllocateRecordPrototype();
    void emitRuntimeAllocateVariablePrototype();
    void emitRuntimeAssignRecordFieldPrototype();
    void emitRuntimeAssignFunctionSymbolSubtermPrototype();
    void emitRuntimeGetIntValuePrototype();
    void emitRuntimeGetBoolValuePrototype();
    void emitRuntimeGetPtrValuePrototype();
    void emitRuntimeGetRecordMemberPrototype();
    void emitRuntimeUnifyPrototype();
    void emitRuntimeContinueSldResolutionPrototype();
    void emitRuntimeInvokePrototype();
    void emitRuntimeFailedPrototype();
    void emitRuntimeEqualsPrototype();
    void emitRuntimeDeepCopyRepresentationPrototype();
    void emitRuntimeSetPtrValuePrototype();
    void emitRuntimeAddGoalPrototype();

    llvm::AllocaInst *createNewVariable(llvm::Function *function, const std::string &name = "");

    llvm::Value *emitGetSldLogMarker();
    llvm::Value *emitTerm(llvm::Function *function, std::shared_ptr<Ast> expression, llvm::Value *value = nullptr);
    llvm::Value *emitValue(llvm::Function *function, std::shared_ptr<Ast> expression, llvm::Value *value = nullptr);

    llvm::Type *getLlvmType(std::shared_ptr<Type> type);
    llvm::FunctionType *getCExportType(std::shared_ptr<FunctionDefinition> functionNode);

    std::shared_ptr<State> state;
    llvm::Function *currentFunction;
    llvm::BasicBlock *failedBlock;
    llvm::BasicBlock *afterForBlock;
    llvm::BasicBlock *nextForBlock = nullptr;
    llvm::Value *result;
    llvm::Function *runtimeGetLogMarkerFunction;
    llvm::Function *runtimeRollbackToMarkerFunction;
    llvm::Function *runtimeAllocateIntFunction;
    llvm::Function *runtimeAllocateBoolFunction;
    llvm::Function *runtimeAllocatePtrFunction;
    llvm::Function *runtimeAllocateFunctionSymbolFunction;
    llvm::Function *runtimeAllocateRecordFunction;
    llvm::Function *runtimeAllocateVariableFunction;
    llvm::Function *runtimeAssignRecordFieldFunction;
    llvm::Function *runtimeAssignFunctionSymbolSubtermFunction;
    llvm::Function *runtimeGetIntValueFunction;
    llvm::Function *runtimeGetBoolValueFunction;
    llvm::Function *runtimeGetPtrValueFunction;
    llvm::Function *runtimeGetRecordMemberFunction;
    llvm::Function *runtimeUnifyFunction;
    llvm::Function *runtimeContinueSldResolutionFunction;
    llvm::Function *runtimeInvokeFunction;
    llvm::Function *runtimeFailedFunction;
    llvm::Function *runtimeEqualsFunction;
    llvm::Function *runtimeDeepCopyRepresentationFunction;
    llvm::Function *runtimeSetPtrValueFunction;
    llvm::Function *runtimeAddGoalFunction;

    std::vector<llvm::Constant *> functions;
    std::unordered_map<std::string, std::vector<llvm::Function *>> functionsForName;
    std::unordered_map<std::string, std::vector<std::tuple<llvm::Function *, llvm::FunctionType *, std::shared_ptr<FunctionDefinition>>>> functionsToExportC;
    llvm::GlobalVariable *functionsListVariable;
};


#endif //KOMPILATOR_CODEGEN_HXX

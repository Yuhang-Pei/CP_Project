//
// Created by Pei Yuhang on 2023/5/16.
//

#ifndef CP_PROJECT_CODEGEN_H
#define CP_PROJECT_CODEGEN_H

#include <map>
#include <stack>
#include <string>

#include <llvm/IR/Module.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Target/TargetOptions.h>

#include "AST.h"

static llvm::LLVMContext Context;

static llvm::IRBuilder<> Builder(Context);

using VarTable = std::map<std::string, llvm::Value *>;

class CodeGenContext {

    class CodeGenBlock {
    public:
        llvm::BasicBlock *basicBlock;
        llvm::Value *returnValue;
        VarTable localVars;

        CodeGenBlock(llvm::BasicBlock *basicBlock) : basicBlock(basicBlock), returnValue(nullptr) {}
    };

public:
    llvm::Module *module;

    CodeGenContext(const std::string &moduleID) : module(new llvm::Module(moduleID, Context)) {}

    void GenerateCode(AST::Prog *root);

    void GenerateObject(const std::string &fileName) const;

    void ExecuteCode();

    void DumpLLVMIR(const std::string &fileName) const;

    /* 基本块操作 */

    void PushBasicBlock(llvm::BasicBlock *basicBlock);

    void PopBasicBlock();

    llvm::BasicBlock *GetCurrentBlock() const { return this->blocks.back()->basicBlock; }

//    llvm::Type *GetCurrentReturnType() const { return this->blocks.top()->returnValue->getType(); }

    void SetCurrentReturnValue(llvm::Value *returnValue) { this->blocks.back()->returnValue = returnValue; }

    llvm::Value *GetCurrentReturnValue() const { return this->blocks.back()->returnValue; }

    /* 变量表操作 */

    VarTable &GetLocalVars() { return this->blocks.back()->localVars; }

    llvm::Value *GetLocalVar(const std::string &varName) { return GetLocalVars()[varName]; }

    bool AddLocalVar(llvm::Value *var, const std::string &varName);

    bool IsVarInLocal(const std::string &varName) { return !(GetLocalVars().find(varName) == GetLocalVars().end()); }

    llvm::Value *GetVar(const std::string &varName);

    bool IsVarDefined(const std::string &varName);

    /* 函数操作 */

    void SetMainFunc(llvm::Function *mainFunc) { this->mainFunc = mainFunc; }

    bool IsFuncExist(const std::string &funcName) { return !(this->module->getFunction(funcName)); }

    /* 当前函数操作 */

    void EnterFunc(llvm::Function *func) { this->currentFunc = func; }

    void LeaveFunc() { this->currentFunc = nullptr; }

    llvm::Function *GetCurrentFunc() const { return this->currentFunc; }

    std::string GetCurrentFuncName() const { return this->currentFunc->getName().str(); }

    llvm::BasicBlock *GetCurrentFuncEntryBlock() const { return &this->currentFunc->getEntryBlock(); }

    llvm::Type *GetCurrentReturnType() const { return this->currentFunc->getReturnType(); }

private:
    std::vector<CodeGenBlock *> blocks;
    llvm::Function *mainFunc;
    llvm::Function *currentFunc = nullptr;
};

#endif //CP_PROJECT_CODEGEN_H

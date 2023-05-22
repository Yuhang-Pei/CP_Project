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

using VarTable = std::map<std::string, llvm::Value *>;

class CodeGenContext {

    class CodeGenBlock {
    public:
        llvm::BasicBlock *basicBlock;
        llvm::Value *returnValue;
        VarTable localVars;
    };

public:
    llvm::Module *module;

    CodeGenContext(const std::string &moduleID) : module(new llvm::Module(moduleID, Context)) {}

    void GenerateCode(AST::Prog *root);

    void GenerateObject(const std::string &fileName) const;

    void ExecuteCode();

    void DumpLLVMIR(const std::string &fileName) const;

    void PushBasicBlock(llvm::BasicBlock *basicBlock);

    void PopBasicBlock();

    llvm::BasicBlock *GetCurrentBlock() { return this->blocks.top()->basicBlock; }

    llvm::Value *GetReturnValue() { return this->blocks.top()->returnValue; }

    void SetReturnValue(llvm::Value *returnValue) { this->blocks.top()->returnValue = returnValue; }

    void SetMainFunc(llvm::Function *mainFunc) { this->mainFunc = mainFunc; }

    VarTable &GetLocalVars() { return this->blocks.top()->localVars; }

    llvm::Value *GetLocalVar(const std::string &varName) { return GetLocalVars()[varName]; }

    void AllocateLocalVar(llvm::AllocaInst *allocaInst, const std::string &varName) { this->blocks.top()->localVars[varName] = allocaInst; }

    bool IsVarInLocal(const std::string &varName) { return !(GetLocalVars().find(varName) == GetLocalVars().end()); }

private:
    std::stack<CodeGenBlock *> blocks;
    llvm::Function *mainFunc;
};

#endif //CP_PROJECT_CODEGEN_H

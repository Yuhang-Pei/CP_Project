//
// Created by Pei Yuhang on 2023/5/15.
//

#include <iostream>

#include "AST.h"
#include "parser.hpp"

static llvm::LLVMContext Context;
static llvm::IRBuilder<> Builder(Context);
static llvm::Module *Module;

namespace AST {

    llvm::Value *ExprStmt::CodeGen(CodeGenContext *context) {
        std::cout << "Creating expression statement" << std::endl;
        return this->expr->CodeGen(context);
    }

    llvm::Value *Integer::CodeGen(CodeGenContext *context) {
        std::cout << "Creating integer: " << this->intVal << std::endl;
        return llvm::ConstantInt::get(llvm::Type::getInt32Ty(Context), this->intVal, true);
    }

    llvm::Value *AddExpr::CodeGen(CodeGenContext *context) {
        std::cout << "Creating add expression" << std::endl;
        llvm::Value *LHS = this->lhs->CodeGen(context);
        llvm::Value *RHS = this->rhs->CodeGen(context);
        return Builder.CreateAdd(LHS, RHS);
    }

    llvm::Value *Prog::CodeGen(CodeGenContext *context) {
        for (auto stmt: *(this->stmts))
            if (stmt)
                stmt->CodeGen(context);
        return nullptr;
    }

}
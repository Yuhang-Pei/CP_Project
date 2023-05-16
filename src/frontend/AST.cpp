//
// Created by Pei Yuhang on 2023/5/15.
//

#include "AST.h"

static llvm::LLVMContext Context;
static llvm::IRBuilder<> Builder(Context);
static llvm::Module *Module;

namespace AST {
    llvm::Value *Prog::CodeGen() {
        for (auto Stmt : *(this->Stmts))
            if (Stmt)
                Stmt->CodeGen();
        return nullptr;
    }

    llvm::Value *VarType::CodeGen() {
        return nullptr;
    }

    llvm::Type *BuiltInType::GetLLVMType() {
        if (this->LLVMType)
            return this->LLVMType;

        switch (this->Type) {
            case Void:   this->LLVMType = Builder.getVoidTy();   break;
            case Bool:   this->LLVMType = Builder.getInt1Ty();   break;
            case Char:   this->LLVMType = Builder.getInt8Ty();   break;
            case Int:    this->LLVMType = Builder.getInt32Ty();  break;
            case Float:  this->LLVMType = Builder.getFloatTy();  break;
            case Double: this->LLVMType = Builder.getDoubleTy(); break;
        }
        return this->LLVMType;
    }


    llvm::Value *AddExpr::CodeGen() {
        llvm::Value *L = this->LHS->CodeGen();
        llvm::Value *R = this->RHS->CodeGen();
        if (!L || !R)
            return nullptr;

        return Builder.CreateAdd(L, R);
    }

//    llvm::Value *AssignExpr::CodeGen() {
//
//    }
//
//    llvm::Value *AssignExpr::CodeGenPtr() {
//        llvm::Value *L = this->LHS->CodeGenPtr();
//        llvm::Value *R = this->RHS->CodeGen();
//
//    }

    llvm::Value *Integer::CodeGen() {
        return Builder.getInt32(this->IntVal);
    }
}
//
// Created by Pei Yuhang on 2023/5/15.
//

#ifndef CP_PROJECT_AST_H
#define CP_PROJECT_AST_H

#include <iostream>
#include <string>
#include <vector>

#include <llvm/IR/Value.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/PassManager.h>
#include <llvm/IR/CallingConv.h>
#include <llvm/IR/IRPrintingPasses.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/GlobalVariable.h>
#include <llvm/IRReader/IRReader.h>
#include <llvm/IR/ValueSymbolTable.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Verifier.h>
#include <llvm/ExecutionEngine/MCJIT.h>
#include <llvm/ExecutionEngine/Interpreter.h>
#include <llvm/ExecutionEngine/GenericValue.h>
#include <llvm/ExecutionEngine/SectionMemoryManager.h>
#include <llvm/Support/SourceMgr.h>
#include <llvm/Support/ManagedStatic.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/DynamicLibrary.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Passes/PassBuilder.h>
#include <llvm/Passes/PassPlugin.h>
#include <llvm/Transforms/InstCombine/InstCombine.h>
#include <llvm/Transforms/Scalar.h>
#include <llvm/Transforms/Scalar/GVN.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/Host.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Target/TargetOptions.h>

// LLVM global context
//extern llvm::LLVMContext Context;

// Uniform API for creating instructions and inserting them into a basic block
//extern llvm::IRBuilder<> Builder;

class CodeGenContext;

namespace AST {

    class Node;

    class Stmt;

    using Stmts = std::vector<Stmt *>;

    class ExprStmt;

    class Expr;

    class Integer;

    class AddExpr;

    class Prog;

}

namespace AST {

    class Node {
    public:
        Node() = default;

        virtual ~Node() = default;

        virtual llvm::Value *CodeGen(CodeGenContext *context) { return nullptr; }
    };

    class Stmt : public Node {
    };

    class Expr : public Node {
    };

    class ExprStmt : public Stmt {
    public:
        Expr *expr;

        ExprStmt(Expr *expr) : expr(expr) {}

        ~ExprStmt() = default;

        llvm::Value *CodeGen(CodeGenContext *context);
    };

    class Integer : public Expr {
    public:
        int intVal;

        Integer(int intVal) : intVal(intVal) {}

        ~Integer() = default;

        llvm::Value *CodeGen(CodeGenContext *context);
    };

    class AddExpr : public Expr {
    public:
        Expr *lhs;
        Expr *rhs;

        AddExpr(Expr *lhs, Expr *rhs) : lhs(lhs), rhs(rhs) {}

        ~AddExpr() = default;

        llvm::Value *CodeGen(CodeGenContext *context);
    };

    class Prog : public Node {
    public:
        Stmts *stmts;

        Prog(Stmts *stmts) : stmts(stmts) {}

        ~Prog() = default;

        llvm::Value *CodeGen(CodeGenContext *context);
    };

}

#endif //CP_PROJECT_AST_H

//
// Created by Pei Yuhang on 2023/5/15.
//

#ifndef CP_PROJECT_AST_H
#define CP_PROJECT_AST_H

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

namespace AST {
    class Node;

    class Prog;

    class Decl;
    using Decls = std::vector<Decl *>;

    class VarType;
        class BuiltInType;

    class Stmt;
    using Stmts = std::vector<Stmt *>;

    class Expr;
    using ExprList = std::vector<Expr *>;
        class AddExpr;
//        class AssignExpr;
        class Const;
            class Int;

}

namespace AST {

    class Node {
    public:
        Node() = default;
        ~Node() = default;
        virtual llvm::Value *CodeGen() = 0;
    };


    class Prog : public Node {
    public:
        Prog(Stmts *Stmts) : Stmts(Stmts) {}
        ~Prog() = default;
        llvm::Value *CodeGen() override;
    private:
        Stmts *Stmts;
    };


    class Stmt : public Node {
    public:
        Stmt() = default;
        ~Stmt() = default;
        llvm::Value *CodeGen() override = 0;
    };


    class Decl : public Stmt {
    public:
        Decl() = default;
        ~Decl() = default;
        llvm::Value *CodeGen() override = 0;
    };


    class VarType : public Node {
    public:
        VarType(): LLVMType(nullptr) {}
        ~VarType() = default;
        virtual llvm::Type *GetLLVMType() = 0;
        llvm::Value *CodeGen() override;
    protected:
        llvm::Type *LLVMType;
    };

    class BuiltInType : public VarType {
    public:
        enum TypeID {
            Void, Bool, Char, Int, Float, Double
        };
        BuiltInType(TypeID Type) : Type(Type) {}
        ~BuiltInType() = default;
        llvm::Type *GetLLVMType() override;
    private:
        TypeID Type;
    };


    class Expr : public Stmt {
    public:
        Expr() = default;
        ~Expr() = default;
        llvm::Value *CodeGen() override = 0;
        virtual llvm::Value *CodeGenPtr() = 0;
    };

    class AddExpr : public Expr {
    public:
        AddExpr(Expr *LHS, Expr *RHS) : LHS(LHS), RHS(RHS) {}
        ~AddExpr() = default;
        llvm::Value *CodeGen() override;
    private:
        Expr *LHS, *RHS;
    };

//    class AssignExpr : public Expr {
//    public:
//        AssignExpr(Expr *LHS, Expr *RHS) : LHS(LHS), RHS(RHS) {}
//        ~AssignExpr() = default;
//        llvm::Value *CodeGen() override;
//        llvm::Value *CodeGenPtr() override;
//    private:
//        Expr *LHS, *RHS;
//    };

    class Constant : public Expr {
    public:
        explicit Constant(BuiltInType::TypeID Type) : Type(Type) {}
        ~Constant() = default;
        llvm::Value *CodeGen() override = 0;
    protected:
        BuiltInType::TypeID Type;
    };

    class Integer : public Constant {
    public:
        Integer(int IntVal) : Constant(BuiltInType::Int), IntVal(IntVal) {}
        ~Integer() = default;
        llvm::Value *CodeGen() override;
    private:
        int IntVal;
    };

}


#endif //CP_PROJECT_AST_H

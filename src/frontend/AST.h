//
// Created by Pei Yuhang on 2023/5/15.
//

#ifndef CP_PROJECT_AST_H
#define CP_PROJECT_AST_H

#include <iostream>
#include <string>
#include <utility>
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


class CodeGenContext;

/* AST 节点的类声明 */

namespace AST {

    class Node;

    class Prog;

    class Unit;
    using Units = std::vector<Unit *>;

    class Def;
        class FuncDef;
            class Param;
            using Params = std::vector<Param *>;

    class TypeSpecifier;
        class BuiltInType;

    class Stmt;
    using Stmts = std::vector<Stmt *>;
        class Block;
        class ExprStmt;

    class Expr;
        class FuncCall;
            using Args = std::vector<Expr *>;
        class AddExpr;
        class Constant;
            class Integer;

}

/* AST 节点的类定义 */

namespace AST {

    class Node {
    public:
        Node() = default;

        virtual ~Node() = default;

        virtual llvm::Value *CodeGen(CodeGenContext *context) { return nullptr; }
    };

    class Unit : public Node {
    public:
        Unit() = default;

        virtual ~Unit() = default;
    };

    class Def : public Unit {
    public:
        Def() = default;

        virtual ~Def() = default;
    };

    class Param : public Node {
    public:
        TypeSpecifier *paramType;   // 形参类型
        std::string paramName;      // 形参名称

        Param(TypeSpecifier *paramType, std::string paramName) : paramType(paramType), paramName(std::move(paramName)) {}

        virtual ~Param() = default;

        // TODO: 暂时只实现了VOID，后续需要实现形参列表的CodeGen
        llvm::Value *CodeGen(CodeGenContext *content) { return nullptr; }
    };

    class FuncDef : public Def {
    public:
        TypeSpecifier *returnType;  // 函数返回类型
        std::string funcName;       // 函数名称
        Params *params;             // 函数形参列表
        Block *funcBody;            // 函数体

        FuncDef(TypeSpecifier *returnType, std::string funcName, Params *params, Block *funcBody) :
            returnType(returnType), funcName(std::move(funcName)), params(params), funcBody(funcBody) {}

        ~FuncDef() = default;

        llvm::Value *CodeGen(CodeGenContext *context);
    };

    class TypeSpecifier : public Node {
    public:
        llvm::Type *LLVMType;       // 类型对应的 LLVM 类型

        TypeSpecifier() : LLVMType(nullptr) {}

        virtual llvm::Type *GetLLVMType(CodeGenContext *context) = 0;
    };

    class BuiltInType : public TypeSpecifier {
    public:
        enum TypeID { _VOID, _INT };    // 可能预置类型的枚举编号
        TypeID type;                    // 内置类型的编号

        BuiltInType(TypeID type) : type(type) {}

        ~BuiltInType() = default;

        llvm::Type *GetLLVMType(CodeGenContext *context);
    };

    class Stmt : public Node {
    public:
        Stmt() = default;

        virtual ~Stmt() = default;
    };

    class Block : public Stmt {
    public:
        Stmts* stmts;   // 代码块内的语句列表

        Block(Stmts *stmts) : stmts(stmts) {}

        ~Block() = default;

        llvm::Value *CodeGen(CodeGenContext *context);
    };

    class Expr : public Node {
    public:
        Expr() = default;

        virtual ~Expr() = default;
    };

    class ExprStmt : public Stmt {
    public:
        Expr *expr;     // 表达式语句中的表达式部分

        ExprStmt(Expr *expr) : expr(expr) {}

        ~ExprStmt() = default;

        llvm::Value *CodeGen(CodeGenContext *context);
    };

    class FuncCall : public Expr {
    public:
        std::string funcName;   // 函数调用的函数名
        Args *args;             // 函数调用的实参列表

        FuncCall(std::string funcName, Args *args) : funcName(std::move(funcName)), args(args) {}

        ~FuncCall() = default;

        llvm::Value *CodeGen(CodeGenContext *context);
    };

    class AddExpr : public Expr {
    public:
        Expr *lhs;  // 加法表达式的左侧表达式
        Expr *rhs;  // 加法表达式的右侧表达式

        AddExpr(Expr *lhs, Expr *rhs) : lhs(lhs), rhs(rhs) {}

        ~AddExpr() = default;

        llvm::Value *CodeGen(CodeGenContext *context);
    };

    class Constant : public Expr {
    public:
        Constant() = default;

        virtual ~Constant() = default;
    };

    class Integer : public Constant {
    public:
        int intVal; // 整型类型的整型值

        Integer(int intVal) : intVal(intVal) {}

        ~Integer() = default;

        llvm::Value *CodeGen(CodeGenContext *context);
    };

    class Prog : public Node {
    public:
        Units *units;   // 程序中的基本单元组成的列表

        Prog(Units *units) : units(units) {}

        ~Prog() = default;

        llvm::Value *CodeGen(CodeGenContext *context);
    };

}

#endif //CP_PROJECT_AST_H

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
#include <llvm/IR/Type.h>
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
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/Host.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Target/TargetOptions.h>

#if LLVM_VERSION_MAJOR >= 16
#include <llvm/MC/TargetRegistry.h>
#endif


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
            class FuncBody;
        class VarDef;
            class VarInit;
            using VarInitList = std::vector<VarInit *>;

    class TypeSpecifier;
        class BuiltInType;
        class ArrType;
        class PtrType;

    class Stmt;
    using Stmts = std::vector<Stmt *>;
        class Block;
        class ExprStmt;
        class IfStmt;
        class ForStmt;
        class ReturnStmt;
        class EmptyStmt;

    class Expr;
        class FuncCall;
            using Args = std::vector<Expr *>;
        class SubscriptExpr;
        class AddExpr;
        class MulExpr;
        class DivExpr;
        class SubExpr;
        class EqExpr;
        class NeqExpr;
        class GreatExpr;
        class LessExpr;
        class AssignExpr;
        class CommaExpr;
        class Variable;
        class Constant;
            class Boolean;
            class Character;
            class Integer;
            class ConstString;
            class Real;
}

/* AST 节点的类定义 */

namespace AST {

    class Node {
    public:
        Node() = default;

        virtual ~Node() = default;

        virtual llvm::Value *CodeGen(CodeGenContext *context) = 0;
    };

    class Stmt : public Node {
    public:
        Stmt() = default;

        virtual ~Stmt() = default;
    };

    // 注意其继承了 Stmt
    class Unit : public Stmt {
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

        llvm::Value *CodeGen(CodeGenContext *context);
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

    class VarDef : public Def {
    public:
        TypeSpecifier *typeSpecifier;   // 变量类型
        VarInitList *varInitList;       // 变量初始化列表

        VarDef(TypeSpecifier *typeSpecifier, VarInitList *varInitList) : typeSpecifier(typeSpecifier), varInitList(varInitList) {}

        ~VarDef() = default;

        llvm::Value *CodeGen(CodeGenContext *context);
    };

    class VarInit : public Node {
    public:
        std::string varName;        // 变量名称
        Expr *initExpr;             // 变量初始化表达式
        TypeSpecifier *complexType; // 用于指明指针、数组构成的复杂类型

        VarInit(std::string varName, Expr *initExpr = nullptr) : varName(std::move(varName)), initExpr(initExpr), complexType(nullptr) {}

        VarInit(std::string varName, TypeSpecifier *complexType, TypeSpecifier *baseType, Expr *initExpr = nullptr);

        ~VarInit() = default;

        llvm::Value *CodeGen(CodeGenContext *context);

    private:
        TypeSpecifier *ReverseComplexType();
    };

    class TypeSpecifier : public Node {
    public:
        llvm::Type *LLVMType;       // 类型对应的 LLVM 类型
        bool isArr = false;
        bool isPtr = false;

        TypeSpecifier() : LLVMType(nullptr) {}

        virtual llvm::Type *GetLLVMType(CodeGenContext *context) = 0;

        virtual std::string GetTypeName() = 0;
    };

    class BuiltInType : public TypeSpecifier {
    public:
        enum TypeID {   // 可能预置类型的枚举编号
            _VOID,
            _BOOL,
            _CHAR,
            _INT,
            _DOUBLE
        };
        TypeID type;    // 内置类型的编号

        BuiltInType(TypeID type) : type(type) {}

        ~BuiltInType() = default;

        llvm::Type *GetLLVMType(CodeGenContext *context);

        std::string GetTypeName();

        llvm::Value *CodeGen(CodeGenContext *context) { return nullptr; }
    };

    class ArrType : public TypeSpecifier {
    public:
        TypeSpecifier *elementType = nullptr;
        size_t size;
        TypeSpecifier *_fatherType;

        ArrType(TypeSpecifier *_fatherType, const size_t size) : _fatherType(_fatherType), size(size) { this->isArr = true; }

        ~ArrType() = default;

        // TODO: 以下函数未实现
        llvm::Value *CodeGen(CodeGenContext *context) { return nullptr; }

        llvm::Type *GetLLVMType(CodeGenContext *context);

        std::string GetTypeName() { return "array"; }
    };

    class PtrType : public TypeSpecifier {
    public:
        TypeSpecifier *objectType = nullptr;
        TypeSpecifier *_fatherType;

        PtrType(TypeSpecifier *_fatherType) : _fatherType(_fatherType) { this->isPtr = true; }

        ~PtrType() = default;

        // TODO: 以下函数未实现
        llvm::Value *CodeGen(CodeGenContext *context) { return nullptr; }

        llvm::Type *GetLLVMType(CodeGenContext *context);

        std::string GetTypeName() { return "pointer"; }
    };

    class Block : public Stmt {
    public:
        Stmts* stmts;   // 代码块内的语句列表

        Block(Stmts *stmts) : stmts(stmts) {}

        ~Block() = default;

        virtual llvm::Value *CodeGen(CodeGenContext *context);
    };

    class FuncBody : public Block {
    public:
        FuncBody(Stmts *stmts) : Block(stmts) {}

        ~FuncBody() = default;

        llvm::Value *CodeGen(CodeGenContext *context) override;
    };

    class Expr : public Node {
    public:
        Expr() = default;

        virtual ~Expr() = default;

        virtual llvm::Value *CodeGen(CodeGenContext *context) = 0;

        // 获取表达式的指针，即左值
        virtual llvm::Value* CodeGenPtr(CodeGenContext *context) = 0;
    };

    class ExprStmt : public Stmt {
    public:
        Expr *expr;     // 表达式语句中的表达式部分

        ExprStmt(Expr *expr) : expr(expr) {}

        ~ExprStmt() = default;

        llvm::Value *CodeGen(CodeGenContext *context);
    };

    class IfStmt : public Stmt {
    public:
        Expr *condition;    // 条件表达式
        Stmt *thenStmt;     // 如果条件为真执行的语句
        Stmt *elseStmt;     // 如果条件为假执行的语句

        IfStmt(Expr *condition, Stmt *thenStmt, Stmt *elseStmt = nullptr) : condition(condition), thenStmt(thenStmt), elseStmt(elseStmt) {}

        ~IfStmt() = default;

        llvm::Value *CodeGen(CodeGenContext *context);
    };

    class ForStmt : public Stmt {
    public:
        Stmt *init;         // 循环前的初始化表达式
        Expr *condition;    // 循环继续执行或退出的条件表达式
        Expr *increment;    // 完成一次循环后的增量表达式
        Stmt *loopStmt;     // 循环体内的语句

        ForStmt(Stmt *init, Expr *condition, Expr *increment, Stmt *loopStmt) : init(init), condition(condition), increment(increment), loopStmt(loopStmt) {}

        ~ForStmt() = default;

        llvm::Value *CodeGen(CodeGenContext *context);
    };

    class ReturnStmt : public Stmt {
    public:
        Expr *returnVal;    // 返回表达式

        ReturnStmt(Expr *returnVal = nullptr) : returnVal(returnVal) {}

        ~ReturnStmt() = default;

        llvm::Value *CodeGen(CodeGenContext *context);
    };

    class EmptyStmt : public Stmt {
    public:
        EmptyStmt() = default;

        ~EmptyStmt() = default;

        llvm::Value *CodeGen(CodeGenContext *context) { return nullptr; }
    };

    class FuncCall : public Expr {
    public:
        std::string funcName;   // 函数调用的函数名
        Args *args;             // 函数调用的实参列表

        FuncCall(std::string funcName, Args *args) : funcName(std::move(funcName)), args(args) {}

        ~FuncCall() = default;

        llvm::Value *CodeGen(CodeGenContext *context);

        llvm::Value *CodeGenPtr(CodeGenContext *context);
    };

//    class SubscriptExpr : public Expr {
//    public:
//        Expr *array;
//        Expr *index;
//
//        SubscriptExpr(Expr *array, Expr *index) : array(array), index(index) {}
//
//        ~SubscriptExpr() = default;
//
//
//    };

    class AddExpr : public Expr {
    public:
        Expr *lhs;  // 加法表达式的左侧表达式
        Expr *rhs;  // 加法表达式的右侧表达式

        AddExpr(Expr *lhs, Expr *rhs) : lhs(lhs), rhs(rhs) {}

        ~AddExpr() = default;

        llvm::Value *CodeGen(CodeGenContext *context);

        llvm::Value *CodeGenPtr(CodeGenContext *context);
    };

    class MulExpr : public Expr {
    public:
        Expr *lhs;  // 乘法表达式的左侧表达式
        Expr *rhs;  // 乘法表达式的右侧表达式

        MulExpr(Expr *lhs, Expr *rhs) : lhs(lhs), rhs(rhs) {}

        ~MulExpr() = default;

        llvm::Value *CodeGen(CodeGenContext *context);

        llvm::Value *CodeGenPtr(CodeGenContext *context);
    };

    class DivExpr : public Expr {
    public:
        Expr *lhs;  // 除法表达式的左侧表达式
        Expr *rhs;  // 除法表达式的右侧表达式

        DivExpr(Expr *lhs, Expr *rhs) : lhs(lhs), rhs(rhs) {}

        ~DivExpr() = default;

        llvm::Value *CodeGen(CodeGenContext *context);

        llvm::Value *CodeGenPtr(CodeGenContext *context);
    };

    class SubExpr : public Expr {
    public:
        Expr *lhs;  // 减法表达式的左侧表达式
        Expr *rhs;  // 减法表达式的右侧表达式

        SubExpr(Expr *lhs, Expr *rhs) : lhs(lhs), rhs(rhs) {}

        ~SubExpr() = default;

        llvm::Value *CodeGen(CodeGenContext *context);

        llvm::Value *CodeGenPtr(CodeGenContext *context);
    };

    class EqExpr : public Expr {
    public:
        Expr *lhs;
        Expr *rhs;

        EqExpr(Expr *lhs, Expr *rhs) : lhs(lhs), rhs(rhs) {}

        ~EqExpr() = default;

        llvm::Value *CodeGen(CodeGenContext *context);

        llvm::Value *CodeGenPtr(CodeGenContext *context);
    };

    class NeqExpr : public Expr {
    public:
        Expr *lhs;
        Expr *rhs;

        NeqExpr(Expr *lhs, Expr *rhs) : lhs(lhs), rhs(rhs) {}

        ~NeqExpr() = default;

        llvm::Value *CodeGen(CodeGenContext *context);

        llvm::Value *CodeGenPtr(CodeGenContext *context);
    };

    class GreatExpr : public Expr {
    public:
        Expr *lhs;
        Expr *rhs;

        GreatExpr(Expr *lhs, Expr *rhs) : lhs(lhs), rhs(rhs) {}

        ~GreatExpr() = default;

        llvm::Value *CodeGen(CodeGenContext *context);

        llvm::Value *CodeGenPtr(CodeGenContext *context);
    };

    class LessExpr : public Expr {
    public:
        Expr *lhs;
        Expr *rhs;

        LessExpr(Expr *lhs, Expr *rhs) : lhs(lhs), rhs(rhs) {}

        ~LessExpr() = default;

        llvm::Value *CodeGen(CodeGenContext *context);

        llvm::Value *CodeGenPtr(CodeGenContext *context);
    };

    class AssignExpr : public Expr {
    public:
        Expr *lhs;  // 赋值符号左侧表达式
        Expr *rhs;  // 赋值符号右侧表达式

        AssignExpr(Expr *lhs, Expr *rhs) : lhs(lhs), rhs(rhs) {}

        ~AssignExpr() = default;

        llvm::Value *CodeGen(CodeGenContext *context);

        llvm::Value *CodeGenPtr(CodeGenContext *context);
    };

    class Variable : public Expr {
    public:
        std::string varName;

        Variable(std::string varName) : varName(std::move(varName)) {}

        ~Variable() = default;

        llvm::Value *CodeGen(CodeGenContext *context);

        llvm::Value *CodeGenPtr(CodeGenContext *context);
    };

    class Constant : public Expr {
    public:
        Constant() = default;

        virtual ~Constant() = default;

        virtual llvm::Value *CodeGen(CodeGenContext *context) = 0;

        llvm::Value *CodeGenPtr(CodeGenContext *context);
    };

    class Boolean : public Constant {
    public:
        bool boolVal;   // 布尔类型值

        Boolean(bool boolVal) : boolVal(boolVal) {}

        ~Boolean() = default;

        llvm::Value *CodeGen(CodeGenContext *context);
    };

    class Character : public Constant {
    public:
        char charVal;   // 字符类型的字符

        Character(char charVal) : charVal(charVal) {}

        ~Character() = default;

        llvm::Value *CodeGen(CodeGenContext *context);
    };

    class Integer : public Constant {
    public:
        int intVal; // 整型类型的整型值

        Integer(int intVal) : intVal(intVal) {}

        ~Integer() = default;

        llvm::Value *CodeGen(CodeGenContext *context);
    };

    class Real : public Constant {
    public:
        double doubleVal; // 整型类型的整型值

        Real(double doubleVal) : doubleVal(doubleVal) {}

        ~Real() = default;

        llvm::Value *CodeGen(CodeGenContext *context);
    };

    class ConstString : public Constant {
    public:
        std::string strVal; // 代表字符串常量字面量

        ConstString(std::string strVal) : strVal(std::move(strVal)) {}

        ~ConstString() = default;

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

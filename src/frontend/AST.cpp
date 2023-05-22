//
// Created by Pei Yuhang on 2023/5/15.
//

#include <iostream>

#include "AST.h"
#include "codegen.h"
#include "parser.hpp"

static llvm::IRBuilder<> Builder(Context);


/**
 * @brief 对以 root 为根节点的抽象语法树，遍历每个节点，生成代码
 * @param root 抽象语法树的根节点的指针
 */
void CodeGenContext::GenerateCode(AST::Prog *root) {
    std::cout << "\033[31mGenerating code for the program...\033[0m\n" << std::endl;

    // 为了处理全局变量和类型的定义，创建一个临时的 _AST_GLOBAL 函数
    std::vector<llvm::Type *> paramTypes;   // 为 _AST_GLOBAL 定义空的形参列表
    // 创建返回类型为空、形参列表为空、参数数量不可变的函数类型
    llvm::FunctionType *funcType =
            llvm::FunctionType::get(llvm::Type::getVoidTy(Context), llvm::ArrayRef(paramTypes), false);
    // 创建 _AST_GLOBAL 函数
    llvm::Function *globalFunc =
            llvm::Function::Create(funcType, llvm::GlobalValue::InternalLinkage, "_AST_GLOBAL", this->module);
    // 创建基本块
    llvm::BasicBlock *basicBlock =
            llvm::BasicBlock::Create(Context, "_AST_GLOBAL_entry", globalFunc, 0);

    // 基本块入栈
    PushBasicBlock(basicBlock);
    // 调用根节点的 CodeGen()，递归地调用抽象语法书各个节点的 CodeGen() 操作
    root->CodeGen(this);
    // 创建返回指令
    llvm::ReturnInst::Create(Context, basicBlock);
    // 基本块出栈
    PopBasicBlock();

    std::cout << "\033[32mCode of the program has been generated\033[0m\n" << std::endl;

    // LLVM 提供了 PassManager 类，可以将生成的 LLVM IR 中的模块信息打印到标准输出流 llvm::outs() 中
    // 可以直接在程序输出中看到生成的 LLVM IR 结果
    llvm::legacy::PassManager passManager;
    passManager.add(llvm::createPrintModulePass(llvm::outs()));
    std::cout << "\033[31mLLVM IR of the program:\033[0m\n" << std::endl;
    passManager.run(*(this->module));   // passManager执行，打印 LLVM IR
    std::cout << std::endl;
}

/**
 * @brief 生成源代码的目标代码
 * @param fileName 目标代码文件的名称
 */
void CodeGenContext::GenerateObject(const std::string &fileName) const {
    // TargetTriplet (目标三元组) 用来指定体系架构、操作系统和环境
    // 通过 getDefaultTargetTriple 可以获取到当前系统环境下相应的目标三元组
    auto targetTriplet = llvm::sys::getDefaultTargetTriple();

    // 初始化目标信息、汇编解析器等内容
    llvm::InitializeAllTargetInfos();
    llvm::InitializeAllTargets();
    llvm::InitializeAllTargetMCs();
    llvm::InitializeAllAsmParsers();
    llvm::InitializeAllAsmPrinters();

    std::string error;
    // 根据目标三元组获取对应的目标
    const llvm::Target *target = llvm::TargetRegistry::lookupTarget(targetTriplet, error);
    if (target == nullptr)
        throw std::runtime_error(error);

    // 生成重定位模型，用来指定链接器在链接时如何处理符号地址 (重定位在 OS 课程中讲过，可以回去复习)
    auto relocModel = std::optional<llvm::Reloc::Model>();
    // 创建 llvm::TargetMachine，它是将 LLVM IR 转化为目标机器代码的核心组建
    llvm::TargetMachine *targetMachine =
            target->createTargetMachine(targetTriplet, "generic", "", llvm::TargetOptions(), relocModel);

    // 设置 module 的数据布局，数据布局是 llvm::Module 的一个属性
    // llvm::DataLayout 描述了不同类型的数据在内存中的表示方式和布局方式
    // 数据布局与系统环境相关，因此需要通过 llvm::TargetMachine 的 createDataLayout() 来得到
    this->module->setDataLayout(targetMachine->createDataLayout());
    // 设置 module 的目标三元组
    this->module->setTargetTriple(targetTriplet);

    std::error_code errorCode;
    // 创建 llvm::raw_fd_ostream 类的输出文件流对象
    llvm::raw_fd_ostream objectFile(fileName, errorCode);
    // llvm::raw_fd_ostream objectFile(fileName, errorCode, llvm::sys::fs::OF_None);
    if (errorCode)
        throw std::runtime_error(errorCode.message());

    // 将输出文件的类型设为目标文件
    llvm::CodeGenFileType fileType = llvm::CGFT_ObjectFile;
    // 创建 PassManager，用于将生成的内容输出到目标文件中
    llvm::legacy::PassManager passManager;
    // 将 targetMachine 中的包含的优化和代码生成 pass 传入到 passManager 中
    targetMachine->addPassesToEmitFile(passManager, objectFile, nullptr, fileType);
    // passManager 执行，将目标代码输入到目标文件
    passManager.run(*this->module);
    // 刷新输出流
    objectFile.flush();
}

/**
 * @brief 直接执行编译后的源代码
 */
void CodeGenContext::ExecuteCode() {
    std::cout << "\033[31mExecuting code...\033[0m" << std::endl;

    // llvm::ExecutionEngion 能够对 LLVM IR 进行解释并执行
    llvm::ExecutionEngine *executionEngine =
            llvm::EngineBuilder(std::unique_ptr<llvm::Module>(this->module)).create();
    // 完成 llvm::ExecutionEngine 实例的初始化
    executionEngine->finalizeObject();

    // 创建一个空的参数列表
    // TODO: 如果要允许用户输入参数，则需要进一步增加参数列表的内容
    std::vector<llvm::GenericValue> emptyArg;
    // 利用 llvm::ExecutionEngion 的 runFunction()，直接运行 main 函数
    executionEngine->runFunction(this->mainFunc, emptyArg);

    std::cout << "\033[32mExecution finishes\033[0m" << std::endl;
}

/**
 * @brief 将 LLVM IR 输出到指定文件中
 * @param fileName LLVM IR 输出的文件的名称
 */
void CodeGenContext::DumpLLVMIR(const std::string &fileName) const {
    std::error_code errorCode;
    // 创建 llvm::raw_fd_ostream 类的输出文件流对象
    llvm::raw_fd_ostream llvmFile(fileName, errorCode);

    // 调用 llvm::Module 的 print() 方法，可以直接将 LLVM IR 输出到指定文件流中
    this->module->print(llvmFile, nullptr);
}

/**
 * @brief 将一个基本块压入基本块构成的栈
 * @param basicBlock 需要压入的基本块的指针
 */
void CodeGenContext::PushBasicBlock(llvm::BasicBlock *basicBlock) {
    this->blocks.push(new CodeGenBlock());
    this->blocks.top()->returnValue = nullptr;
    this->blocks.top()->basicBlock = basicBlock;
}

/**
 * @brief 将一个基本块弹出基本块构成的栈
 */
void CodeGenContext::PopBasicBlock() {
    CodeGenBlock *top = this->blocks.top();
    this->blocks.pop();
    delete top;
}


/* 以下是 AST 节点类型的方法实现（主要为 GenCode 方法） */

namespace AST {

    llvm::Value *Param::CodeGen(CodeGenContext *context) {
        std::cout << "Creating parameter " << this->paramName << "..." << std::endl;

        // 获取到该形参的 LLVM 类型
        llvm::Type *LLVMType = this->paramType->GetLLVMType(context);

        // 创建 llvm::AllocaInst 指令，在函数栈中为形参分配内存空间
        llvm::AllocaInst *alloca =
                new llvm::AllocaInst(LLVMType, 0, this->paramName, context->GetCurrentBlock());
        // 在变量表中插入 (paramName, allocaInst) 对
        context->AllocateLocalVar(alloca, this->paramName);

        std::cout << "Parameter " << this->paramName << " has be created" << std::endl;
        return alloca;
    }

    llvm::Value *VarDef::CodeGen(CodeGenContext *context) {
        llvm::Type *LLVMType = this->typeSpecifier->GetLLVMType(context);

        // 处理类型未知的错误
        if (LLVMType == nullptr)
            throw std::logic_error("Define variables with unknown type");
        // 处理定义 void 类型的错误
        if (LLVMType->isVoidTy())
            throw std::logic_error("Cannot define variables of \"void\" type");

        // 逐个创建 VarInitList 中的每个变量
        for (auto var : *this->varInitList) {
            std::cout << "Creating variable " << var->varName << " with type " << this->typeSpecifier->GetTypeName() << std::endl;

            // 创建 llvm::AllocaInst 指令，在函数栈中为变量分配内存空间
            llvm::AllocaInst *alloca =
                    new llvm::AllocaInst(LLVMType, 0, var->varName, context->GetCurrentBlock());
            // 在变量表中插入 (varName, allocaInst) 对
            context->AllocateLocalVar(alloca, var->varName);

            // 处理包含初始值的情况
            if (var->initExpr)
                new llvm::StoreInst(
                        var->initExpr->CodeGen(context),
                        context->GetLocalVar(var->varName),
                        false,
                        context->GetCurrentBlock());

            std::cout << "Variable " << var->varName << " has been created" << std::endl;
        }

        // 该函数的返回值不会使用，故返回空指针
        return nullptr;
    }

    llvm::Value *VarInit::CodeGen(CodeGenContext *context) {
        // 变量初始化结点不需要 CodeGen() 操作，故直接返回空指针
        return nullptr;
    }

    llvm::Type *BuiltInType::GetLLVMType(CodeGenContext *context) {
        // 如果 this->LLVMType 非空，直接将其作为返回
        if (this->LLVMType)
            return this->LLVMType;

        // 如果 this->LLVMType 为空，根据内置类型使用 IRBuilder 获取对应的 LLVM 内置类型
        switch (this->type) {
            case _VOID: this->LLVMType = Builder.getVoidTy(); break;
            case _INT:  this->LLVMType = Builder.getInt32Ty(); break;
        }

        return this->LLVMType;
    }

    std::string BuiltInType::GetTypeName() {
        switch (this->type) {
            case _VOID: return "void";
            case _INT:  return "int";
        }
    }

    llvm::Value *FuncDef::CodeGen(CodeGenContext *context) {
        std::cout << "Creating definition of function " << this->funcName << "()..." << std::endl;

        // 定义 llvm::Type 类型的函数形参类型列表
        std::vector<llvm::Type *> paramTypes;
        // 把 AST::Param 节点逐个转换为 llvm::Type
        for (auto param : *this->params)
            paramTypes.push_back(param->paramType->GetLLVMType(context));

        // 定义 llvm::Type 类型的函数返回类型
        llvm::Type *retType = this->returnType->GetLLVMType(context);

        // 定义 llvm::FunctionType 类型的函数类型
        // 函数类型由函数的形参类型列表和返回类型共同定义
        llvm::FunctionType *funcType =
                llvm::FunctionType::get(retType, llvm::ArrayRef(paramTypes), false);

        // 创建 llvm::Function 类型的函数
        // 链接方式默认使用 ExternalLinkage
        // TODO: 后续可能可以根据用户增加的修饰符来确定链接方式
        llvm::Function *func =
                llvm::Function::Create(funcType, llvm::GlobalValue::ExternalLinkage, this->funcName.c_str(), context->module);

        // 创建基本块
        llvm::BasicBlock *basicBlock = llvm::BasicBlock::Create(Context, this->funcName + "_entry", func);
        // 将基本块入栈
        context->PushBasicBlock(basicBlock);

        // 同时遍历 AST::Params 列表和 llvm::Function 的函数参数列表
        auto paramIter = this->params->begin();
        auto llvmParamIter = func->arg_begin();
        for (; paramIter != this->params->end(); ++paramIter, ++llvmParamIter) {
            // 每个 AST::Param 节点执行 CodeGen() 操作
            (*paramIter)->CodeGen(context);
            // 把每个 AST::Param 节点的形参名付给 llvm::Function 中的对应参数
            llvmParamIter->setName((*paramIter)->paramName);
            // 创建存数指令，将该形参存到内存中
            new llvm::StoreInst(
                    &*llvmParamIter,
                    context->GetLocalVar((*paramIter)->paramName),
                    false,
                    basicBlock);
        }

        // AST::Block 类型的函数体执行 CodeGen() 操作
        this->funcBody->CodeGen(context);

        // 创建函数返回的指令
        // 从栈顶获取到当前函数的返回值，作为返回指令的参数
        llvm::ReturnInst::Create(Context, context->GetReturnValue(), basicBlock);

        // 如果该函数为 main 函数，则将其设为 context 中的 main 函数
        if (this->funcName == "main")
            context->SetMainFunc(func);

        // 将基本块出栈
        context->PopBasicBlock();

        std::cout << "Definition of function " << this->funcName << "() has been created\n" << std::endl;
        return func;
    }

    llvm::Value *Block::CodeGen(CodeGenContext *context) {
        std::cout << "Creating block..." << std::endl;
        for (auto stmt : *(stmts))
            if (stmt)
                stmt->CodeGen(context); // 为 block 中的每个语句执行 CodeGen() 操作
        std::cout << "Block has be created" << std::endl;

        // 该函数的返回值不会被使用，故返回空指针
        return nullptr;
    }

    llvm::Value *ExprStmt::CodeGen(CodeGenContext *context) {
        std::cout << "Creating expression statement..." << std::endl;
        return this->expr->CodeGen(context);
    }

    llvm::Value *ReturnStmt::CodeGen(CodeGenContext *context) {
        std::cout << "Creating return statement..." << std::endl;

        // 对返回值表达式执行 CodeGen()
        llvm::Value *retVal = this->returnVal->CodeGen(context);
        // 将当前函数的返回值设为 llvm::Value 类型的 retVal
        context->SetReturnValue(retVal);

        return retVal;
    }

    llvm::Value *Integer::CodeGen(CodeGenContext *context) {
        std::cout << "Creating integer " << this->intVal << "..." << std::endl;
        // 返回 llvm::ConstantInt 类型的 32 比特整型常量，默认为有符号
        return llvm::ConstantInt::get(llvm::Type::getInt32Ty(Context), this->intVal, true);
    }

    llvm::Value *FuncCall::CodeGen(CodeGenContext *context) {
        std::cout << "Creating call to function " << this->funcName << "()..." << std::endl;

        // 根据调用函数名称，通过上下文获取该函数
        llvm::Function *func = context->module->getFunction(this->funcName);

        // 如果调用的函数没有被定义，则报错
        if (func == nullptr) {
            std::cerr << this->funcName << " is not a function" << std::endl;
            return nullptr;
        }

        // 定义 llvm::Value 类型的函数调用实参列表
        std::vector<llvm::Value *> argList;
        // 把 AST::Expr 节点逐个转换为 llvm::Value
        for (auto arg: *this->args)
            argList.push_back(arg->CodeGen(context));

        // 创建函数调用的指令
        llvm::CallInst *call = llvm::CallInst::Create(func, llvm::ArrayRef(argList), "", context->GetCurrentBlock());

        std::cout << "Call to function " << this->funcName << "() has been created" << std::endl;
        return call;
    }

    llvm::Value *AddExpr::CodeGen(CodeGenContext *context) {
        std::cout << "Creating add expression..." << std::endl;

        // 对左表达式执行 CodeGen() 操作
        llvm::Value *LHS = this->lhs->CodeGen(context);
        // 对右表达式执行 CodeGen() 操作
        llvm::Value *RHS = this->rhs->CodeGen(context);

        std::cout << "Add expression has been created" << std::endl;

        // 使用 IRBuilder 返回类型为 llvm::Value 的加法表达式
        return Builder.CreateAdd(LHS, RHS);
    }

    llvm::Value *Variable::CodeGen(CodeGenContext *context) {
        std::cout << "Creating reference to variable " << this->varName << "..." << std::endl;

        // 处理变量未定义的错误
        if (!context->IsVarInLocal(this->varName))
            throw std::logic_error("Variable \"" + this-> varName+ "\" is not a variable");

        // 创建一个取数指令
        llvm::LoadInst *load = new llvm::LoadInst(
                context->GetLocalVar(this->varName)->getType(),
                context->GetLocalVar(this->varName),
                this->varName,
                false,
                context->GetCurrentBlock());
        return load;
    }

    llvm::Value *Prog::CodeGen(CodeGenContext *context) {
        for (auto unit: *this->units)
            if (unit)
                unit->CodeGen(context); // 为程序中的每个 AST::Unit 执行 CodeGen() 操作
        // 该函数的返回值不会被使用，故返回空指针
        return nullptr;
    }

}
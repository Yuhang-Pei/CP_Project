//
// Created by Pei Yuhang on 2023/5/15.
//

#include <iostream>

#include "AST.h"
#include "codegen.h"
#include "parser.hpp"
#include "type.hpp"
#include "util.hpp"


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
    // 创建临时基本块
    llvm::BasicBlock *basicBlock =
            llvm::BasicBlock::Create(Context, "_AST_GLOBAL_entry", globalFunc, nullptr);

    // 基本块入栈
    PushBasicBlock(basicBlock);
    // 调用根节点的 CodeGen()，递归地调用抽象语法书各个节点的 CodeGen() 操作
    root->CodeGen(this);
    // 删除临时的基本块
    basicBlock->eraseFromParent();
    // 删除临时的全局函数
    globalFunc->eraseFromParent();
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

#if LLVM_VERSION_MAJOR >= 16
/**
 * @brief 生成源代码的目标代码
 * @param fileName 目标代码文件的名称
 */
void CodeGenContext::GenerateObject(const std::string &fileName) const {
    std::cout << "\033[31mGenerating object code file for the program...\033[0m" << std::endl;

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

    std::cout << "\033[32mObject code file has been generated: " << fileName << "\033[0m\n" << std::endl;
}
#endif

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
    this->blocks.push_back(new CodeGenBlock(basicBlock));
}

/**
 * @brief 将一个基本块弹出基本块构成的栈
 */
void CodeGenContext::PopBasicBlock() {
    CodeGenBlock *top = this->blocks.back();
    this->blocks.pop_back();
    delete top;
}

bool CodeGenContext::AddLocalVar(llvm::Value *var, const std::string &varName) {
    // 如果当前栈为空，即没有基本块，则添加变量失败
    if (this->blocks.size() == 0)
        return false;

    // 如果当前基本块内已经定义了名为 varName 的变量，则添加变量失败
    if (IsVarInLocal(varName))
        return false;

    VarTable &currentVarTable = this->blocks.back()->localVars;  // 获取当前符号表
    currentVarTable[varName] = var; // 在符号表中添加 (varName, var) 键值对
    return true;
}

llvm::Value *CodeGenContext::GetVar(const std::string &varName)  {
    for (auto block = blocks.rbegin(); block != blocks.rend(); ++block)
        if ((*block)->localVars.find(varName) != (*block)->localVars.end())
            return (*block)->localVars[varName];

    return nullptr;
}

bool CodeGenContext::IsVarDefined(const std::string &varName) {
    for (auto block = blocks.rbegin(); block != blocks.rend(); ++block)
        if ((*block)->localVars.find(varName) != (*block)->localVars.end())
            return true;
    return false;
}

/* 以下是 AST 节点类型的方法实现（主要为 GenCode 方法） */

namespace AST {

    llvm::Value *Param::CodeGen(CodeGenContext *context) {
        std::cout << "Creating parameter " << this->paramName << "..." << std::endl;

        // 获取到该形参的 LLVM 类型
        llvm::Type *LLVMType = this->paramType->GetLLVMType(context);

        // 创建 llvm::AllocaInst 指令，在函数栈中为形参分配内存空间
        llvm::IRBuilder<> tmpBuilder(context->GetCurrentBlock());
        llvm::AllocaInst *alloca = tmpBuilder.CreateAlloca(LLVMType, nullptr, this->paramName);

        // 在变量表中插入 (paramName, alloca) 对
        context->AddLocalVar(alloca, this->paramName);

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

            llvm::IRBuilder<> tmpBuilder(context->GetCurrentBlock());
            llvm::AllocaInst *alloca = tmpBuilder.CreateAlloca(LLVMType, nullptr, var->varName);

            // 在变量表中插入 (varName, allocaInst) 对
            // 如果添加变量失败，将 alloca 从基本块中移除;
            if (!context->AddLocalVar(alloca, var->varName)) {
                alloca->eraseFromParent();
                throw std::logic_error("Refine variable " + var->varName);
            }

            // 处理包含初始值的情况
            if (var->initExpr) {
                /// TODO: 需要处理类型转换的问题
                Builder.CreateStore(var->initExpr->CodeGen(context), alloca);
            }

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

        // 如果 this->LLVMType 为空，根据内置类型获取对应的 LLVM 内置类型
        switch (this->type) {
            case _VOID: this->LLVMType = llvm::Type::getVoidTy(Context);  break;
            case _BOOL: this->LLVMType = llvm::Type::getInt1Ty(Context);  break;
            case _CHAR: this->LLVMType = llvm::Type::getInt8Ty(Context);  break;
            case _INT:  this->LLVMType = llvm::Type::getInt32Ty(Context); break;
            // case _FLOAT: this->LLVMType = LLVM::Type::getFloatTy(Context); break;
            case _DOUBLE: this->LLVMType = llvm::Type::getDoubleTy(Context); break;

        }

        return this->LLVMType;
    }

    std::string BuiltInType::GetTypeName() {
        switch (this->type) {
            case _VOID: return "void";
            case _BOOL: return "bool";
            case _CHAR: return "char";
            case _INT:  return "int";
            case _DOUBLE: return "double";
            // case _FLOAT : return "float"                                                     
        }
    }

    llvm::Value *FuncDef::CodeGen(CodeGenContext *context) {
        // 如果函数已经存在，将 func 从基本块中移除
        if (context->module->getFunction(this->funcName))
            throw std::logic_error("Function named " + this->funcName + " has already been defined");

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
        llvm::Function *func =
                llvm::Function::Create(funcType, llvm::GlobalValue::ExternalLinkage, this->funcName, context->module);

        // 创建基本块
        llvm::BasicBlock *basicBlock = llvm::BasicBlock::Create(Context, this->funcName + "_entry", func);
        // 利用 Builder 将当前插入点设为该函数的 entry 基本块
        Builder.SetInsertPoint(basicBlock);
        // 将基本块入栈
        context->EnterFunc(func);
        context->PushBasicBlock(basicBlock);

        // 同时遍历 AST::Params 列表和 llvm::Function 的函数参数列表
        auto paramIter = this->params->begin();
        auto llvmParamIter = func->arg_begin();
        for (; paramIter != this->params->end(); ++paramIter, ++llvmParamIter) {
            // 把每个 AST::Param 节点的形参名付给 llvm::Function 中的对应参数
            llvmParamIter->setName((*paramIter)->paramName);
            // 每个 AST::Param 节点执行 CodeGen() 操作
            // 并创建存储指令
            Builder.CreateStore(llvmParamIter, (*paramIter)->CodeGen(context));
        }

        // AST::Block 类型的函数体执行 CodeGen() 操作
        this->funcBody->CodeGen(context);

        // 如果该函数为 main 函数，则将其设为 context 中的 main 函数
        if (this->funcName == "main")
            context->SetMainFunc(func);

        // 将基本块出栈
        context->PopBasicBlock();
        context->LeaveFunc();

        std::cout << "Definition of function " << this->funcName << "() has been created\n" << std::endl;
        return func;
    }

    llvm::Value *Block::CodeGen(CodeGenContext *context) {
        std::cout << "Creating block..." << std::endl;

        llvm::Function *currentFunc = context->GetCurrentFunc();

        // 为 block 创建一个基本块
        llvm::BasicBlock *blockBB = llvm::BasicBlock::Create(Context, "block");

        // 创建无条件分支语句，跳转到 blockBB
        Builder.CreateBr(blockBB);

        // 将 block 插入到当前函数的基本块列表的末尾
        InsertFuncBasicBlockList(currentFunc, blockBB);
        // 将该 block 设为指令插入点
        Builder.SetInsertPoint(blockBB);
        // 将 blockBB 基本块入栈
        context->PushBasicBlock(blockBB);

        for (auto stmt : *this->stmts)
            if (stmt)
                stmt->CodeGen(context); // 为 block 中的每个语句执行 CodeGen() 操作

        // 将 blockBB 基本块出栈
        context->PopBasicBlock();

        std::cout << "Block has be created" << std::endl;

        // 该函数的返回值不会被使用，故返回空指针
        return nullptr;
    }

    llvm::Value *FuncBody::CodeGen(CodeGenContext *context) {
        std::cout << "Creating function body of function " << context->GetCurrentFuncName() << "()..." << std::endl;

        for (auto stmt : *this->stmts)
            // 如果到达基本块的终止指令（如 return），则停止生成代码
            if (Builder.GetInsertBlock()->getTerminator())
                break;
            else
                stmt->CodeGen(context);

        // 如果该函数没有 return，则创建一个默认的返回值
        if (!Builder.GetInsertBlock()->getTerminator()) {
            // 获取当前函数的返回类型
            llvm::Type *returnType = context->GetCurrentReturnType();

            if (returnType->isVoidTy())
                // 如果返回类型为 void，创建返回 void 的指令
                Builder.CreateRetVoid();
            else
                // 其他情况下，用 llvm::UndefValue 来创建一个 returnType 类型的为定义的值
                Builder.CreateRet(llvm::UndefValue::get(returnType));
        }

        // 该函数的返回值不会被使用，故返回空指针
        return nullptr;
    }

    llvm::Value *ExprStmt::CodeGen(CodeGenContext *context) {
        std::cout << "Creating expression statement..." << std::endl;
        return this->expr->CodeGen(context);
    }

    llvm::Value *IfStmt::CodeGen(CodeGenContext *context) {
        std::cout << "Creating if statement..." << std::endl;

        // 获取条件表达式的结果
        // 并将条件表达式转换为 1 比特整型（布尔类型）
        llvm::Value *ifCondition = CastToBool(this->condition->CodeGen(context));

        // 获取当前函数
        llvm::Function *currentFunc = context->GetCurrentFunc();

        // 构造 then 基本块
        llvm::BasicBlock *thenBB = llvm::BasicBlock::Create(Context, "then");
        // 构造 else 基本块
        llvm::BasicBlock *elseBB = llvm::BasicBlock::Create(Context, "else");
        // 构造 merge 基本块，用于条件语句之后的程序流的汇聚
        llvm::BasicBlock *mergeBB = llvm::BasicBlock::Create(Context, "merge");

        // 构造分支指令，条件为 true 时进入 thenBasicBlock，条件为 false 时进入 elseBasicBlock
        Builder.CreateCondBr(ifCondition, thenBB, elseBB);

        // 在 then 基本块中添加指令
        InsertFuncBasicBlockList(currentFunc, thenBB);    // 在函数的基本块列表的末尾添加 thenBB
        Builder.SetInsertPoint(thenBB); // 将插入指令的位置设为 thenBB
        if (this->thenStmt) {
            context->PushBasicBlock(thenBB);
            this->thenStmt->CodeGen(context);
            context->PopBasicBlock();
        }
        Builder.CreateBr(mergeBB);

        // 在 else 基本块中添加指令
        InsertFuncBasicBlockList(currentFunc, elseBB);    // 在函数的基本块列表的末尾添加 elseBB
        Builder.SetInsertPoint(elseBB); // 将插入指令的位置设为 elseBB
        if (this->elseStmt) {
            context->PushBasicBlock(elseBB);
            this->elseStmt->CodeGen(context);
            context->PopBasicBlock();
        }
        Builder.CreateBr(mergeBB);

        // 在 merge 基本块中添加指令
        InsertFuncBasicBlockList(currentFunc, mergeBB);   // 在函数的基本块列表的末尾添加 mergeBB
        Builder.SetInsertPoint(mergeBB);    // 将插入指令的位置设为 mergeBB

        std::cout << "If statement has been created" << std::endl;

        // 该函数的返回值不会使用，故返回空指针
        return nullptr;
    }

    llvm::Value *ForStmt::CodeGen(CodeGenContext *context) {
        std::cout << "Creating for loop statement..." << std::endl;

        // 获取当前函数
        llvm::Function *currentFunc = context->GetCurrentFunc();

        // 构造 loop 基本块，包含整个循环语句
        llvm::BasicBlock *loopBB = llvm::BasicBlock::Create(Context, "loop");
        // 构造 condition 基本块，包含条件表达式
        llvm::BasicBlock *conditionBB = llvm::BasicBlock::Create(Context, "context");
        // 构造 body 基本块，包含循环体
        llvm::BasicBlock *bodyBB = llvm::BasicBlock::Create(Context, "body");
        // 构造 increment 基本块，包含 increment 语句
        llvm::BasicBlock *incrementBB = llvm::BasicBlock::Create(Context, "increment");
        // 构造 end 基本块，是循环语句退出的位置
        llvm::BasicBlock *endBB = llvm::BasicBlock::Create(Context, "end");

        // 处理 init 表达式
        // 如果 init 表达式不为空，为 init 表达式生成代码
        if (this->init) {
            // 跳转到 loop 基本块
            Builder.CreateBr(loopBB);
            InsertFuncBasicBlockList(currentFunc, loopBB);  // 在函数的基本块列表的末尾添加 loopBB
            Builder.SetInsertPoint(loopBB); // 将插入指令的位置设置为 loopBB
            // 在 init 语句可能定义新变量，因此需要把 loopBB 基本块入栈，以包含新的变量
            context->PushBasicBlock(loopBB);
            this->init->CodeGen(context);
        }
        else
            delete loopBB;
        // 跳转到 condition 基本块
        Builder.CreateBr(conditionBB);

        // 处理循环条件表达式
        InsertFuncBasicBlockList(currentFunc, conditionBB);   // 在函数的基本块列表的末尾添加 conditionBB
        Builder.SetInsertPoint(conditionBB);    // 将插入指令的位置设置为 conditionBB
        if (this->condition) {
            // 如果 condition 表达式不为空，为 condition 表达式生成代码，并进行条件跳转
            llvm::Value *forCondition = CastToBool(this->condition->CodeGen(context));
            Builder.CreateCondBr(forCondition, bodyBB, endBB);
        }
        else
            // 如果 condition 表达式为空，则无条件跳转到 bodyBB，执行循环体
            Builder.CreateBr(bodyBB);

        // 处理循环体
        InsertFuncBasicBlockList(currentFunc, bodyBB);    // 在函数的基本块列表的末尾添加 conditionBB
        Builder.SetInsertPoint(bodyBB);         // 将插入指令的位置设置为 bodyBB
        context->PushBasicBlock(bodyBB);     // 将 body 基本块入栈
        this->loopStmt->CodeGen(context);
        context->PopBasicBlock();   // 将 body 基本块出栈
        Builder.CreateBr(incrementBB);  // 无条件跳转到 incrementBB

        // 处理 increment 表达式
        InsertFuncBasicBlockList(currentFunc, incrementBB); // 在函数的基本块列表的末尾添加 incrementBB
        Builder.SetInsertPoint(incrementBB);    // 将插入指令的位置设置为 incrementBB
        // 如果 increment 表达式不为空，为 increment 表达式生成代码
        if (this->increment)
            this->increment->CodeGen(context);
        Builder.CreateBr(conditionBB);  // 无条件跳转到 conditionBB

        // 处理 for 循环的结束
        InsertFuncBasicBlockList(currentFunc, endBB);
        Builder.SetInsertPoint(endBB);
        // 如果 init 语句不为空，则需要把之前压入的 loopBB 弹出
        if (this->init)
            context->PopBasicBlock();

        std::cout << "For loop statement has been created" << std::endl;

        // 该函数的返回值不会使用，故返回空指针
        return nullptr;
    }

    llvm::Value *ReturnStmt::CodeGen(CodeGenContext *context) {
        llvm::Function *func = context->GetCurrentFunc();   // 获取当前函数
        // 如果当前函数为 nullptr，即 return 被用在全局，则应抛出错误
        if (!func)
            throw std::logic_error("Return statement should be used in a function body");

        std::cout << "Creating return statement for function " << context->GetCurrentFuncName() << "()..." << std::endl;

        // 如果 this->returnVal == nullptr，说明 return 之后没有跟表达式
        if (!this->returnVal)
            if (func->getReturnType()->isVoidTy())
                Builder.CreateRetVoid();
            else
                throw std::logic_error("Expect an expression after \"return\"");
        else {
            // 对返回值表达式执行 CodeGen()
            llvm::Value *retVal = this->returnVal->CodeGen(context);
            // 利用 Builder 创建函数返回指令
            Builder.CreateRet(retVal);
            // 将当前函数的返回值设为 llvm::Value 类型的 retVal
            context->SetCurrentReturnValue(retVal);
        }

        // 该函数的返回值不会被使用，故返回空指针
        return nullptr;
    }

    llvm::Value *Boolean::CodeGen(CodeGenContext *context) {
        std::cout << "Creating boolean " << (this->boolVal ? "true" : "false") << "..." << std::endl;
        // 返回 llvm::ConstantInt 类型的 1 比特整型常量（即布尔类型），默认为无符号
        return llvm::ConstantInt::get(llvm::Type::getInt1Ty(Context), this->boolVal, false);
    }

    llvm::Value *Character::CodeGen(CodeGenContext *context) {
        std::cout << "Creating character \'" << this->charVal << "\'..." << std::endl;
        // 返回 llvm::ConstantInt 类型的 8 比特整型常量（即字符类型），默认为无符号
        return llvm::ConstantInt::get(llvm::Type::getInt8Ty(Context), this->charVal, false);
    }

    llvm::Value *Integer::CodeGen(CodeGenContext *context) {
        std::cout << "Creating integer " << this->intVal << "..." << std::endl;
        // 返回 llvm::ConstantInt 类型的 32 比特整型常量，默认为有符号
        return llvm::ConstantInt::get(llvm::Type::getInt32Ty(Context), this->intVal, true);
    }

    llvm::Value *Real::CodeGen(CodeGenContext *context){
        std::cout << "Creating real " << this->doubleVal << "..." << std::endl;
        // 返回 llvm::ConstantFP 类型的 实数型常量，默认为有符号
        return llvm::ConstantFP::get(llvm::Type::getDoubleTy(Context), this->doubleVal);
    }

    llvm::Value *ConstString::CodeGen(CodeGenContext *context) {
        std::cout << "Creating constant string \"" << this->strVal << "\"..." << std::endl;
        // 利用 IRBuilder 生成全局字符串常量，并返回字符串常量的指针
        // （在 C 语言中，字符串常量代表这一字符串第一个字符的内存指针）
        return Builder.CreateGlobalStringPtr(this->strVal);
    }

    llvm::Value *FuncCall::CodeGen(CodeGenContext *context) {
        std::cout << "Creating call to function " << this->funcName << "()..." << std::endl;

        // 根据调用函数名称，通过上下文获取该函数
        llvm::Function *func = context->module->getFunction(this->funcName);

        // 如果调用的函数没有被定义，则报错
        if (func == nullptr)
            throw std::logic_error(this->funcName + " is not a function");

        // 定义 llvm::Value 类型的函数调用实参列表
        std::vector<llvm::Value *> argList;
        // 把 AST::Expr 节点逐个转换为 llvm::Value
        for (auto arg: *this->args)
            argList.push_back(arg->CodeGen(context));

        // 创建函数调用的指令
        llvm::CallInst *call = Builder.CreateCall(func, argList);

        std::cout << "Call to function " << this->funcName << "() has been created" << std::endl;
        return call;
    }

    llvm::Value *FuncCall::CodeGenPtr(CodeGenContext *context) {
        throw std::logic_error("Function call cannot be used as left-value");
    }

    llvm::Value *AddExpr::CodeGen(CodeGenContext *context) {
        std::cout << "Creating addition expression..." << std::endl;

        // 对左表达式执行 CodeGen() 操作
        llvm::Value *LHS = this->lhs->CodeGen(context);
        // 对右表达式执行 CodeGen() 操作
        llvm::Value *RHS = this->rhs->CodeGen(context);

        std::cout << "Addition expression has been created" << std::endl;

        // 创建加法表达式指令
        // TODO: 只实现了整型的加法
        return Builder.CreateAdd(LHS, RHS);
    }

    llvm::Value *AddExpr::CodeGenPtr(CodeGenContext *context) {
        throw std::logic_error("Addition expression cannot be used as left-value");
    }

    llvm::Value *MulExpr::CodeGen(CodeGenContext *context) {
        std::cout << "Creating multiplication expression..." << std::endl;

        // 对左表达式执行 CodeGen() 操作
        llvm::Value *LHS = this->lhs->CodeGen(context);
        // 对右表达式执行 CodeGen() 操作
        llvm::Value *RHS = this->rhs->CodeGen(context);

        std::cout << "Multiplication expression has been created" << std::endl;

        // 创建加法表达式指令
        // TODO: 只实现了整型的加法
        return Builder.CreateMul(LHS, RHS);
    }

    llvm::Value *MulExpr::CodeGenPtr(CodeGenContext *context) {
        throw std::logic_error("Multiplication expression cannot be used as left-value");
    }

    llvm::Value *EqExpr::CodeGen(CodeGenContext *context) {
        std::cout << "Creating logical equality expression..." << std::endl;

        // 对左表达式执行 CodeGen() 操作
        llvm::Value *LHS = this->lhs->CodeGen(context);
        // 对右表达式执行 CodeGen() 操作
        llvm::Value *RHS = this->rhs->CodeGen(context);

        std::cout << "Logical equality expression has been created" << std::endl;

        // 创建逻辑等于表达式指令
        // TODO: 只实现了整型的逻辑等于
        return Builder.CreateICmpEQ(LHS, RHS);
    }

    llvm::Value *EqExpr::CodeGenPtr(CodeGenContext *context) {
        throw std::logic_error("Logical equality expression cannot be used as left-value");
    }

    llvm::Value *NeqExpr::CodeGen(CodeGenContext *context) {
        std::cout << "Creating logical inequality expression..." << std::endl;

        // 对左表达式执行 CodeGen() 操作
        llvm::Value *LHS = this->lhs->CodeGen(context);
        // 对右表达式执行 CodeGen() 操作
        llvm::Value *RHS = this->rhs->CodeGen(context);

        std::cout << "Logical inequality expression has been created" << std::endl;

        // 创建逻辑等于表达式指令
        // TODO: 只实现了整型的逻辑等于
        return Builder.CreateICmpNE(LHS, RHS);
    }

    llvm::Value *NeqExpr::CodeGenPtr(CodeGenContext *context) {
        throw std::logic_error("Logical inequality expression cannot be used as left-value");
    }

    llvm::Value *AssignExpr::CodeGen(CodeGenContext *context) {
        std::cout << "Creating assignment expression..." << std::endl;

        // 对左表达式获取指针
        llvm::Value *ptrLHS = this->lhs->CodeGenPtr(context);
        // 对右表达式执行 CodeGen() 操作
        llvm::Value *RHS = this->rhs->CodeGen(context);

        // TODO: 缺少赋值时的类型转换
        // 创建 Store 指令，把右表达式的值存入左表达式对应的地址
        Builder.CreateStore(RHS, ptrLHS);
        // 创建 Load 指令，以左表达式的值作为返回值
        llvm::Type *LHSType = GetPtrElementType(ptrLHS);
        return Builder.CreateLoad(LHSType, ptrLHS);
    }

    llvm::Value *AssignExpr::CodeGenPtr(CodeGenContext *context) {
        throw std::logic_error("Assignment expression cannot be used as left-value");
    }

    llvm::Value *Variable::CodeGen(CodeGenContext *context) {
        std::cout << "Creating reference to variable " << this->varName << "..." << std::endl;

        // 处理变量未定义的错误
        if (!context->IsVarDefined(this->varName))
            throw std::logic_error("Variable \"" + this-> varName+ "\" is not a variable");

        // 创建一个取数指令
        llvm::Value *varPtr = context->GetVar(this->varName);
        llvm::Type *varType = GetPtrElementType(varPtr);
        return Builder.CreateLoad(varType, varPtr, this->varName);
    }

    llvm::Value *Variable::CodeGenPtr(CodeGenContext *context) {
        std::cout << "Creating reference to variable " << this->varName << "..." << std::endl;

        // 处理变量未定义的错误
        if (!context->IsVarDefined(this->varName))
            throw std::logic_error("Variable \"" + this-> varName+ "\" is not a variable");

        return context->GetVar(this->varName);
    }

    llvm::Value *Constant::CodeGenPtr(CodeGenContext *context) {
        throw std::logic_error("Constant expression cannot be used as left-value");
    }

    llvm::Value *Prog::CodeGen(CodeGenContext *context) {
        for (auto unit: *this->units)
            if (unit)
                unit->CodeGen(context); // 为程序中的每个 AST::Unit 执行 CodeGen() 操作
        // 该函数的返回值不会被使用，故返回空指针
        return nullptr;
    }

}
//
// Created by Pei Yuhang on 2023/5/17.
//

#include <iostream>
#include "codegen.h"
#include "AST.h"

/**
 * @brief 创建一个在 LLVM IR 中可以调用的 printf 函数，用于定义其他基本输出函数
 * @param context 上下文
 * @return llvm::Function 指针类型的 printf 函数
 */
llvm::Function *CreatePrintfFunc(CodeGenContext *context) {
    std::vector<llvm::Type *> printfArgTypes;
    printfArgTypes.push_back(llvm::Type::getInt8PtrTy(Context));

    // 创建 printf 函数的类型
    // printf 函数的返回类型为32位整型，以 printfArgTypes 作为形参类型列表，true 表示参数数量可变
    llvm::FunctionType *printfFuncType =
            llvm::FunctionType::get(llvm::Type::getInt32Ty(Context), printfArgTypes, true);

    // 创建 printf 函数对应的 llvm::Function 实例
    llvm::Function *printfFunc =
            llvm::Function::Create(printfFuncType, llvm::Function::ExternalLinkage, llvm::Twine("printf"), context->module);

    printfFunc->setCallingConv(llvm::CallingConv::C);
    return printfFunc;
}

/**
 * @brief 创建一个打印 bool 类型的函数
 * @param context 上下文
 * @param printfFunc 可调用的 printf 函数
 */
void CreatePrintBoolFunc(CodeGenContext *context, llvm::Function *printfFunc) {
    // printBool() 的形参列表为一个整型变量
    std::vector<llvm::Type *> printBoolArgTypes;
    printBoolArgTypes.push_back(llvm::Type::getInt1Ty(Context));

    // 创建 printBool() 的函数类型，返回类型为 void
    llvm::FunctionType *printBoolFuncType =
            llvm::FunctionType::get(llvm::Type::getVoidTy(Context), printBoolArgTypes, false);

    // 创建 printBool() 函数
    llvm::Function *printBoolFunc =
            llvm::Function::Create(printBoolFuncType, llvm::Function::ExternalLinkage, llvm::Twine("printBool"), context->module);

    // 为 printBool() 创建基本块
    llvm::BasicBlock *basicBlock = llvm::BasicBlock::Create(Context, "printBool_entry", printBoolFunc, 0);
    // 基本块入栈
    context->PushBasicBlock(basicBlock);

    // 创建 printf() 的输出格式参数
    const std::string printBoolFormat = "%d\n";
    llvm::Constant *printBoolFormatStr = llvm::ConstantDataArray::getString(Context, printBoolFormat);
    // 获取输出格式参数的 LLVM 类型
    llvm::ArrayType *printBoolFormatType =
            llvm::ArrayType::get(llvm::IntegerType::get(Context, 8), printBoolFormat.length() + 1);
    // 创建一个全局变量，存储输出格式参数
    llvm::GlobalVariable *printBoolFormatVar =
            new llvm::GlobalVariable(*context->module, printBoolFormatType, true, llvm::GlobalValue::PrivateLinkage, printBoolFormatStr, ".printBoolFormatStr");

    // 获取 printBool() 函数的整型参数，作为 printf() 的参数，并命名为 "boolToPrint"
    llvm::Value *boolToPrint = printBoolFunc->arg_begin();
    boolToPrint->setName("boolToPrint");

    // 创建 printf() 函数的实参列表
    std::vector<llvm::Value *> printfArgs({ printBoolFormatVar, boolToPrint });

    // 发起对 printf() 的调用，以 printfArgs 作为实参列表
    llvm::CallInst::Create(printfFunc, llvm::ArrayRef(printfArgs), "", basicBlock);

    // 创建返回指令，从 printBool() 返回
    llvm::ReturnInst::Create(Context, basicBlock);

    // 基本块出栈
    context->PopBasicBlock();
}

/**
 * @brief 创建一个打印 char 类型的函数
 * @param context 上下文
 * @param printfFunc 可调用的 printf 函数
 */
void CreatePrintCharFunc(CodeGenContext *context, llvm::Function *printfFunc) {
    // printChar() 的形参列表为一个整型变量
    std::vector<llvm::Type *> printCharArgTypes;
    printCharArgTypes.push_back(llvm::Type::getInt8Ty(Context));

    // 创建 printChar() 的函数类型，返回类型为 void
    llvm::FunctionType *printCharFuncType =
            llvm::FunctionType::get(llvm::Type::getVoidTy(Context), printCharArgTypes, false);

    // 创建 printChar() 函数
    llvm::Function *printCharFunc =
            llvm::Function::Create(printCharFuncType, llvm::Function::ExternalLinkage, llvm::Twine("printChar"), context->module);

    // 为 printChar() 创建基本块
    llvm::BasicBlock *basicBlock = llvm::BasicBlock::Create(Context, "printChar_entry", printCharFunc, 0);
    // 基本块入栈
    context->PushBasicBlock(basicBlock);

    // 创建 printf() 的输出格式参数
    const std::string printCharFormat = "%c\n";
    llvm::Constant *printCharFormatStr = llvm::ConstantDataArray::getString(Context, printCharFormat);
    // 获取输出格式参数的 LLVM 类型
    llvm::ArrayType *printCharFormatType =
            llvm::ArrayType::get(llvm::IntegerType::get(Context, 8), printCharFormat.length() + 1);
    // 创建一个全局变量，存储输出格式参数
    llvm::GlobalVariable *printCharFormatVar =
            new llvm::GlobalVariable(*context->module, printCharFormatType, true, llvm::GlobalValue::PrivateLinkage, printCharFormatStr, ".printCharFormatStr");

    // 获取 printChar() 函数的整型参数，作为 printf() 的参数，并命名为 "charToPrint"
    llvm::Value *charToPrint = printCharFunc->arg_begin();
    charToPrint->setName("charToPrint");

    // 创建 printf() 函数的实参列表
    std::vector<llvm::Value *> printfArgs({ printCharFormatVar, charToPrint });

    // 发起对 printf() 的调用，以 printfArgs 作为实参列表
    llvm::CallInst::Create(printfFunc, llvm::ArrayRef(printfArgs), "", basicBlock);

    // 创建返回指令，从 printChar() 返回
    llvm::ReturnInst::Create(Context, basicBlock);

    // 基本块出栈
    context->PopBasicBlock();
}

/**
 * @brief 创建一个打印 int 类型的函数
 * @param context 上下文
 * @param printfFunc 可调用的 printf 函数
 */
void CreatePrintIntFunc(CodeGenContext *context, llvm::Function *printfFunc) {
    // printInt() 的形参列表为一个整型变量
    std::vector<llvm::Type *> printIntArgTypes;
    printIntArgTypes.push_back(llvm::Type::getInt32Ty(Context));

    // 创建 printInt() 的函数类型，返回类型为 void
    llvm::FunctionType *printIntFuncType =
            llvm::FunctionType::get(llvm::Type::getVoidTy(Context), printIntArgTypes, false);

    // 创建 printInt() 函数
    llvm::Function *printIntFunc =
            llvm::Function::Create(printIntFuncType, llvm::Function::ExternalLinkage, llvm::Twine("printInt"), context->module);

    // 为 printInt() 创建基本块
    llvm::BasicBlock *basicBlock = llvm::BasicBlock::Create(Context, "printInt_entry", printIntFunc, 0);
    // 基本块入栈
    context->PushBasicBlock(basicBlock);

    // 创建 printf() 的输出格式参数
    const std::string printIntFormat = "%d\n";
    llvm::Constant *printIntFormatStr = llvm::ConstantDataArray::getString(Context, printIntFormat);
    // 获取输出格式参数的 LLVM 类型
    llvm::ArrayType *printIntFormatType =
            llvm::ArrayType::get(llvm::IntegerType::get(Context, 8), printIntFormat.length() + 1);
    // 创建一个全局变量，存储输出格式参数
    llvm::GlobalVariable *printIntFormatVar =
            new llvm::GlobalVariable(*context->module, printIntFormatType, true, llvm::GlobalValue::PrivateLinkage, printIntFormatStr, ".printIntFormatStr");

    // 获取 printInt() 函数的整型参数，作为 printf() 的参数，并命名为 "intToPrint"
    llvm::Value *intToPrint = printIntFunc->arg_begin();
    intToPrint->setName("intToPrint");

    // 创建 printf() 函数的实参列表
    std::vector<llvm::Value *> printfArgs({ printIntFormatVar, intToPrint });

    // 发起对 printf() 的调用，以 printfArgs 作为实参列表
    llvm::CallInst::Create(printfFunc, llvm::ArrayRef(printfArgs), "", basicBlock);

    // 创建返回指令，从 printInt() 返回
    llvm::ReturnInst::Create(Context, basicBlock);

    // 基本块出栈
    context->PopBasicBlock();
}

/**
 * @brief 创建一个打印字符串常量类型的函数
 * @param context 上下文
 * @param printfFunc 可调用的 printf 函数
 */
void CreatePrintConstStringFunc(CodeGenContext *context, llvm::Function *printfFunc) {
    // print() 的形参列表为一个整型变量
    std::vector<llvm::Type *> printConstStringArgTypes;
    printConstStringArgTypes.push_back(llvm::Type::getInt8PtrTy(Context));

    // 创建 printConstString() 的函数类型，返回类型为 void
    llvm::FunctionType *printConstStringFuncType =
            llvm::FunctionType::get(llvm::Type::getVoidTy(Context), printConstStringArgTypes, false);

    // 创建 printConstString() 函数
    llvm::Function *printConstStringFunc =
            llvm::Function::Create(printConstStringFuncType, llvm::Function::ExternalLinkage, llvm::Twine("printConstString"), context->module);

    // 为 printConstString() 创建基本块
    llvm::BasicBlock *basicBlock = llvm::BasicBlock::Create(Context, "printConstString_entry", printConstStringFunc, 0);
    // 基本块入栈
    context->PushBasicBlock(basicBlock);

    // 创建 printf() 的输出格式参数
    const std::string printConstStringFormat = "%s\n";
    llvm::Constant *printConstStringFormatStr = llvm::ConstantDataArray::getString(Context, printConstStringFormat);
    // 获取输出格式参数的 LLVM 类型
    llvm::ArrayType *printConstStringFormatType =
            llvm::ArrayType::get(llvm::IntegerType::get(Context, 8), printConstStringFormat.length() + 1);
    // 创建一个全局变量，存储输出格式参数
    llvm::GlobalVariable *printConstStringFormatVar =
            new llvm::GlobalVariable(*context->module, printConstStringFormatType, true, llvm::GlobalValue::PrivateLinkage, printConstStringFormatStr, ".printConstStringFormatStr");

    // 获取 printConstString() 函数的字符串常量参数，作为 printf() 的参数，并命名为 "constStringToPrint"
    llvm::Value *constStringToPrint = printConstStringFunc->arg_begin();
    constStringToPrint->setName("constStringToPrint");

    // 创建 printf() 函数的实参列表
    std::vector<llvm::Value *> printfArgs({ printConstStringFormatVar, constStringToPrint });

    // 发起对 printf() 的调用，以 printfArgs 作为实参列表
    llvm::CallInst::Create(printfFunc, llvm::ArrayRef(printfArgs), "", basicBlock);

    // 创建返回指令，从 printConstString() 返回
    llvm::ReturnInst::Create(Context, basicBlock);

    // 基本块出栈
    context->PopBasicBlock();
}

/**
 * @brief 创建输入输出相关的内置函数
 * @param context 上下文
 */
void CreateIOFunc(CodeGenContext *context) {
    llvm::Function *printfFunc = CreatePrintfFunc(context);

    CreatePrintBoolFunc(context, printfFunc);
    CreatePrintCharFunc(context, printfFunc);
    CreatePrintIntFunc(context, printfFunc);
    CreatePrintConstStringFunc(context, printfFunc);
}
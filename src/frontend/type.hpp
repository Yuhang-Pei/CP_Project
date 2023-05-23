//
// Created by Pei Yuhang on 2023/5/23.
//

#ifndef CP_PROJECT_TYPE_HPP
#define CP_PROJECT_TYPE_HPP

#include "codegen.h"

llvm::Value *CastToBool(llvm::Value *val) {
    if (val->getType() == Builder.getInt1Ty())
        return val;
    if (val->getType()->isIntegerTy())
        return Builder.CreateICmpNE(val, llvm::ConstantInt::get(llvm::Type::getInt32Ty(Context), 0));

    /// TODO: 需要完成其他类型转为布尔类型

    throw std::logic_error("Cannot cast to bool");
}

llvm::Type *GetPtrElementType(llvm::Value *ptr) {
    if (!ptr->getType()->isPointerTy())
        throw std::logic_error("Should pass a pointer to get the element type");

    return static_cast<llvm::AllocaInst *>(ptr)->getAllocatedType();
}

#endif //CP_PROJECT_TYPE_HPP

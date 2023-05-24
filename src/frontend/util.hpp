//
// Created by Pei Yuhang on 2023/5/24.
//

#ifndef CP_PROJECT_UTIL_HPP
#define CP_PROJECT_UTIL_HPP

#include "codegen.h"

void InsertFuncBasicBlockList(llvm::Function *func, llvm::BasicBlock *BB) {
#if LLVM_VERSION_MAJOR >= 16
    func->insert(func->end(), BB);
#else
    func->getBasicBlockList().push_back(BB);
#endif
}

#endif //CP_PROJECT_UTIL_HPP

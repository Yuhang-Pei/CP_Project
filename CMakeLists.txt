cmake_minimum_required(VERSION 3.25)
project(CP_Project)

set(CMAKE_CXX_STANDARD 17)

# 查找 LLVM 库
find_package(LLVM REQUIRED CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

# 设置包含目录
include_directories(${LLVM_INCLUDE_DIRS})

# 添加 LLVM 定义
add_definitions(${LLVM_DEFINITIONS})

# 添加可执行文件
add_executable(CP_Project src/compiler.cpp)

# 链接 LLVM 库
target_link_libraries(CP_Project LLVM)

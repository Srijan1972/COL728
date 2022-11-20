#ifndef __LLVM_HH__
#define __LLVM_HH__
#include <llvm-c/Analysis.h>
#include <llvm-c/BitWriter.h>
#include <llvm-c/Core.h>
#include <llvm-c/ExecutionEngine.h>
#include <llvm-c/Initialization.h>
#include <llvm-c/Target.h>
#include <llvm-c/Transforms/Scalar.h>

#define VALUE LLVMValueRef
#define BLOCK LLVMBasicBlockRef
#define TYPE LLVMTypeRef
#define PASS LLVMPassManagerRef
#endif
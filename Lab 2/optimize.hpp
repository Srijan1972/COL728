#ifndef __OPTIMIZE_HH__
#define __OPTIMIZE_HH__
#include "node.hpp"
#include "llvm.hpp"
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <stack>
#include <iostream>
void optimize(LLVMModuleRef mod,LLVMBuilderRef builder);
#endif
#ifndef __CODEGEN_HH__
#define __CODEGEN_HH__
#include <assert.h>
#include <stack>
#include <string>
#include <unordered_map>
#include <iostream>
#include <fstream>
#include "llvm.hpp"
#include "node.hpp"
#include "optimize.hpp"
void codegen(Node* AST);
#endif
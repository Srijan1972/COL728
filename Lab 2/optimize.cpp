#include "optimize.hpp"


void copyPropagate(BLOCK blk,LLVMBuilderRef builder,std::unordered_map<std::string,VALUE>& propagate, std::unordered_map<VALUE, VALUE>& loaded, std::unordered_map<BLOCK,std::vector<VALUE>>& dead,std::unordered_set<BLOCK>& visited){
    VALUE end = LLVMGetBasicBlockTerminator(blk);
    if(end == NULL) return;
    VALUE curr = LLVMGetFirstInstruction(blk);
    while(curr!=NULL){
        int op = LLVMGetInstructionOpcode(curr);
        switch(op){
            case LLVMCall:
            {
                int n = LLVMGetNumOperands(curr);
                for(int i=0;i<n;i++){
                    VALUE oper = LLVMGetOperand(curr,i);
                    std::string o(LLVMGetValueName(oper));
                    if(propagate.find(o)!=propagate.end()) LLVMSetOperand(curr,i,propagate[o]);
                    if(loaded.find(oper)!=loaded.end()) loaded.erase(oper);
                }
            }break;
            case LLVMLoad:
            {
                VALUE addr = LLVMGetOperand(curr,0);
                std::string addrName(LLVMGetValueName(addr));
                std::string inst(LLVMGetValueName(curr));
                propagate[inst] = addr;
                if(loaded.find(addr)==loaded.end()) loaded[addr] = curr;
                else dead[blk].push_back(curr);
            }break;
            default:
            {
                int n = LLVMGetNumOperands(curr);
                for(int i=0;i<n;i++){
                    VALUE oper = LLVMGetOperand(curr,i);
                    std::string o(LLVMGetValueName(oper));
                    if(propagate.find(o)!=propagate.end()) LLVMSetOperand(curr,i,loaded[propagate[o]]);
                }
            }break;
        }
        curr = LLVMGetNextInstruction(curr);
    }
    // Run for all successors of the block i. e. DFS.
    int n = LLVMGetNumSuccessors(end);
    for(int i=0;i<n;i++){
        BLOCK s = LLVMGetSuccessor(end,i);
        // Already visited
        if(visited.find(s)!=visited.end()) continue;
        visited.insert(s);
        copyPropagate(s,builder,propagate,loaded,dead,visited);
    }
}

void completeCopyPropagate(VALUE func,LLVMBuilderRef builder){
    BLOCK curr = LLVMGetEntryBasicBlock(func);
    std::unordered_set<BLOCK> traverse;
    std::unordered_map<std::string,VALUE> propagate;
    std::unordered_map<VALUE,VALUE> loaded;
    std::unordered_map<BLOCK,std::vector<VALUE>> dead;
    std::unordered_set<BLOCK> visited;
    if(curr!=NULL) copyPropagate(curr,builder,propagate,loaded,dead,visited);
    for(auto it = dead.begin();it!=dead.end();it++){
        for(auto d:it->second) LLVMInstructionEraseFromParent(d);
    }
}

void constantFold(BLOCK blk,LLVMBuilderRef builder,std::unordered_set<BLOCK>& visited){
    VALUE end = LLVMGetBasicBlockTerminator(blk);
    if(end == NULL) return;
    VALUE curr = LLVMGetFirstInstruction(blk);
    while(curr!=NULL){
        int op = LLVMGetInstructionOpcode(curr);
        switch(op){
            case LLVMAdd:
            case LLVMFAdd:
            case LLVMSub:
            case LLVMFSub:
            case LLVMMul:
            case LLVMFMul:
            case LLVMSDiv:
            case LLVMFDiv:
            case LLVMSRem:
            case LLVMShl:
            case LLVMAShr:
            case LLVMAnd:
            case LLVMOr:
            case LLVMXor:
            {
                VALUE left = LLVMGetOperand(curr,0);
                VALUE right = LLVMGetOperand(curr,1);
                if(LLVMIsConstant(left) && LLVMIsConstant(right)){
                    VALUE inst = LLVMBuildBinOp(builder,LLVMOpcode(op),left,right,"");
                    LLVMReplaceAllUsesWith(curr,inst);
                }
            }
            default:
                break;
        }
        curr = LLVMGetNextInstruction(curr);
    }
    // Run for all successors of the block i. e. DFS.
    int n = LLVMGetNumSuccessors(end);
    for(int i=0;i<n;i++){
        BLOCK s = LLVMGetSuccessor(end,i);
        // Already visited
        if(visited.find(s)!=visited.end()) continue;
        visited.insert(s);
        // Back edge
        constantFold(s,builder,visited);
    }
}

void completeConstantFold(VALUE func,LLVMBuilderRef builder){
    BLOCK curr = LLVMGetEntryBasicBlock(func);
    std::unordered_set<BLOCK> visited;
    if(curr!=NULL) constantFold(curr,builder,visited);
}

void optimize(LLVMModuleRef mod,LLVMBuilderRef builder){
    PASS manager = LLVMCreateFunctionPassManagerForModule(mod);
    LLVMAddPromoteMemoryToRegisterPass(manager);
    LLVMInitializeFunctionPassManager(manager);
    VALUE curr = LLVMGetFirstFunction(mod);
    while(curr!=NULL){
        LLVMRunFunctionPassManager(manager,curr);
        completeCopyPropagate(curr,builder);
        completeConstantFold(curr,builder);
        curr = LLVMGetNextFunction(curr);
    }
    LLVMFinalizeFunctionPassManager(manager);
}
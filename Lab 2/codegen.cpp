#include "codegen.hpp"

LLVMModuleRef code_module;
std::stack<std::unordered_map<std::string,VALUE>> varTable;
std::stack<std::unordered_map<std::string,VALUE>> arrTable;
std::unordered_map<std::string,VALUE> funcTable;
std::stack<std::unordered_map<std::string,VALUE>> argTable;
LLVMBuilderRef builder;
LLVMContextRef context;

VALUE searchTable(std::string var,std::stack<std::unordered_map<std::string,VALUE>> &table){
    if(table.empty()) return NULL;
    std::unordered_map<std::string,VALUE> topScope = table.top();
    if(topScope.find(var)!=topScope.end()) return topScope[var];
    else{
        table.pop();
        VALUE ret = searchTable(var,table);
        table.push(topScope);
        return ret;
    }
}

TYPE toLLVMType(std::string t,LLVMContextRef c){
    if(t == "VOID") return LLVMVoidTypeInContext(c);
    else if(t == "CHAR") return LLVMInt8TypeInContext(c);
    if(t == "INT") return LLVMInt32TypeInContext(c);
    else if(t == "FLOAT") return LLVMFloatTypeInContext(c);
    else if(t == "BOOL") return LLVMInt1TypeInContext(c);
    else if(t.find("*")!=std::string::npos){
        t.pop_back();
        TYPE deref = toLLVMType(t,c);
        return LLVMPointerType(deref,0);
    }
    else return NULL;
}

VALUE loadVal(Node* node,VALUE curr){
    // Load value if variable/dereference
    if(node->which == "ID" && node->nodes.size() == 0){
        std::string kind = ((ID*)node)->kind;
        VALUE load = searchTable(kind,arrTable);
        if(load == NULL){
            load = searchTable(kind,varTable);
            if(load!=NULL) return LLVMBuildLoad(builder,load,"load_var");
        }
        else{
            VALUE arr_base = LLVMBuildStructGEP(builder,load,0,"arr_base");
            VALUE idx = LLVMConstInt(LLVMInt32TypeInContext(context),0,false);
            return LLVMBuildGEP(builder,arr_base,&idx,0,"arr_begin");
        }
    }
    else if(node->which == "ID" && node->nodes[0]->which == "[]"){
        return LLVMBuildLoad(builder,curr,"array_deref_");
    }
    else if(node->which == "DE") return LLVMBuildLoad(builder,curr,"ptr_deref");
    return curr;
}

VALUE binExp(Node* left,Node* right,LLVMOpcode op){
    VALUE lval = left->codegen();
    lval = loadVal(left,lval);
    VALUE rval = right->codegen();
    rval = loadVal(right,rval);

    if(LLVMGetTypeKind(LLVMTypeOf(lval)) == LLVMFloatTypeKind && LLVMGetTypeKind(LLVMTypeOf(rval)) == LLVMFloatTypeKind){
        return LLVMBuildBinOp(builder,(LLVMOpcode)(op+1),lval,rval,"float_op");
    }
    else if(LLVMGetTypeKind(LLVMTypeOf(lval)) == LLVMFloatTypeKind){
        VALUE temp = LLVMBuildSIToFP(builder,rval,LLVMFloatType(),"int_to_float");
        return LLVMBuildBinOp(builder,(LLVMOpcode)(op+1),lval,temp,"float_op");
    }
    else if(LLVMGetTypeKind(LLVMTypeOf(rval)) == LLVMFloatTypeKind){
        VALUE temp = LLVMBuildSIToFP(builder,lval,LLVMFloatType(),"int_to_float");
        return LLVMBuildBinOp(builder,(LLVMOpcode)(op+1),temp,rval,"float_op");
    }
    else if(LLVMGetTypeKind(LLVMTypeOf(lval)) == LLVMPointerTypeKind && LLVMGetTypeKind(LLVMTypeOf(rval)) == LLVMIntegerTypeKind){
        return LLVMBuildGEP(builder,lval,&rval,1,"ptr_arith");
    }
    else if(LLVMGetTypeKind(LLVMTypeOf(rval)) == LLVMPointerTypeKind && LLVMGetTypeKind(LLVMTypeOf(lval)) == LLVMIntegerTypeKind){
        return LLVMBuildGEP(builder,rval,&lval,1,"ptr_arith");
    }
    else return LLVMBuildBinOp(builder,op,lval,rval,"int_op");
}

VALUE compExp(Node* left,Node* right,LLVMIntPredicate op){
    VALUE lval = left->codegen();
    lval = loadVal(left,lval);
    VALUE rval = right->codegen();
    rval = loadVal(right,rval);

    if(LLVMGetTypeKind(LLVMTypeOf(lval)) == LLVMFloatTypeKind && LLVMGetTypeKind(LLVMTypeOf(rval)) == LLVMFloatTypeKind){
        return LLVMBuildFCmp(builder,(LLVMRealPredicate)(op),lval,rval,"float_cmp");
    }
    else if(LLVMGetTypeKind(LLVMTypeOf(lval)) == LLVMFloatTypeKind){
        VALUE temp = LLVMBuildSIToFP(builder,rval,LLVMFloatType(),"int_to_float");
        return LLVMBuildFCmp(builder,(LLVMRealPredicate)(op),lval,temp,"float_cmp");
    }
    else if(LLVMGetTypeKind(LLVMTypeOf(rval)) == LLVMFloatTypeKind){
        VALUE temp = LLVMBuildSIToFP(builder,lval,LLVMFloatType(),"int_to_float");
        return LLVMBuildFCmp(builder,(LLVMRealPredicate)(op),temp,rval,"float_cmp");
    }
    else return LLVMBuildICmp(builder,op,lval,rval,"int_cmp");
}

VALUE globalDeclaration(std::string id,TYPE t){
    VALUE alloc = LLVMAddGlobal(code_module,t,id.c_str());
    LLVMSetLinkage(alloc,LLVMCommonLinkage);
    LLVMSetGlobalConstant(alloc,false);
    return alloc;
}

VALUE initToZero(TYPE t,TYPE base=NULL,int len=0){
    LLVMTypeKind kind = LLVMGetTypeKind(t);
    if(kind == LLVMIntegerTypeKind) return LLVMConstInt(LLVMInt32TypeInContext(context),0,false);
    if(kind == LLVMFloatTypeKind) return LLVMConstReal(LLVMFloatTypeInContext(context),0.0);
    if(kind == LLVMPointerTypeKind) return LLVMConstPointerNull(t);
    if(kind == LLVMArrayTypeKind){
        VALUE zeros[len];
        for(int i = 0; i < len ; i++) zeros[i] = initToZero(t,base);
        return LLVMConstArray(t,zeros,len); 
    }
    return NULL;
}

VALUE Node::codegen(){
    if(which == "BEGIN"){
        for(Node* n:nodes){
            if(n->which == "DEC") ((Dec*)(n))->codegen(true);
            else n->codegen();
        }
        return NULL;
    }
    else if(which == "FUN"){
        std::unordered_map<std::string,VALUE> func_vars;
        TYPE ret = toLLVMType(nodes[0]->which,context);
        VALUE func = ((Function*)nodes[1])->codegen(true,ret);
        VALUE par = LLVMGetFirstParam(func);
        for(int i = 0; i < nodes[1]->nodes[1]->nodes.size(); i++){
            std::string parName = ((ID*)nodes[1]->nodes[1]->nodes[i]->nodes[1])->kind;
            func_vars[parName] = par;
            par = LLVMGetNextParam(par);
        }
        BLOCK func_entry = LLVMAppendBasicBlock(func,"entry");
        LLVMPositionBuilderAtEnd(builder,func_entry);
        for(auto kv:func_vars){
            VALUE temp = LLVMBuildAlloca(builder,LLVMTypeOf(kv.second),kv.first.c_str());
            LLVMBuildStore(builder,kv.second,temp);
            func_vars[kv.first] = temp;
        }
        varTable.push(func_vars);
        ((FunctionBlock*)nodes[2])->codegen(func,ret);
        varTable.pop();
        return NULL;
    }
    else if(which == "ASSIGN"){
        VALUE var = nodes[0]->codegen();
        VALUE toAssign = nodes[1]->codegen();
        toAssign = loadVal(nodes[1],toAssign);
        return LLVMBuildStore(builder,toAssign,var);
    }
    else if(which == "PLUS") return binExp(nodes[0],nodes[1],LLVMAdd);
    else if(which == "MINUS") return binExp(nodes[0],nodes[1],LLVMSub);
    else if(which == "TIMES") return binExp(nodes[0],nodes[1],LLVMMul);
    else if(which == "DIV") return binExp(nodes[0],nodes[1],LLVMSDiv);
    else if(which == "MOD") return binExp(nodes[0],nodes[1],LLVMSRem);
    else if(which == "LEFT") return binExp(nodes[0],nodes[1],LLVMShl);
    else if(which == "RIGHT") return binExp(nodes[0],nodes[1],LLVMAShr);
    else if(which == "PLUS") return binExp(nodes[0],nodes[1],LLVMAdd);
    else if(which == "BitwiseOr" || which == "LogicalOr") return binExp(nodes[0],nodes[1],LLVMOr);
    else if(which == "BitwiseAnd" || which == "LogicalAnd") return binExp(nodes[0],nodes[1],LLVMAnd);
    else if(which == "XOR") return binExp(nodes[0],nodes[1],LLVMXor);
    else if(which == "EQ") return compExp(nodes[0],nodes[1],LLVMIntEQ);
    else if(which == "NEQ") return compExp(nodes[0],nodes[1],LLVMIntNE);
    else if(which == "GT") return compExp(nodes[0],nodes[1],LLVMIntSGT);
    else if(which == "GEQ") return compExp(nodes[0],nodes[1],LLVMIntSGE);
    else if(which == "LEQ") return compExp(nodes[0],nodes[1],LLVMIntSLE);
    else if(which == "LT") return compExp(nodes[0],nodes[1],LLVMIntSLT);
    else if(which == "CONDITION") return nodes[0]->codegen();
    else if(which == "RETURN"){
        VALUE ret = nodes[0]->codegen();
        return loadVal(nodes[0],ret);
    }
    else if(which == "REF") return nodes[0]->codegen();
    else if(which == "DE") return loadVal(nodes[0],nodes[0]->codegen());
    else if(which == "+"){
        Constant* n = new Constant(1);
        return binExp(n,nodes[0],LLVMMul);
    }
    else if(which == "-"){
        Constant* n = new Constant(-1);
        return binExp(n,nodes[0],LLVMMul);
    }
    else if(which == "NOT") return LLVMBuildNot(builder,nodes[0]->codegen(),"negate");
    else if(which == "++"){
        Constant* n = new Constant(1);
        VALUE var = nodes[0]->codegen();
        VALUE inc = LLVMBuildBinOp(builder,LLVMAdd,loadVal(nodes[0],var),n->codegen(),"inc");
        return LLVMBuildStore(builder,inc,var);
    }
    else if(which == "--"){
        Constant* n = new Constant(1);
        VALUE var = nodes[0]->codegen();
        VALUE dec = LLVMBuildBinOp(builder,LLVMSub,loadVal(nodes[0],var),n->codegen(),"dec"); 
        return LLVMBuildStore(builder,dec,var);
    }
    return NULL;
}

VALUE Dec::codegen(bool global){
    std::string which_dec = nodes[0]->which;
    return ((InitDec*)nodes[1])->codegen(global,toLLVMType(which_dec,context));
}

VALUE InitDec::codegen(bool global,TYPE t){
    for(Node* n:nodes){
        if(n->which == "VARIABLE"){
            ((Var*)(n))->codegen(global,t);
        }
        else if(n->which == "ARRAY"){
            ((Array*)(n))->codegen(global,t);
        }
        else if(n->which == "PTR"){
            ((Pointer*)(n))->codegen(global,t);
        }
        else if(n->which == "FUNCTION"){
            ((Function*)(n))->codegen(global,t);
        } 
    }
    return NULL;
}

VALUE Var::codegen(bool global,TYPE t){
    std::string kind = ((ID*)(nodes[0]))->kind;
    if(global){
        VALUE alloc = globalDeclaration(kind,t);
        LLVMSetInitializer(alloc,initToZero(t));
        varTable.top()[kind] = alloc;
        return alloc;
    }
    else{
        VALUE alloc = LLVMBuildAlloca(builder,t,kind.c_str());
        varTable.top()[kind] = alloc;
        return alloc;
    }
}

VALUE Pointer::codegen(bool global,TYPE t){
    std::string deref_type = nodes[0]->which;
    VALUE ret;
    TYPE de = LLVMPointerType(t,0);
    if(deref_type == "PTR") ret = ((Pointer*)nodes[0])->codegen(global,de);
    else if(deref_type == "VARIABLE") ret = ((Var*)nodes[0])->codegen(global,de);
    else return ((Function*)nodes[0])->codegen(global,de);
    LLVMSetInitializer(ret,initToZero(t));
    LLVMSetAlignment(ret,8);
    return ret;
}

VALUE Array::codegen(bool global,TYPE t){
    std::string kind = ((ID*)nodes[0])->kind;
    int size = ((Constant*)nodes[1])->num;
    TYPE nt = LLVMArrayType(t,size);
    VALUE alloc;
    if(global){
        alloc = globalDeclaration(kind,nt);
        LLVMSetInitializer(alloc,initToZero(nt,t,size));
    }
    else{
        BLOCK curr = LLVMGetInsertBlock(builder);
        VALUE parent = LLVMGetBasicBlockParent(curr);
        alloc = LLVMBuildArrayAlloca(builder,nt,NULL,kind.c_str());
    }
    varTable.top()[kind] = alloc;
    arrTable.top()[kind] = alloc;
    return alloc;
}

VALUE Function::codegen(bool global,TYPE t){
    std::string kind = ((ID*)nodes[0])->kind;
    std::string voidNot = nodes[1]->which;
    if(funcTable.find(kind)!=funcTable.end()) return funcTable[kind];
    if(voidNot == "VOID"){
        TYPE args[] = {};
        TYPE ret = LLVMFunctionType(t,args,0,false);
        VALUE dec = LLVMAddFunction(code_module,kind.c_str(),ret);
        funcTable[kind] = dec;
        return dec;
    }
    std::vector<TYPE> args;
    bool isNumVar = ((Param*)nodes[1])->numVariable;
    for(Node* n:nodes[1]->nodes) args.push_back(toLLVMType(n->nodes[0]->which,context));
    TYPE ret = LLVMFunctionType(t,args.data(),args.size(),isNumVar);
    VALUE dec = LLVMAddFunction(code_module,kind.c_str(),ret);
    funcTable[kind] = dec;
    return dec;
}

// TODO: Statements do not seem correct
VALUE Branch::codegen(TYPE t){
    BLOCK curr = LLVMGetInsertBlock(builder);
    VALUE parent = LLVMGetBasicBlockParent(curr);
    if(which == "DO-WHILE"){
        BLOCK body = LLVMAppendBasicBlock(parent,"loop_body");
        BLOCK cond = LLVMAppendBasicBlock(parent,"loop_cond");
        BLOCK end = LLVMAppendBasicBlock(parent,"end_loop");
        LLVMPositionBuilderAtEnd(builder,curr);
        ((Conditional*)nodes[0])->codegen(t,cond);
        LLVMPositionBuilderAtEnd(builder,body);
        LLVMBuildCondBr(builder,nodes[1]->codegen(),body,end);
        LLVMPositionBuilderAtEnd(builder,end);
    }
    else if(which == "FOR"){
        BLOCK cond = LLVMAppendBasicBlock(parent,"loop_cond");
        BLOCK body = LLVMAppendBasicBlock(parent,"loop_body");
        BLOCK inc = LLVMAppendBasicBlock(parent,"inc");
        BLOCK end = LLVMAppendBasicBlock(parent,"end_loop");
        LLVMPositionBuilderAtEnd(builder,curr);
        nodes[0]->codegen();
        LLVMBuildBr(builder,cond);
        LLVMPositionBuilderAtEnd(builder,cond);
        LLVMBuildCondBr(builder,nodes[1]->codegen(),body,end);
        LLVMPositionBuilderAtEnd(builder,body);
        ((Conditional*)nodes[3])->codegen(t,inc);
        LLVMPositionBuilderAtEnd(builder,inc);
        nodes[2]->codegen();
        LLVMBuildBr(builder,cond);
        LLVMPositionBuilderAtEnd(builder,end);
    }
    else if(which == "IT"){
        BLOCK body = LLVMAppendBasicBlock(parent,"if_body");
        BLOCK end = LLVMAppendBasicBlock(parent,"end_if");
        LLVMPositionBuilderAtEnd(builder,curr);
        LLVMBuildCondBr(builder,nodes[0]->codegen(),body,end);
        LLVMPositionBuilderAtEnd(builder,body);
        ((Conditional*)nodes[1])->codegen(t,end);
        LLVMPositionBuilderAtEnd(builder,end);
    }
    else if(which == "ITE"){
        BLOCK true_body = LLVMAppendBasicBlock(parent,"true_body");
        BLOCK false_body = LLVMAppendBasicBlock(parent,"false_body");
        BLOCK end = LLVMAppendBasicBlock(parent,"end_if");
        LLVMPositionBuilderAtEnd(builder,curr);
        LLVMBuildCondBr(builder,nodes[0]->codegen(),true_body,false_body);
        LLVMPositionBuilderAtEnd(builder,true_body);
        ((Conditional*)nodes[1])->codegen(t,end);
        LLVMPositionBuilderAtEnd(builder,false_body);
        ((Conditional*)nodes[2])->codegen(t,end);
        LLVMPositionBuilderAtEnd(builder,end);
    }
    else if(which == "WHILE"){
        BLOCK cond = LLVMAppendBasicBlock(parent,"loop_cond");
        BLOCK body = LLVMAppendBasicBlock(parent,"loop_body");
        BLOCK end = LLVMAppendBasicBlock(parent,"end_loop");
        LLVMPositionBuilderAtEnd(builder,curr);
        LLVMBuildCondBr(builder,nodes[0]->codegen(),body,end);
        LLVMPositionBuilderAtEnd(builder,cond);
        ((Conditional*)nodes[1])->codegen(t,cond);
        LLVMPositionBuilderAtEnd(builder,end);
    }
    return NULL;
}

VALUE FunctionBlock::codegen(VALUE func,TYPE t){
    std::unordered_map<std::string,VALUE> vars;
    std::unordered_map<std::string,VALUE> arrs;
    varTable.push(vars);
    arrTable.push(arrs);
    for(auto n:nodes){
        if(n->which == "RETURN"){
            if(t == LLVMVoidType()) LLVMBuildRetVoid(builder);
            else LLVMBuildRet(builder,n->codegen());
        }
        else if(n->which == "DEC") ((Dec*)n)->codegen(false);
        else if(n->which == "DO-WHILE" || n->which == "FOR" || n->which == "IT" || n->which == "ITE" || n->which == "WHILE") ((Branch*)n)->codegen(t);
        else n->codegen();
    }
    varTable.pop();
    arrTable.pop();
    return NULL;
}

VALUE Conditional::codegen(TYPE t,BLOCK next){
    std::unordered_map<std::string,VALUE> vars;
    std::unordered_map<std::string,VALUE> arrs;
    varTable.push(vars);
    arrTable.push(arrs);
    for(auto n:nodes){
        if(n->which == "RETURN"){
            if(t == LLVMVoidType()) LLVMBuildRetVoid(builder);
            else LLVMBuildRet(builder,n->codegen());
        }
        else if(n->which == "DEC") ((Dec*)n)->codegen(false);
        else if(n->which == "DO-WHILE" || n->which == "FOR" || n->which == "IT" || n->which == "ITE" || n->which == "WHILE") ((Branch*)n)->codegen(t);
        else n->codegen();
    }
    LLVMBuildBr(builder,next);
    varTable.pop();
    arrTable.pop();
    return NULL;
}

VALUE Constant::codegen(){
    if(kind == "INT") return LLVMConstInt(LLVMInt32TypeInContext(context),num,false);
    else if(kind == "FLOAT") return LLVMConstReal(LLVMFloatTypeInContext(context),dec);
    else return LLVMBuildGlobalStringPtr(builder,lit.c_str(),"string_literal");
}

VALUE ID::codegen(){
    VALUE val = searchTable(kind,varTable);
    if(val!=NULL){
        if(nodes.size() == 0) return val;
        else{
            Node* idx = nodes[0]->nodes[0];
            VALUE i = idx->codegen();
            i = loadVal(idx,i);
            VALUE arr = searchTable(kind,arrTable);
            VALUE base,elem;
            if(arr!=NULL){
                base = LLVMBuildStructGEP(builder,val,0,"base_array");
                elem = LLVMBuildInBoundsGEP(builder,base,&i,1,"element");
            }
            else{
                base = LLVMBuildLoad(builder,val,"base");
                elem = LLVMBuildGEP(builder,base,&i,1,"load_elem");
            }
            return elem;
        }
    }
    // if(argTable.top().find(kind)!=argTable.top().end()) return argTable.top()[kind];
    if(funcTable.find(kind)!=funcTable.end()){
        val = funcTable[kind];
        if(nodes.size() == 0){
            VALUE* empty;
            return LLVMBuildCall(builder,val,empty,0,(kind+"_call").c_str());
        }
        std::vector<VALUE> codeArgs;
        for(auto n:nodes[0]->nodes){
            VALUE g = n->codegen();
            g = loadVal(n,g);
            codeArgs.push_back(g);
        }
        return LLVMBuildCall(builder,val,codeArgs.data(),codeArgs.size(),(kind+"_call").c_str());
    }
    return NULL;
}

void codegen(Node* AST){
    context = LLVMGetGlobalContext();
    builder = LLVMCreateBuilderInContext(context);
    std::unordered_map<std::string,VALUE> vars;
    std::unordered_map<std::string,VALUE> arrs;
    funcTable.clear();
    varTable.push(vars);
    arrTable.push(arrs);
    code_module = LLVMModuleCreateWithNameInContext("Code",context);
    AST->codegen();
    std::fstream f;
    f.open("code_gen.txt",std::ios::out);
    f << LLVMPrintModuleToString(code_module) << '\n';
    f.close();
    optimize(code_module,builder);
    f.open("code_opt.txt",std::ios::out);
    f << LLVMPrintModuleToString(code_module) << '\n';
    f.close();
    varTable.pop();
    arrTable.pop();
    funcTable.clear();
    LLVMDisposeBuilder(builder);
    LLVMContextDispose(context);
}
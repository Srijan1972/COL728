#include "semantics.hpp"

std::pair<bool,std::string> get_symbol(std::stack<Scope*>& scopes,std::string id){
    std::pair<bool,std::string> ret = std::make_pair(false,"");
    if(scopes.empty()) return ret;
    Scope* t = scopes.top();
    bool found = t->check_scope(id);
    if(found){
        ret.first = true;
        ret.second = t->get_type(id);
    }
    else{
        scopes.pop();
        ret = get_symbol(scopes,id);
        scopes.push(t);
    }
    return ret;
}

std::vector<std::string> getFunParams(Function* function,std::string curr){
    std::vector<std::string> ret;

    // All declared parameters
    for(auto node:function->nodes[1]->nodes) ret.push_back(node->nodes[0]->which);

    // Return type
    ret.push_back(curr);
    return ret;
}

bool checkFuncDecArgs(std::vector<std::string> func_params, std::vector<Node*> tree_params){
    // func_params also contains return type
    if(func_params.size() - 1 != tree_params.size()) return false;
    int n = tree_params.size();
    // Checking type same of all n params
    for(int i = 0; i < n; i++){
        if(func_params[i] != tree_params[i]->nodes[0]->which) return false;
    }
    return true;
}

bool checkFuncUseArgs(std::vector<std::string> func_params, std::vector<std::string> used_params){
    if(func_params.size() - 1 != used_params.size()) return false;
    int n = used_params.size();
    // Checking type same of all n params
    for(int i = 0; i < n; i++){
        if(func_params[i] != used_params[i]) return false;
    }
    return true;
}

bool checkDecBeforeUse(Node* node,std::string curr,std::stack<Scope*>& vars,std::stack<Scope*>& arrs, std::unordered_map<std::string, std::vector<std::string>>& funcs,  std::unordered_map<std::string, bool>& isNumVar){
    if(node == NULL) return false;
    std::string which = node->which;
    if(which == "DEC"){
        return checkDecBeforeUse(node->nodes[1],node->nodes[0]->which,vars,arrs,funcs,isNumVar);
    }
    else if(which == "VARIABLE"){
        vars.top()->add_symbol(((ID*)(node->nodes[0]))->kind,curr);
        return true;
    }
    else if(which == "PTR"){
        return checkDecBeforeUse(node->nodes[0],curr + "*",vars,arrs,funcs,isNumVar);
    }
    else if(which == "ARRAY"){
        arrs.top()->add_symbol(((ID*)(node->nodes[0]))->kind,curr,((Constant*)(node->nodes[1]))->num);
        return true;
    }
    else if(which == "FUNCTION"){
        std::string func_name = ((ID*)(node->nodes[0]))->kind;
        auto prev = funcs.find(func_name);
        if(prev == funcs.end()){
            // Not previously declared
            funcs[func_name] = getFunParams((Function*)node,curr);
            if(node->nodes[1]->which == "PAR-LIST" && ((Param*)(node->nodes[1]))->numVariable) isNumVar[func_name] = true;
            return true;
        }
        else{
            // Already declared
            bool check = checkFuncDecArgs(prev->second,node->nodes) && (prev->second.back() == curr);
            if(check || isNumVar[func_name]) return true;
            std::cout<<"Error: The function "<<func_name<<" has inconsistent arguments\n";
            return false;
        }
    }
    else if(which == "FUN"){
        // See if pointer is returned
        if(node->nodes[1]->which == "PTR"){
            auto temp = node->nodes[1];
            while(temp->which == "PTR"){
                node->nodes[0]->which += "*";
                temp = temp->nodes[0];
            }
            node->nodes[1] = temp;
        }
        bool dec = checkDecBeforeUse(node->nodes[1],node->nodes[0]->which,vars,arrs,funcs,isNumVar);
        Scope *new_scope = new Scope();
        Node* args = node->nodes[1]->nodes[1];
        for(auto arg:args->nodes) new_scope->add_symbol(((ID*)(arg->nodes[1]))->kind,arg->nodes[0]->which);
        vars.push(new_scope);
        bool check = dec && checkDecBeforeUse(node->nodes[2],curr,vars,arrs,funcs,isNumVar);
        vars.pop();
        return check;
    }
    else if(which == "WHILE"){
        return checkDecBeforeUse(node->nodes[1],curr,vars,arrs,funcs,isNumVar);
    }
    else if(which == "DO-WHILE"){
        return checkDecBeforeUse(node->nodes[0],curr,vars,arrs,funcs,isNumVar);
    }
    else if(which == "ID"){
        std::string kind = ((ID*)(node))->kind;
        auto var = get_symbol(vars,kind);
        if(!var.first){
            var = get_symbol(arrs,kind);
            if(var.first){
                bool c = true;
                if(node->nodes.size()){
                    c = checkDecBeforeUse(node->nodes[0]->nodes[0],curr,vars,arrs,funcs,isNumVar);
                }
                return c;
            }

            auto it = funcs.find(kind);
            if(it == funcs.end()){
                std::cout<<"Error: The function "<<kind<<" has not been declared\n";
                return false;
            }
            else{
                if(it->second.size() - 1 == node->nodes[0]->nodes.size()){
                    return checkDecBeforeUse(node->nodes[0],curr,vars,arrs,funcs,isNumVar);
                }
                if(isNumVar[kind]) return true;
                std::cout<<"Error: The function"<<kind<<"has been called with an invalid number of arguments\n";
                return false;
            }
        }
        return var.first;
    }
    else if(which == "Constant"){
        return true;
    }
    else{
        if(which == "BEGIN" || which == "BLOCK"){
            Scope *new_scope = new Scope();
            Scope *arr_scope = new Scope();
            vars.push(new_scope);
            arrs.push(arr_scope);
        }
        bool check = true;
        for(auto n:node->nodes){
            check &= checkDecBeforeUse(n,curr,vars,arrs,funcs,isNumVar);
        }
        if(which == "BLOCK"){
            vars.pop();
            arrs.pop();
        }
        return check;
    }
}

std::string checkType(Node* node,std::string curr,std::stack<Scope*>& vars,std::stack<Scope*>& arrs,std::unordered_map<std::string,std::vector<std::string>>& funcs,std::unordered_map<std::string,bool>& isNumVar){
    if(node == NULL) return "";
    std::string which = node->which;
    if(which == "DEC"){
        return checkType(node->nodes[1],node->nodes[0]->which,vars,arrs,funcs,isNumVar);
    }
    else if(which == "VARIABLE"){
        vars.top()->add_symbol(((ID*)(node->nodes[0]))->kind,curr);
        return "VOID";
    }
    else if(which == "PTR"){
        return checkType(node->nodes[0],curr + "*",vars,arrs,funcs,isNumVar);
    }
    else if(which == "ARRAY"){
        arrs.top()->add_symbol(((ID*)(node->nodes[0]))->kind,curr,((Constant*)(node->nodes[1]))->num);
        return "VOID";
    }
    else if(which == "ID"){
        std::string kind = ((ID*)(node))->kind;
        auto var = get_symbol(vars,kind);
        if(!var.first){
            var = get_symbol(arrs,kind);
            if(var.first){
                if(node->nodes.size()>0){
                    std::string idx = checkType(node->nodes[0]->nodes[0],curr,vars,arrs,funcs,isNumVar);
                    if(idx == "INT") return var.second;
                    else{
                        std::cout<<"Error: An array, "<<kind<<" can only be indexed with integers\n";
                        return "";
                    }
                }
                else{
                    return var.second;
                }
                std::cout<<"Error: Array "<<kind<<" can only be read through index\n";
                    return "";
            }
            auto it = funcs.find(kind);
            // Function will be declared
            std::vector<std::string> used_params;
            for(auto n:node->nodes[0]->nodes){
                std::string parType = checkType(n,curr,vars,arrs,funcs,isNumVar);
                if(n->which == "ID" && n->nodes.size() == 0){
                    auto f = get_symbol(arrs,((ID*)(n))->kind);
                    if(f.first) parType += "*";
                }
                used_params.push_back(parType);
            }
            bool correct = checkFuncUseArgs(it->second,used_params);
            if(correct || isNumVar[kind]) return it->second.back();
            else{
                std::cout<<"Function "<<kind<<" has been called with incorrectly typed arguments\n";
                return "";
            }
        }
        else{
            std::string temp = var.second;
            if(node->nodes.size() && node->nodes[0]->which == "[]") temp.pop_back();
            return temp;
        }
    }
    else if(which == "FUNCTION"){
        return "VOID";
    }
    else if(which == "FUN"){
        std::string ret_type = node->nodes[0]->which;
        Scope *new_scope = new Scope();
        Node* args = node->nodes[1]->nodes[1];
        for(auto arg:args->nodes) new_scope->add_symbol(((ID*)(arg->nodes[1]))->kind,arg->nodes[0]->which);
        vars.push(new_scope);
        std::string ret = checkType(node->nodes[2],ret_type,vars,arrs,funcs,isNumVar);
        vars.pop();
        if(ret_type == "VOID" || ret == "RETURN") return "VOID";
        std::string func = ((ID*)node->nodes[1]->nodes[0])->kind;
        std::cout<<"Error: Incorrect return type of function "<<func<<"\n";
        return "";
    }
    else if(which == "RETURN"){
        if(node->nodes.size()) return checkType(node->nodes[0],curr,vars,arrs,funcs,isNumVar);
        return "VOID";
    }
    else if(which == "Constant"){
        return ((Constant*)(node))->kind;
    }
    else if(which == "ASSIGN"){
        std::string left = checkType(node->nodes[0],curr,vars,arrs,funcs,isNumVar);
        std::string right = checkType(node->nodes[1],curr,vars,arrs,funcs,isNumVar);
        if(left == right) return "VOID";
        else if(left == "BOOL" && right == "INT") return "VOID";
        else if(left == "INT" && right == "BOOL") return "VOID";
        else if(left.find("INT")==0 && right == "INT") return "VOID";
        else{
            if(node->nodes[0]->which == "ID"){
                std::cout<<right<<'\n';
                std::cout<<"Error: Incorrect type assigned to variable "<<((ID*)(node->nodes[0]))->kind<<"\n";
            }
            return "";
        }
    }
    else if(which == "++" || which == "--"){
        std::string expr = checkType(node->nodes[0],curr,vars,arrs,funcs,isNumVar);
        return (expr == "INT")?"VOID":"";
    }
    else if(which == "PLUS" || which == "MINUS" || which == "TIMES" || which == "DIV"){
        std::string left = checkType(node->nodes[0],curr,vars,arrs,funcs,isNumVar);
        std::string right = checkType(node->nodes[1],curr,vars,arrs,funcs,isNumVar);
        if(left == "INT" && left == right) return "INT";
        else if(left == "INT" && right == "FLOAT") return "FLOAT";
        else if(left == "FLOAT" && right == "INT") return "FLOAT";
        else if(left == "FLOAT" && left == right) return "FLOAT";
        else if(left.find("*")!=std::string::npos && right == "INT" && (which == "PLUS" || which == "MINUS")) return "INT";
        else if(right.find("*")!=std::string::npos && left == "INT" && (which == "PLUS" || which == "MINUS")) return "INT";
        else return "";
    }
    else if(which == "MOD" || which == "LEFT" || which == "RIGHT"){
        std::string left = checkType(node->nodes[0],curr,vars,arrs,funcs,isNumVar);
        std::string right = checkType(node->nodes[1],curr,vars,arrs,funcs,isNumVar);
        if(left == "INT" && left == right) return "INT";
        else return "";
    }
    else if(which == "BitwiseAnd" || which == "BitwiseOr" || which == "XOR"){
        std::string left = checkType(node->nodes[0],curr,vars,arrs,funcs,isNumVar);
        std::string right = checkType(node->nodes[1],curr,vars,arrs,funcs,isNumVar);
        if(left == "INT" && left == right) return "INT";
        else if(left == "BOOL" && left == right) return "BOOL";
        else return "";
    }
    else if(which == "LogicalAnd" || which == "LogicalOr"){
        std::string left = checkType(node->nodes[0],curr,vars,arrs,funcs,isNumVar);
        std::string right = checkType(node->nodes[1],curr,vars,arrs,funcs,isNumVar);
        if(left == "BOOL" && left == right) return "BOOL";
        else return "";
    }
    else if(which == "EQ" || which == "NEQ"){
        std::string left = checkType(node->nodes[0],curr,vars,arrs,funcs,isNumVar);
        std::string right = checkType(node->nodes[1],curr,vars,arrs,funcs,isNumVar);
        if(left == right) return "BOOL";
        else return "";
    }
    else if(which == "LT" || which == "GT" || which == "LEQ" || which == "GEQ"){
        std::string left = checkType(node->nodes[0],curr,vars,arrs,funcs,isNumVar);
        std::string right = checkType(node->nodes[1],curr,vars,arrs,funcs,isNumVar);
        if(left == right && left != "BOOL" && left.find("*") == std::string::npos) return "BOOL";
        else return "";
    }
    else if(which == "REF"){
        std::string expr = checkType(node->nodes[0],curr,vars,arrs,funcs,isNumVar);
        return (expr + "*");
    }
    else if(which == "DE"){
        std::string expr = checkType(node->nodes[0],curr,vars,arrs,funcs,isNumVar);
        if(expr.find("*") != std::string::npos){
            expr.pop_back();
            return expr;
        }
        if(expr == "INT") return "INT*";
        Node* it = node->nodes[0];
        while(it->which != "ID") it = it->nodes[0];
        std::cout<<"Error: Variable "<<((ID*)(it))->kind<<" is not a pointer\n";
        return "";
    }
    else if(which == "+"){
        std::string expr = checkType(node->nodes[0],curr,vars,arrs,funcs,isNumVar);
        if(expr == "INT" || expr == "FLOAT") return expr;
        else return "";
    }
    else if(which == "-"){
        std::string expr = checkType(node->nodes[0],curr,vars,arrs,funcs,isNumVar);
        if(expr == "INT" || expr == "FLOAT") return expr;
        else return "";
    }
    else if(which == "NOT"){
        std::string expr = checkType(node->nodes[0],curr,vars,arrs,funcs,isNumVar);
        if(expr == "INT" || expr == "BOOL") return expr;
        else return "";
    }
    else if(which == "CONDITION"){
        std::string expr = checkType(node->nodes[0],curr,vars,arrs,funcs,isNumVar);
        if(expr == "BOOL") return "VOID";
        else return "";
    }
    else{
        if(which == "BLOCK"){
            Scope *new_scope = new Scope();
            Scope *arr_scope = new Scope();
            vars.push(new_scope);
            arrs.push(arr_scope);
        }
        std::string ret = "VOID";
        for(auto n:node->nodes){
            std::string expr = checkType(n,curr,vars,arrs,funcs,isNumVar);
            if(expr == ""){
                ret = "";
                break;
            }
            if(which == "BLOCK" && n->which == "RETURN"){
                if(curr == expr) ret = "RETURN";
                else{
                    ret = "";
                    std::cout<<"Error: Invalid return type\n";
                }
                break;
            }
        }
        if(which == "BLOCK"){
            vars.pop();
            arrs.pop();
        }
        return ret;
    }
}

bool semanticCheck(Node* AST){
    std::stack<Scope*> vars;
    std::stack<Scope*> arrs;
    std::unordered_map<std::string,std::vector<std::string>> funcs;
  	std::unordered_map<std::string,bool> isNumVar;
    bool check = checkDecBeforeUse(AST,"VOID",vars,arrs,funcs,isNumVar);
    if(!check){
        std::cout<<"Error: Declaration must be done before use\n";
        return false;
    }
    std::string prog = checkType(AST,"VOID",vars,arrs,funcs,isNumVar);
    if(prog == ""){
        std::cout<<"Error: Program is not type correct\n";
        return false;
    }
    return true;
}
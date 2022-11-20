#ifndef __SEMANTICS_HH__
#define __SEMANTICS_HH__
#include <algorithm>
#include <iostream>
#include <stack>
#include <unordered_map>
#include "node.hpp"

class Scope{
public:
    mutable std::vector<std::string> identifiers;
    mutable std::vector<std::string> types;
    mutable std::vector<int> sizes;
    Scope(){
        identifiers.clear();
        types.clear();
        sizes.clear();
    }
    Scope(Scope &s){
        identifiers = s.identifiers;
        types = s.types;
        sizes = s.sizes;
    }
    Scope(Scope *s){
        identifiers = s->identifiers;
        types = s->types;
        sizes = s->sizes;
    }

    void add_symbol(std::string id,std::string ty,int size = 0){
        identifiers.push_back(id);
        types.push_back(ty);
        sizes.push_back(size);
    }

    std::string get_type(std::string id){
        auto it = std::find(identifiers.begin(),identifiers.end(),id);
        if(it == identifiers.end()) return "";
        return types[it - identifiers.begin()];
    }

    bool check_scope(std::string id){
        auto it = std::find(identifiers.begin(),identifiers.end(),id);
        return it != identifiers.end();
    }
};

std::pair<bool,std::string> get_symbol(std::stack<Scope*>& scopes,std::string id);
std::vector<std::string> getFunParams(Function* function,std::string curr);
bool checkFuncDecArgs(std::vector<std::string> func_params, std::vector<Node*> tree_params);
bool checkFuncUseArgs(std::vector<std::string> func_params, std::vector<std::string> used_params);
bool checkDecBeforeUse(Node* node,std::string curr,std::stack<Scope*>& vars,std::stack<Scope*>& arrs, std::unordered_map<std::string, std::vector<std::string>>& funcs,  std::unordered_map<std::string, bool>& isNumVar);
std::string checkType(Node* node,std::string curr,std::stack<Scope*>& vars,std::stack<Scope*>& arrs,std::unordered_map<std::string,std::vector<std::string>>& funcs,std::unordered_map<std::string, bool>& isNumVar);
bool semanticCheck(Node* AST);
#endif
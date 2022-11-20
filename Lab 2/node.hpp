#ifndef __NODE_HH__
#define __NODE_HH__
#include <string>
#include <vector>
#include "llvm.hpp"

class Node{
public:
    std::string which;
    std::vector<Node*> nodes;

    Node(){
        which = "";
    }
    Node(std::string t){
        which = t;
    }
    Node(Node &n){
        which = n.which;
        nodes = n.nodes;
    }
    Node(Node *n){
        which = n->which;
        nodes = n->nodes;
    }

    virtual VALUE codegen();
};

class Dec : public Node{
public:
    Dec(){
        which = "";
    }
    Dec(std::string t){
        which = t;
    }
    Dec(Dec &n){
        which = n.which;
        nodes = n.nodes;
    }
    Dec(Dec *n){
        which = n->which;
        nodes = n->nodes;
    }
    Dec(Node *n){
        which = n->which;
        nodes = n->nodes;
    }

    VALUE codegen(bool global);
};

class InitDec : public Node{
public:
    InitDec(){
        which = "";
    }
    InitDec(std::string t){
        which = t;
    }
    InitDec(InitDec &n){
        which = n.which;
        nodes = n.nodes;
    }
    InitDec(InitDec *n){
        which = n->which;
        nodes = n->nodes;
    }
    InitDec(Node *n){
        which = n->which;
        nodes = n->nodes;
    }

    VALUE codegen(bool global, TYPE t);
};

class Var : public Node{
public:
    Var(){
        which = "";
    }
    Var(std::string t){
        which = t;
    }
    Var(Var &n){
        which = n.which;
        nodes = n.nodes;
    }
    Var(Var *n){
        which = n->which;
        nodes = n->nodes;
    }
    Var(Node *n){
        which = n->which;
        nodes = n->nodes;
    }

    VALUE codegen(bool global, TYPE t);
};

class Pointer : public Node{
public:
    Pointer(){
        which = "";
    }
    Pointer(std::string t){
        which = t;
    }
    Pointer(Pointer &n){
        which = n.which;
        nodes = n.nodes;
    }
    Pointer(Pointer *n){
        which = n->which;
        nodes = n->nodes;
    }
    Pointer(Node *n){
        which = n->which;
        nodes = n->nodes;
    }

    VALUE codegen(bool global, TYPE t);
};

class Array : public Node{
public:
    Array(){
        which = "";
    }
    Array(std::string t){
        which = t;
    }
    Array(Array &n){
        which = n.which;
        nodes = n.nodes;
    }
    Array(Array *n){
        which = n->which;
        nodes = n->nodes;
    }
    Array(Node *n){
        which = n->which;
        nodes = n->nodes;
    }

    VALUE codegen(bool global, TYPE t);
};

class Function : public Node{
public:
    Function(){
        which = "";
    }
    Function(std::string t){
        which = t;
    }
    Function(Function &n){
        which = n.which;
        nodes = n.nodes;
    }
    Function(Function *n){
        which = n->which;
        nodes = n->nodes;
    }
    Function(Node *n){
        which = n->which;
        nodes = n->nodes;
    }

    VALUE codegen(bool global,TYPE t);
};

class Param : public Node{
public:
    bool numVariable=false;
    Param(){
        which = "";
    }
    Param(std::string t){
        which = t;
    }
    Param(Param &n){
        which = n.which;
        nodes = n.nodes;
    }
    Param(Param *n){
        which = n->which;
        nodes = n->nodes;
    }
    Param(Node *n){
        which = n->which;
        nodes = n->nodes;
    }
};

class Branch : public Node{
public:
    Branch(){
        which = "";
    }
    Branch(std::string t){
        which = t;
    }
    Branch(Branch &n){
        which = n.which;
        nodes = n.nodes;
    }
    Branch(Branch *n){
        which = n->which;
        nodes = n->nodes;
    }
    Branch(Node *n){
        which = n->which;
        nodes = n->nodes;
    }

    VALUE codegen(TYPE t);
};

class FunctionBlock : public Node{
public:
    FunctionBlock(){
        which = "";
    }
    FunctionBlock(std::string t){
        which = t;
    }
    FunctionBlock(FunctionBlock &n){
        which = n.which;
        nodes = n.nodes;
    }
    FunctionBlock(FunctionBlock *n){
        which = n->which;
        nodes = n->nodes;
    }
    FunctionBlock(Node *n){
        which = n->which;
        nodes = n->nodes;
    }

    VALUE codegen(VALUE func,TYPE t);
};

class Conditional : public Node{
public:
    Conditional(){
        which = "";
    }
    Conditional(std::string t){
        which = t;
    }
    Conditional(Conditional &n){
        which = n.which;
        nodes = n.nodes;
    }
    Conditional(Conditional *n){
        which = n->which;
        nodes = n->nodes;
    }
    Conditional(Node *n){
        which = n->which;
        nodes = n->nodes;
    }

    VALUE codegen(TYPE t,BLOCK next);
};

class Constant : public Node{
public:
    std::string kind;
    int num;
    float dec;
    std::string lit;
    Constant(){
        which = "Constant";
    }
    Constant(int i){
        which = "Constant";
        kind = "INT";
        num = i;
    }
    Constant(float f){
        which = "Constant";
        kind = "FLOAT";
        dec = f;
    }
    Constant(std::string s){
        which = "Constant";
        kind = "CHAR*";
        lit = s;
    }

    VALUE codegen();
};

class ID : public Node{
public:
    std::string kind;
    ID(){
        which = "ID";
        kind = "";
    }
    ID(std::string t){
        which = "ID";
        kind = t;
    }

    VALUE codegen();
};
#endif
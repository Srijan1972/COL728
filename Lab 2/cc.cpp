#include <stdlib.h>
#include <assert.h>
#include "c.tab.hpp"
#include "semantics.hpp"
#include "codegen.hpp"

extern "C" int yylex();
int yyparse();
extern "C" FILE *yyin;
extern "C" Node *AST;

void printTree(Node *node,int depth){
    if(node == NULL) return;
    for(int i = 0; i < depth; i++) std::cout<<"| ";
    if(node->which == "Constant"){
        std::string kind = ((Constant*)node)->kind;
        if(kind == "INT") std::cout<<((Constant*)node)->num<<'\n';
        else if(kind == "FLOAT") std::cout<<((Constant*)node)->dec<<'\n';
        else std::cout<<((Constant*)node)->lit<<'\n';
    }
    else if(node->which == "ID") std::cout<<((ID*)node)->kind<<'\n';
    else std::cout<<node->which<<'\n';

    for(Node* n:node->nodes) printTree(n,depth + 1);
}

static void usage()
{
    std::cout<<"Usage: cc <prog.c>\n";
    exit(1);
}

int
main(int argc, char **argv)
{
    if (argc != 2) usage();
    char const *filename = argv[1];
    yyin = fopen(filename, "r");
    assert(yyin);
    // Lexing + Parsing to Create AST
    int ret = yyparse();
    // Outputting Created AST
    // printTree(AST,0);
    // Semantic Analysis
    ret = semanticCheck(AST);
    if(ret == 0) exit(0);
    // Codegen
    codegen(AST);
    exit(0);
}

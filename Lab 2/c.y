%code requires{
	#include "node.hpp"
}

%{
#include <cstdio>
#include <iostream>
#include "node.hpp"
using namespace std;

// stuff from flex that bison needs to know about:
extern "C" int yylex();
int yyparse();
extern "C" FILE *yyin;
 
void yyerror(const char *s);

Node* AST = NULL;
%}

%union {
	int num;
	float dec;
	char* lit;
	Node* node;
	Constant* constant;
	ID* id;
}
%token <lit> IDENTIFIER STRING_LITERAL
%token <num> I_CONSTANT
%token <dec> F_CONSTANT
%token  FUNC_NAME SIZEOF
%token	PTR_OP INC_OP DEC_OP LEFT_OP RIGHT_OP LE_OP GE_OP EQ_OP NE_OP
%token	AND_OP OR_OP MUL_ASSIGN DIV_ASSIGN MOD_ASSIGN ADD_ASSIGN
%token	SUB_ASSIGN LEFT_ASSIGN RIGHT_ASSIGN AND_ASSIGN
%token	XOR_ASSIGN OR_ASSIGN

%token	TYPEDEF EXTERN STATIC AUTO REGISTER INLINE
%token	CONST RESTRICT VOLATILE
%token	BOOL CHAR SHORT INT LONG SIGNED UNSIGNED FLOAT DOUBLE VOID
%token	COMPLEX IMAGINARY 
%token	STRUCT UNION ELLIPSIS

%token	CASE DEFAULT IF ELSE SWITCH WHILE DO FOR GOTO CONTINUE BREAK RETURN

%token	ALIGNAS ALIGNOF ATOMIC GENERIC NORETURN STATIC_ASSERT THREAD_LOCAL

%type <node> multiplicative_expression additive_expression shift_expression relational_expression equality_expression and_expression exclusive_or_expression inclusive_or_expression argument_expression_list primary_expression logical_and_expression jump_statement iteration_statement selection_statement expression_statement parameter_declaration unary_operator postfix_expression expression logical_or_expression statement parameter_list unary_expression conditional_expression block_item parameter_type_list assignment_expression init_declarator block_item_list direct_declarator pointer type_specifier init_declarator_list compound_statement declarator declaration_specifiers declaration function_definition external_declaration translation_unit
%type <constant> constant

%start translation_unit
%%

translation_unit
	: external_declaration {$$ = new Node("BEGIN"); $$->nodes.push_back($1); AST = $$;}
	| translation_unit external_declaration {$$ = $1; $1->nodes.push_back($2);}
	;

external_declaration
	: function_definition {$$ = $1;}
	| declaration {$$ = $1;}
	;

function_definition
	: declaration_specifiers declarator compound_statement {$$ = new Node("FUN"); for(Node* n:$1->nodes){$$->nodes.push_back(n);} $$->nodes.push_back($2); FunctionBlock* temp = new FunctionBlock($3); $$->nodes.push_back($3);}
	;

declaration
	: declaration_specifiers init_declarator_list ';' {$$ = new Dec("DEC"); for(Node* n:$1->nodes){$$->nodes.push_back(n);} $$->nodes.push_back($2);}
	;

declaration_specifiers
	: type_specifier {$$ = new Node("DEC-SPEC"); $$->nodes.push_back($1);}
	;

declarator
	: pointer direct_declarator {Node* temp = $1; while(temp->nodes.size() > 0){temp = temp->nodes[0];} temp->nodes.push_back($2); $$ = $1;}
	| direct_declarator {$$ = $1;}
	;

compound_statement
	: '{' '}' {$$ = new Node("BLOCK");}
	| '{'  block_item_list '}' {$$ = $2;}
	;

init_declarator_list
	: init_declarator {$$ = new InitDec("INIT-DEC"); $$->nodes.push_back($1);}
	| init_declarator_list ',' init_declarator {$$ = $1; $$->nodes.push_back($3);}
	;

type_specifier
	: VOID {$$ = new Node("VOID");}
	| CHAR {$$ = new Node("CHAR");}
	| INT {$$ = new Node("INT");}
	| FLOAT {$$ = new Node("FLOAT");}
	| BOOL {$$ = new Node("BOOL");}
	;

pointer
	: '*' pointer {$$ = new Pointer("PTR"); $$->nodes.push_back($2);}
	| '*' {$$ = new Pointer("PTR");}
	;

direct_declarator
	: IDENTIFIER {ID* temp = new ID($1); $$ = new Var("VARIABLE"); $$->nodes.push_back(temp);}
	| IDENTIFIER '[' I_CONSTANT ']'	 {ID* temp = new ID($1); Constant* temp1 = new Constant($3); $$ = new Array("ARRAY"); $$->nodes.push_back(temp); $$->nodes.push_back(temp1);}
	| IDENTIFIER '(' parameter_type_list ')' {ID* temp = new ID($1); $$ = new Function("FUNCTION"); $$->nodes.push_back(temp); $$->nodes.push_back($3);}
	;

block_item_list
	: block_item {$$ = new Node("BLOCK"); $$->nodes.push_back($1);}
	| block_item_list block_item {$$ = $1; $$->nodes.push_back($2);}
	;

init_declarator
	: declarator {$$ = $1;}
	;

assignment_expression
	: conditional_expression {$$ = $1;}
	| unary_expression '=' assignment_expression {$$ = new Node("ASSIGN"); $$->nodes.push_back($1); $$->nodes.push_back($3);}
	;

parameter_type_list
	: parameter_list ',' ELLIPSIS {((Param*) $1)->numVariable=true; $$ = $1;}
	| parameter_list {$$ = $1;}
	| VOID {$$ = new Node("VOID");}
	;

block_item
	: declaration {$$ = $1;}
	| statement {$$ = $1;}
	;

conditional_expression
	: logical_or_expression {$$ = $1;}
	| logical_or_expression '?' expression ':' conditional_expression {$$ = new Node("TERNARY"); $$->nodes.push_back($1); $$->nodes.push_back($3); $$->nodes.push_back($5);}
	;

unary_expression
	: postfix_expression {$$ = $1;}
	| INC_OP unary_expression {$$ = new Node("++"); $$->nodes.push_back($2);}
	| DEC_OP unary_expression {$$ = new Node("--"); $$->nodes.push_back($2);}
	| unary_operator unary_expression {$$ = $1; $1->nodes.push_back($2);}
	;

parameter_list
	: parameter_declaration {$$ = new Param("PAR-LIST"); $$->nodes.push_back($1);}
	| parameter_list ',' parameter_declaration {$$ = $1; $$->nodes.push_back($3);}
	;

statement
	: compound_statement {$$ = $1;}
	| expression_statement {$$ = $1;}
	| selection_statement {$$ = $1;}
	| iteration_statement {$$ = $1;}
	| jump_statement {$$ = $1;}
	;

logical_or_expression
	: logical_and_expression {$$ = $1;}
	| logical_or_expression OR_OP logical_and_expression {$$ = new Node("LogicalOr"); $$->nodes.push_back($1); $$->nodes.push_back($3);}
	;

expression
	: assignment_expression {$$ = $1;}
	;

postfix_expression
	: primary_expression {$$ = $1;}
	| postfix_expression '[' expression ']' {Node* temp = new Node("[]"); temp->nodes.push_back($3); $1->nodes.push_back(temp); $$ = $1;}
	| postfix_expression '(' ')' {$$ = $1;}
	| postfix_expression '(' argument_expression_list ')' {$1->nodes.push_back($3); $$ = $1;}
	| postfix_expression INC_OP {$$ = new Node("++"); $$->nodes.push_back($1);}
	| postfix_expression DEC_OP {$$ = new Node("--"); $$->nodes.push_back($1);}
	;

unary_operator
	: '&' {$$ = new Node("REF");}
	| '*' {$$ = new Node("DE");}
	| '+' {$$ = new Node("+");}
	| '-' {$$ = new Node("-");}
	| '!' {$$ = new Node("NOT");}
	;

parameter_declaration
	: type_specifier IDENTIFIER {Node* temp = new Node("PAR-DEC"); temp->nodes.push_back($1); temp->nodes.push_back(new ID($2)); $$ = temp;}
	| type_specifier pointer IDENTIFIER {Node* temp = new Node("PAR-DEC"); $1->which += "*"; temp->nodes.push_back($1); Node* temp1 = $2; while(temp1->nodes.size() > 0){$1->which += "*"; temp1 = temp1->nodes[0];} temp->nodes.push_back(new ID($3)); $$ = temp;}
	;

expression_statement
	: ';' {$$ = new Node(";");}
	| expression ';' {$$ = $1;}
	;

selection_statement
	: IF '(' expression ')' statement ELSE statement {Branch* temp = new Branch("ITE"); Node* temp1 = new Node("CONDITION"); temp1->nodes.push_back($3); Conditional* temp2 = new Conditional($5); Conditional* temp3 = new Conditional($7); temp->nodes.push_back(temp1); temp->nodes.push_back(temp2); temp->nodes.push_back(temp3); $$ = temp;}
	| IF '(' expression ')' statement {Branch* temp = new Branch("IT"); Node* temp1 = new Node("CONDITION"); temp1->nodes.push_back($3); Conditional* temp2 = new Conditional($5); temp->nodes.push_back(temp1); temp->nodes.push_back(temp2); $$ = temp;}
	;

iteration_statement
	: WHILE '(' expression ')' statement {Branch* temp = new Branch("WHILE"); Node* temp1 = new Node("CONDITION"); temp1->nodes.push_back($3); temp->nodes.push_back(temp1); Conditional* temp2 = new Conditional($5); temp->nodes.push_back(temp2); $$ = temp;}
	| DO statement WHILE '(' expression ')' ';' {Branch* temp = new Branch("DO-WHILE"); Node* temp1 = new Node("CONDITION"); temp1->nodes.push_back($5); Conditional* temp2 = new Conditional($2); temp->nodes.push_back(temp2); temp->nodes.push_back(temp1); $$ = temp;}
	| FOR '(' expression_statement expression_statement expression ')' statement {Branch* temp = new Branch("FOR"); temp->nodes.push_back($3); temp->nodes.push_back($4); temp->nodes.push_back($5); Conditional* temp1 = new Conditional($7); temp->nodes.push_back(temp1); $$ = temp;}
	;

jump_statement
	: RETURN ';' {$$ = new Node("RETURN");}
	| RETURN expression ';' {$$ = new Node("RETURN"); $$->nodes.push_back($2);}
	/* : GOTO IDENTIFIER ';' */
	/* | CONTINUE ';' */
	/* | BREAK ';' */
	;

logical_and_expression
	: inclusive_or_expression {$$ = $1;}
	| logical_and_expression AND_OP inclusive_or_expression {$$ = new Node("LogicalAnd"); $$->nodes.push_back($1); $$->nodes.push_back($3);}
	;

primary_expression
	: IDENTIFIER {$$ = new ID($1);}
	| constant {$$ = $1;}
	| '(' expression ')' {$$ = $2;}
	;

constant
	: I_CONSTANT {$$ = new Constant($1);}
	| F_CONSTANT {$$ = new Constant($1);}
	| STRING_LITERAL {$$ = new Constant($1);}
	;

argument_expression_list
	: assignment_expression {$$ = new Node("arg_list"); $$->nodes.push_back($1);}
	| argument_expression_list ',' assignment_expression {Node* temp = new Node("arg_list"); for(Node* it:$1->nodes){temp->nodes.push_back(it);} temp->nodes.push_back($3); $$ = temp;}
	;

inclusive_or_expression
	: exclusive_or_expression {$$ = $1;}
	| inclusive_or_expression '|' exclusive_or_expression {$$ = new Node("BitwiseOr"); $$->nodes.push_back($1); $$->nodes.push_back($3);}
	;

exclusive_or_expression
	: and_expression {$$ = $1;}
	| exclusive_or_expression '^' and_expression {$$ = new Node("XOR"); $$->nodes.push_back($1); $$->nodes.push_back($3);}
	;

and_expression
	: equality_expression {$$ = $1;}
	| and_expression '&' equality_expression {$$ = new Node("BitwiseAnd"); $$->nodes.push_back($1); $$->nodes.push_back($3);} 
	;

equality_expression
	: relational_expression {$$ = $1;}
	| equality_expression EQ_OP relational_expression {$$ = new Node("EQ"); $$->nodes.push_back($1); $$->nodes.push_back($3);}
	| equality_expression NE_OP relational_expression {$$ = new Node("NEQ"); $$->nodes.push_back($1); $$->nodes.push_back($3);}
	;

relational_expression
	: shift_expression {$$ = $1;}
	| relational_expression '<' shift_expression {$$ = new Node("LT"); $$->nodes.push_back($1); $$->nodes.push_back($3);}
	| relational_expression '>' shift_expression {$$ = new Node("GT"); $$->nodes.push_back($1); $$->nodes.push_back($3);}
	| relational_expression LE_OP shift_expression {$$ = new Node("LEQ"); $$->nodes.push_back($1); $$->nodes.push_back($3);}
	| relational_expression GE_OP shift_expression {$$ = new Node("GEQ"); $$->nodes.push_back($1); $$->nodes.push_back($3);}
	;

shift_expression
	: additive_expression {$$ = $1;}
	| shift_expression LEFT_OP additive_expression {$$ = new Node("LEFT"); $$->nodes.push_back($1); $$->nodes.push_back($3);}
	| shift_expression RIGHT_OP additive_expression {$$ = new Node("RIGHT"); $$->nodes.push_back($1); $$->nodes.push_back($3);}
	;

additive_expression
	: multiplicative_expression {$$ = $1;}
	| additive_expression '+' multiplicative_expression {$$ = new Node("PLUS"); $$->nodes.push_back($1); $$->nodes.push_back($3);}
	| additive_expression '-' multiplicative_expression {$$ = new Node("MINUS"); $$->nodes.push_back($1); $$->nodes.push_back($3);}
	;

multiplicative_expression
	: unary_expression {$$ = $1;}
	| multiplicative_expression '*' unary_expression {$$ = new Node("TIMES"); $$->nodes.push_back($1); $$->nodes.push_back($3);}
	| multiplicative_expression '/' unary_expression {$$ = new Node("DIV"); $$->nodes.push_back($1); $$->nodes.push_back($3);}
	| multiplicative_expression '%' unary_expression {$$ = new Node("MOD"); $$->nodes.push_back($1); $$->nodes.push_back($3);}
	;

%%
#include <stdio.h>

void yyerror(const char *s)
{
	fflush(stdout);
	fprintf(stderr, "*** %s\n", s);
}

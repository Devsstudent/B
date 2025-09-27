%{
	#include "B_header.h"
	#include "lexing.h"
	#include "parsing.h"
	#include "building.h"

	#include <stdbool.h>
	#include <stdlib.h>
	#include <stdio.h>
	#include <string.h>
	int		yylex(void);
	t_scope		*current_scope;
	LLVMModuleRef	mod;
	LLVMBuilderRef	builder;
%}

%union {
	int					num;
	char				c;
	char				*str;
	char				*name;
	char				**parameter;
	t_node_var			*const_val;
	t_g_var_list		**def_list;
	t_param				*fonction_arguments;
	t_scope				*current_scope;
	t_unary				unary;
	t_statement			*statement;
	t_statement_list		*stm_list;
	t_kind			binary;
	t_var_list		**var_list;
	t_node_var	*node_v;
}

%start	program
%type	<node_v> constant opt_constant const_or_name var_definition
%type	<str> statement_content
%type	<str> statement_def
%type	<fonction_arguments> params
%type	<def_list> name_cst_chain
%type	<var_list>	opt_constant_list
%type	<unary>			unary
%type	<statement>			opt_rvalue
%type	<statement>			ret_content
%type	<stm_list>			rvalue_chain
%type	<statement>			rvalue
%type	<statement>			lvalue
%type	<binary>			binary
%type	<binary>			opt_binary
%type	<str>			inc_dec
%type	<str>			assignation
%type	<num>			opt_number

//%type	<num> ival

%token <c> CHARCONST
%token <num> NUMBER
%token <str> STRING
%token <name> IDENTIFIER;
%token <str> AUTO EXTRN RETURN IF ELSE WHILE GOTO
%token <binary> LPAREN ASSIGN PLUS MINUS MUL MOD DIV NE LT GT EQ GT_EQ LT_EQ SHIFT_L SHIFT_R


%right ASSIGN
%left OR               /* || */  
%left AND              /* && */
%left EQ NE            /* == != */
%left PLUS MINUS
%left MUL DIV MOD
%nonassoc UMINUS
%nonassoc LT GT  /* <, >, == */
%left LPAREN


//%destructor { del_var_elem($$); printf("im CALLLED\n"); } constant opt_constant
//%destructor { free($$); } IDENTIFIER
//%destructor { del_definition($$); printf("definition destructor called\n"); } definition

%%

program
	: /* empty */
	| program var_definition { 
		// Depending on the kind  for the second parameter
		// Also check the return, if not null then it means there is a global alerady existing.
		addGlobalVariable($2, mod);
		/* handle defintion  genre qui check le type etc  function ou var*/
//		t_g_var_list *def = add_var_to_list(g_var_list, $2); if (def == NULL) {printf("ERROR\n"); yyerrok;} }
	}
	| program func_definition {
		}
	;

//opt_constant_list

var_definition	:
				/* empty */
				| IDENTIFIER opt_constant ';' /* Cas a; ou a 12; */
				{
					t_node_var *var = $2;
					printf("%s \n", var->val.str);
					var->name = $1;
					$$ = var;
				}
				 /* Doit pouvoir gérer les declaration de array */
				| IDENTIFIER '[' opt_number ']' opt_constant_list ';' {
					printf("TEst\n");
					/* Checker type des elements */ /* ensuite cree le tableau de ptr 32 */
					t_array *arr = new_arr($3, $5);
					t_node_var *new = new_node($1, (t_val)arr, CONST_ARR);


					if (!validArrayDeclaration(arr->values, arr->len) || !checkElemArrayAllSameType(arr)) {
						$$ = NULL;
					} else {
						adaptArraySize(arr);
						// Checker les types que ce soit tous les memes
						$$ = new;
					}

			//si il a pas de len et des elems faut adapter la taille au declaration
			// Case plus d'elem que declaraer size ca saute
			// Case moins d'elem que delcarer size (c'est ok)
			// 
			}
				// New t_array len = $3, var_list $5
				
				/* $3 c'est la list de constant*/
				// Function qui vas checker le type des elements declaré si il y en 

opt_constant_list:
				const_or_name { $$ = add_node($1, NULL); }
				| opt_constant_list ',' const_or_name {t_var_list **li = $1; $$ = add_node($3, li);}
				;

const_or_name:
			IDENTIFIER { t_val val = {0}; t_node_var *var = new_node($1, val, CONST_VAR);  $$ = var; }
			| constant { if ($1) { t_val val = $1->val; t_node_var *var = new_node(NULL, val, $1->kind);  $$ = var;} }

func_definition	:
				| IDENTIFIER LPAREN params ')' statement_def {
					LLVMTypeRef		proto = handleParam($3);
					LLVMValueRef	func = LLVMAddFunction(mod, $1, proto);
					t_func * new = malloc(sizeof(t_func));
					new->proto = proto;
					new->name = $1;
					new->next = NULL;
					t_func *buff = g_funcs.head;
					if (!buff) {
						g_funcs.head = new;
					}
					while (buff && buff->next) {
						buff = buff->next;
					}
					buff = new;
					for (int j = 0; j < $3->len; j++) {
						LLVMValueRef param = LLVMGetParam(func, j);
						LLVMSetValueName(param, $3->param[j]);
					}
					LLVMBasicBlockRef	functionEntry = LLVMAppendBasicBlock(func, $1);
					LLVMPositionBuilderAtEnd(builder, functionEntry);
					// On a besoin de lui passer les parameters ici
					handleStatements(&current_scope->statements, func, builder, mod);
					char *error = NULL;
    				LLVMVerifyModule(mod, LLVMAbortProcessAction, &error);
    				LLVMDisposeMessage(error);
					//Handle statement $5 pour généréles fonction.
					generate_asm(mod);
					create_scope();
					printf("current_scope : %i\n", current_scope->statements.size);
					//Free all the statements
				}
				;


statement_def	:
			AUTO name_cst_chain ';' { }
			|'{' statement_content '}' {
				// Ici faudrait géré le handle statement du scope etc
				// pop_scope
				printf("Handle content\n");
			}
			| EXTRN name_cst_chain ';' {$$ = NULL}
			;

statement_content	:
					/* empty */
					| statement_content AUTO name_cst_chain ';' {
						// TO DO define type statemetn
						handle_declaration($3,  &current_scope->statements, false);
//						add_statement_elem($3, VAR, &current_scope->statements);
}
					| statement_content EXTRN name_cst_chain ';' {handle_declaration($3, &current_scope->statements, true);}
					| statement_content RETURN ret_content ';' {
						t_statement *ret_stm = new_statement(NULL, RET, $3, NULL);
						add_statement_elem(ret_stm, &current_scope->statements);}
					| statement_content opt_rvalue ';' {add_statement_elem($2, &current_scope->statements) ;}
					| statement_content WHILE LPAREN rvalue ')' statement_def { printf("WHILE\n"); }
					;

ret_content:
				{ $$ = NULL }
			| LPAREN opt_rvalue ')' { $$ = $2; }

name_cst_chain	:
				IDENTIFIER { t_node_var *var = new_node($1, (t_val){0}, CONST_VAR); $$ = add_elem(NULL, var);}
				| name_cst_chain ',' IDENTIFIER { t_node_var *var = new_node($3, (t_val){0}, CONST_VAR); $$ = add_elem($1, var);}
				;
opt_rvalue:
			/* empty */ {$$ = NULL;}
			| rvalue {$$ = $1;}

rvalue_chain:
			opt_rvalue {
				if ($1) {
					t_statement_list *new_list = new_stm_list();
					add_statement_elem($1, new_list);
					$$ = new_list; }
				}
			| rvalue_chain ',' rvalue {
				add_statement_elem($3, $1);
				$$ = $1;
			}

rvalue	:
			LPAREN rvalue ')' {$$ = $2;}
			| rvalue LPAREN rvalue_chain ')' {
				// normalement $1 c'est toujours un nom de la function
				t_function_call	*func_call = malloc(sizeof(t_function_call));

				char *func_name = $1->content->name;
				func_call->name = func_name;
				func_call->head = $3;
				t_statement_elem *buff = $3->head;
				int size = 0;
				while (buff) {
					size++;
					buff = buff->next;
				}
				func_call->nb_param = size;
				t_statement *func_stm = new_statement(NULL, CALL, NULL, NULL);
				func_stm->call = func_call;
//				add_statement_elem(func_stm, &current_scope->statements);
				$$ = func_stm;
			}
			| lvalue {$$ = $1;}
			| constant { $$ = new_statement($1, CONST, NULL, NULL); }
			| unary rvalue %prec UMINUS
			| lvalue assignation rvalue { /* ici on recupere $3 pour le mettre en rhs */
				t_statement *lhs = $1;
				t_statement *rhs = $3;
				printf("wtf %p %i\n", rhs, rhs->kind);
				$$ = new_statement(NULL, OP_ASSIGN, rhs, lhs);}

			| inc_dec rvalue {/* $$ =  */}
			| lvalue inc_dec
			| '&' lvalue { /* Pour passer des addresses */ }
			| rvalue PLUS rvalue     {
				t_statement *lhs = $1; /* $$ =   */
				t_statement *rhs = $3;/* $$ =   */
				$$ = new_statement(NULL, OP_ADD, rhs, lhs);}
			| rvalue MINUS rvalue     {
				t_statement *lhs = $1; /* $$ =   */
				t_statement *rhs = $3;/* $$ =   */
				$$ = new_statement(NULL, OP_SUB, rhs, lhs);}
			| rvalue MUL rvalue      {
				t_statement *lhs = $1; /* $$ =   */
				t_statement *rhs = $3;/* $$ =   */
				$$ = new_statement(NULL, OP_MUL, rhs, lhs);}
			| rvalue DIV rvalue      {
				t_statement *lhs = $1; /* $$ =   */
				t_statement *rhs = $3;/* $$ =   */
				$$ = new_statement(NULL, OP_DIV, rhs, lhs);}
			| rvalue MOD rvalue      {
				t_statement *lhs = $1; /* $$ =   */
				t_statement *rhs = $3;/* $$ =   */
				$$ = new_statement(NULL, OP_MOD, rhs, lhs);}
			;

//| rvalue EQ rvalue       BINARY_OP(EQ)
//| rvalue NE rvalue       BINARY_OP(NE)
//| rvalue LT rvalue       BINARY_OP(LT)
//| rvalue GT rvalue       BINARY_OP(GT)
//| rvalue OR rvalue       BINARY_OP(OR)
//| rvalue AND rvalue      BINARY_OP(AND)
//			| rvalue binary rvalue {printf("OPERATION %i \n", $1->content->value->u.num/* ,$3->content->value->u.num */);

lvalue:
		IDENTIFIER {
			$$ = new_statement(new_node($1, (t_val){0}, CONST_VAR), VAR, NULL, NULL);
			// Géré les différences entre label, var-extrn et var auto plus tard
			// Peut etre rename en VAR tous cours
		}
		| '*' rvalue { /* Pour déréférence des variables qui doivent stocker une ref ptr*/ }
		| rvalue '[' rvalue ']' { /* Gestion des index d'un tableau */ } 
		;

assignation:
			ASSIGN opt_binary

opt_binary:
			/* empty */
			| binary
			;

binary:
			'|'
			| MINUS { $$ = OP_SUB;}
			| MUL   { $$ = OP_MUL;}
			| MOD   { $$ = OP_MOD;}
			| DIV   { $$ = OP_DIV;}
			| '!'
			| LT
			| GT
			| '|'
			| SHIFT_R
			| SHIFT_L
			| NE
			| LT_EQ
			| GT_EQ
			| EQ

inc_dec:
		'+''+'
		| '-''-'
		;

unary	:
		MINUS
		| '!'
		;

params: 
	/* empty */ { t_param a = (t_param){ .len = 0, .param = NULL }; $$ = &a; printf("emtpy param\n");}
	| IDENTIFIER {t_param first =  (t_param){ .len = 0, .param = NULL }; $$ = add_param(&first, $1);}
	| params ',' IDENTIFIER {$$ = add_param($1, $3);}
	;

opt_constant
    : /* empty */   { $$ = new_node(NULL, (t_val){0}, CONST_VAR /* maybe undefined yet */); }
    | constant      { $$ = $1 }
	;

//opt_ival:		/* empty */
//				| ival

constant
    : NUMBER     { $$ = new_node(NULL, (t_val){$1}, CONST_INT); }
    | CHARCONST  { $$ = new_node(NULL, (t_val){.num = (int)$1}, CONST_CHAR); }
    | STRING     { $$ = new_node(NULL, (t_val){.str = $1}, CONST_STR); }

opt_number:
		/* empty */ { $$ = -1;}
		| NUMBER { $$ = (int)$1; }

//ival:	constant
//	| IDENTIFIER


%%


int main(void) {
	printf("Welcome to a little B compiler :) \n");
	g_funcs.head = NULL;
	mod = LLVMModuleCreateWithName("b");
	current_scope = malloc(sizeof(t_scope));
	current_scope->statements = (t_statement_list){.head = NULL, .size = 0};
	builder = LLVMCreateBuilder();
    yyparse();
	return 1;
}

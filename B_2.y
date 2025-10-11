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
	LLVMContextRef ctx; // Global mais a voir
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
	t_value				vv;
	t_kind			binary;
	t_var_list		**var_list; t_node_var	*node_v;
	LLVMBasicBlockRef	blockRef;
	t_value_list		**val_li;
	LLVMValueRef		valRef;
}

%start	program
%type	<node_v>  var_definition
%type	<vv> constant opt_constant const_or_name
%type	<str> statement_content
%type	<str> statement_def
%type	<blockRef> func_name_params
%type	<fonction_arguments> params
%type	<val_li> name_cst_chain opt_constant_list
%type	<unary>			unary
%type	<const_val>			opt_rvalue
%type	<statement>			ret_content
%type	<stm_list>			rvalue_chain
%type	<const_val>			rvalue
%type	<const_val>			lvalue
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
				char *ir = LLVMPrintModuleToString(mod);
				printf("%s\n", ir);
		// Also check the return, if not null then it means there is a global alerady existing.
//		addGlobalVariable($2, mod);
//		printf("globalVar %p\n", $2);
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
					t_node_var	*new;
					t_to_assign	glob = buildPseudoVariableRep($2); // Pseudo Representation de la variable
					printf("%p %i\n", glob.ptr, glob.layer);
					t_assigned *t = malloc(sizeof(t_assigned));
					*t =(t_assigned) {.name=$1, .val=NULL, .type=NULL }; // LLVMVariablePreBuild
					typeBuilder(glob, 0, t); // LLVMVariablePreBuild
					char *type_str = LLVMPrintTypeToString(t->type);
					LLVMValueRef val = getValue(glob, *t,  $2.val);
					// Initializer
					printf("Pointed-to type: %s \n", type_str);
					LLVMValueRef globalArray = LLVMAddGlobal(mod, t->type, $1);
					// Set alignment a mettre a 4 si int ou ptr e
					LLVMSetInitializer(globalArray, val);
					if (glob.ptr) {
						LLVMTypeRef i8PtrTy = LLVMPointerType(LLVMInt8Type(), 0);
						new = new_ptr($1, globalArray, i8PtrTy, LLVMInt8Type(), 1);
					} else {
						new = new_node($1, globalArray, t->type, T_GLOBAL);
					}
					tsearch(new, &current_scope->root_tree, compare_var_name);
//					LLVMSetAlignment(globalVar, 1);
					(void) val;
	//				(void) glob;


	//				t_node_var *var = $2;
					//printf("%s \n", var->val.str);
	//				var->name = $1;
					// si c'est const_str, on check le NAME, du node, et enft c'est le contenu de la string
					// comme ca on peux 
//					LLVMTypeRef arrayType = LLVMArrayType(LLVMInt8TypeInContext(context), 3);
//					LLVMValueRef constStr = LLVMConstStringInContext(context, "af\0", 3, 1);

					//$$ = var;
				}
				 /* Doit pouvoir gérer les declaration de array */
				| IDENTIFIER '[' opt_number ']' opt_constant_list ';' {
					printf("TEst\n");
					/* Checker type des elements */ /* ensuite cree le tableau de ptr 32 */
					t_array *arr = new_arr($3, $5);
					
					if (validArrayDeclaration($5, $3)) {
				// Et si size est a 0 alors faut set la size a la sizede getsize
					// Adapt array
						if (arr->len == -1) {
							arr->len = getSizeValList(arr->values);
						}
						adaptArraySize(arr);
						t_value_list *buff = *arr->values;

						// Ajouter un malloc plutot
						LLVMValueRef elements[arr->len];
						t_assigned strings[arr->len];

						LLVMTypeRef type = NULL;
						int i = 0;
						while (buff) {
							// Check if it's a var
							// Sinon on generate le type la
								t_to_assign	pseudoRep = buildPseudoVariableRep(buff->elem); // Pseudo Representation de la variable
								t_assigned *t = malloc(sizeof(t_assigned));
								*t =(t_assigned) {.name="", .val=NULL, .type=NULL }; // LLVMVariablePreBuild
								typeBuilder(pseudoRep, 0, t); // LLVMVariablePreBuild
								char *type_str = LLVMPrintTypeToString(t->type);
							//	LLVMValueRef val = getValue(glob, *t,  buff->elem->vall.val);
								// Initializer
								printf("Pointed-to type: %s \n", type_str);

							if (!type) {
								type = t->type;
							} else if (type != t->type) {
								printf("Type in array doesn't match\n");
							}
							if (buff->elem.type == TYPE_STR) {
								LLVMValueRef str = getValue(pseudoRep, *t, buff->elem.val);
								LLVMTypeRef arrayType = LLVMArrayType(LLVMInt8Type(), strlen(buff->elem.val.str) + 1);
								LLVMValueRef globalVar = LLVMAddGlobal(mod, arrayType, buff->elem.val.str);
								LLVMSetInitializer(globalVar, str);
								strings[i].val = globalVar;
								strings[i].type = arrayType;
							} else {
								elements[i] = getValue(pseudoRep, *t, buff->elem.val);
							}
							i++;
							buff = buff->next;
						}
						LLVMTypeRef i8PtrTy = LLVMPointerType(LLVMInt8Type(), 0);
						LLVMTypeRef final =  NULL;
						int depth;
						LLVMTypeRef	arrType = NULL;
						LLVMTypeRef baseType = NULL;
						if (type == LLVMInt32Type() || type == LLVMInt8Type()) {
 							final = type;
							arrType = LLVMArrayType(type, arr->len);
							baseType = type;
							depth = 1;
						} else {
							LLVMValueRef indices[] = {
								LLVMConstInt(LLVMInt32Type(), 0, 0),
								LLVMConstInt(LLVMInt32Type(), 0, 0)
							};
							
							for (int i = 0; i < arr->len; i++) {
								printf("test %i\n", i);
								elements[i] = LLVMBuildInBoundsGEP2(builder,strings[i].type, strings[i].val, indices, 2, "");
							}
							final = i8PtrTy;
							arrType = LLVMArrayType(final, arr->len);
							depth = 2;
							baseType = LLVMInt8Type();
 							//final = LLVMPointerType(LLVMInt8Type(), 0);
							//final = LLVMArrayType(final, arr->len);
						}
						LLVMValueRef arrayVal = LLVMConstArray(final, elements, arr->len);
						LLVMValueRef globalArray = LLVMAddGlobal(mod, arrType, $1);
						LLVMSetInitializer(globalArray, arrayVal);
						t_node_var *node = new_ptr($1, globalArray, final, baseType, depth);
						tsearch(node, &current_scope->root_tree, compare_var_name);
						char *type_str = LLVMPrintTypeToString(final);
						printf("Pointed-to type: %s \n", type_str);
					}

					// Si c'est que des VAR on compare leurs LLVMTypeRef avec le LLVMGetGlobalTypeFromName
			
					// Sinon genere le typeRef pour la val, sachant que on a un ptr sur ce type 
			
					// Ensuite on verifie que toutes les variables on bien ce type. 

//					if (!validarraydeclaration(arr->values, arr->len) || !checkelemarrayallsametype(arr)) {
//						$$ = null;
//					}
					
//					t_node_var *new = new_node($1, (t_val)arr, CONST_ARR);
					// Parcoues opt_list si c'est char ou des int mais si c'est des string faut mesurer leurs size

//					if (!validarraydeclaration(arr->values, arr->len) || !checkelemarrayallsametype(arr)) {
//						$$ = null;
//					} else {
//						adaptArraySize(arr);
//						// Checker les types que ce soit tous les memes
//						$$ = new;
//					}

			//si il a pas de len et des elems faut adapter la taille au declaration
			// Case plus d'elem que declaraer size ca saute
			// Case moins d'elem que delcarer size (c'est ok)
			// 
			}
				// New t_array len = $3, var_list $5
				
				/* $3 c'est la list de constant*/
				// Function qui vas checker le type des elements declaré si il y en 

opt_constant_list:
				const_or_name { $$ = addToList($1, NULL); }
				| opt_constant_list ',' const_or_name {$$ = addToList($3, $1);}
				;

const_or_name:
			IDENTIFIER { t_value val = (t_value){.type=TYPE_IDENTIFIER, .val=(t_val){.str=$1}}; $$ = val; }
			| constant { $$ = $1; }

func_name_params:
	IDENTIFIER LPAREN params ')' { 
		create_scope();
		LLVMTypeRef		proto = handleParam($3);
		LLVMValueRef	func = LLVMAddFunction(mod, $1, proto);
		t_func *new = malloc(sizeof(t_func));
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
		// Stocker la ref dans le tree aussi
		LLVMBasicBlockRef	functionEntry = LLVMAppendBasicBlock(func, $1);
		LLVMPositionBuilderAtEnd(builder, functionEntry);
		printf("Handle the function before the statement\n");
		$$ = functionEntry;
	}

func_definition	:
				| func_name_params statement_def {
					// Ici on analyze statement_def pour savoir le type de retour
					// Ensuite on va cree une variable dans le context du module qui va contenir la fonction
					// Ensuite faire pointer la variable sur la functoin + 4.

					char *error = NULL;
					char *ir = LLVMPrintModuleToString(mod);
					printf("%s\n", ir);
					LLVMVerifyModule(mod, LLVMAbortProcessAction, &error);
    				LLVMDisposeMessage(error);
					generate_asm(mod);

//					// On a besoin de lui passer les parameters ici
//					handleStatements(&current_scope->statements, func, builder, mod);
//			
//					//Handle statement $5 pour généréles fonction.

//					create_scope();
//					printf("current_scope : %i\n", current_scope->statements.size);
//					//Free all the statements
					pop_scope();
				}
				;


statement_def	:
			AUTO name_cst_chain ';' { // Ajouter la variable dans le context }
			}
			|'{' statement_content '}'
				// Ici faudrait géré le handle statement du scope etc
				// pop_scope
				// Grace au token scope type je sait si je suis dans une fonction ou un while
				// 
				// On check le old_parent type ?? Meme pas sur on a juste besoin de savoir si on est dans un sous scope ou pas.
				// Si c'est le cas on gére le truc en mode layer et on 
	//			printf("Handle content\n");}
			| EXTRN name_cst_chain ';' {
				
				// Ajouter la varibale dans le current_scope->tree
				$$ = NULL};

statement_content	:
					/* empty */
					| statement_content AUTO name_cst_chain ';' {
						addAutoVarsToTree($3, &current_scope->root_tree);

						// Loop sur name_cst_chain, avec une fonction a appeler 
						// Function add var local la meme  qui sera appeler pour le label
						
//						t_node_var	*new = new_node(name, (t_val){.ref=extrn_var}, CONST_VAR);
//						t_node_var *found = var_tree_find(&current_scope->root_var, name);
//						if (!found)
//						/*void **result = */tsearch(new, &root_var, compare_var_name);
						
						// TO DO define type statemetn
						// Check dans le global
//						handle_declaration($3,  &current_scope->statements, false);
//						add_statement_elem($3, VAR, &current_scope->statements);
						// Ajouter la varibale dans le current_scope->tree
}
					| statement_content EXTRN name_cst_chain ';' {
						// On verifie que les name existe bien, et on remplace les node par leurs ref ?
						handleExternVars($3, &current_scope->root_tree, &current_scope->parent->root_tree);

						// Loop sur le name_cst_chain
						//LLVMValueRef extrn_var = LLVMGetNamedGlobal(mod, name);
						//if (!extrn_var) /* Handle erreur n'existe pas */
						//	continue ;
						//// look for the variable in
						//t_node_var *found = var_tree_find(&current_scope->root_var, name);
						//if (!found) /* Handle erreur */
						//	
						//found->val = {.ref = extrn_var}; // Normalement pas necessaire car c'est deja bien fait dans assign global
//						t_node_var	*new = new_node(name, (t_val){.ref=extrn_var}, CONST_VAR);
//	/*void **result = */tsearch(new, &root_var, compare_var_name);
//handle_declaration($3, &current_scope->statements, true);}
					}
					| statement_content RETURN ret_content ';' {
						// ret_content : resultat de do_op / ou variable / ou rien / ou const
			//			t_statement *ret_stm = new_statement(NULL, RET, $3, NULL);
			//			add_statement_elem(ret_stm, &current_scope->statements);
					}
					| statement_content opt_rvalue ';' {
				//			add_statement_elem($2, &current_scope->statements) ;
						printf("handle basic state\n");
					}
//					| statement_content WHILE LPAREN rvalue ')' statement_def { printf("WHILE\n"); }
					;

ret_content:
				{ 
					printf("return void\n");
					LLVMValueRef v = LLVMConstNull(LLVMPointerType(LLVMVoidType(), 0));

					// convert null pointer to i32 integer
					LLVMValueRef intVal = LLVMBuildPtrToInt(builder, v, LLVMInt32Type(), "ptr_to_int");
					LLVMBuildRet(builder, intVal);
					$$ = NULL
				}
			| LPAREN opt_rvalue ')' { 
				//$$ = $2; 
			//	if ($2 == NULL) {
			//		LLVMTypeRef void_ptr_type = LLVMPointerType(LLVMVoidType(), 0);
			//		LLVMValueRef null_void_ptr = LLVMConstNull(void_ptr_type);
			//		LLVMBuildRet(builder, null_void_ptr);
			//	} else {
			t_node_var *found = var_tree_find(&current_scope->root_tree, $2->name);
			printf("%p %s\n", found, $2->name);
			handleRet(found, builder);
//					LLVMValueRef v = $2->val.ref;
//					LLVMValueRef load = LLVMBuildLoad2(builder, LLVMInt32Type(), v, "returnLoad");
//
//					LLVMTypeRef void_ptr_type = LLVMPointerType(LLVMVoidType(), 0);
//					// Depends the type always..
//
//					printf("%p %p\n", v, $2);
//					LLVMValueRef int_ptr = LLVMBuildIntToPtr(builder, load, void_ptr_type, "inttoptr");
//
//					LLVMValueRef ptr_as_void = LLVMBuildBitCast(builder, int_ptr, void_ptr_type, "cast_to_voidptr");
//					LLVMBuildRet(builder,ptr_as_void);
			//	}
			}

name_cst_chain	:
				IDENTIFIER { t_value val = (t_value){.type=TYPE_IDENTIFIER, .val=(t_val){.str=$1}}; $$ = addToList(val, NULL); }
				| name_cst_chain ',' IDENTIFIER { t_value val = (t_value){.type=TYPE_IDENTIFIER, .val=(t_val){.str=$3}}; $$ = addToList(val, $1);}
				;
opt_rvalue:
			/* empty */ {$$ = NULL;}
			| rvalue { $$ = $1;}

rvalue_chain:
			opt_rvalue {
			//	if ($1) {
			//		t_statement_list *new_list = new_stm_list();
			//		add_statement_elem($1, new_list);
			//		$$ = new_list; }
				}
			| rvalue_chain ',' rvalue {
			//	add_statement_elem($3, $1);
			//	$$ = $1;
			}

rvalue	:
			//LPAREN rvalue ')' {$$ = $2;}
			//| rvalue LPAREN rvalue_chain ')' {
			//	// normalement $1 c'est toujours un nom de la function
			//	t_function_call	*func_call = malloc(sizeof(t_function_call));

			//	char *func_name = $1->content->name;
			//	func_call->name = func_name;
			//	func_call->head = $3;
			//	t_statement_elem *buff = $3->head;
			//	int size = 0;
			//	while (buff) {
			//		size++;
			//		buff = buff->next;
			//	}
			//	func_call->nb_param = size;
			//	t_statement *func_stm = new_statement(NULL, CALL, NULL, NULL);
			//	func_stm->call = func_call;
//			//	add_statement_elem(func_stm, &current_scope->statements);
			//	$$ = func_stm;
			//}
			| lvalue { $$ = $1;}
			| constant { 
				t_to_assign	c = buildPseudoVariableRep($1);
				t_assigned *t = malloc(sizeof(t_assigned));
				typeBuilder(c, 0, t);
				char *type_str = LLVMPrintTypeToString(t->type);
				LLVMValueRef val = getValue(c, *t, $1.val);
				printf("CONSTANT : Pointed-to type: %s \n", type_str);
				// Type c'est t->type
				$$ = new_node(NULL, val, t->type, T_CONST);
				// Checker que les type sont fine
				// On est une fonction donc c'est du big LLVMBuildGlobalString
				// Ou alloca au choix
			//	$$ = new_node(NULL, $1.val, /* int par default mais a changer*/CONST_INT);
			}
//				return cst;
// Build un const et return ca valueRef
// Checker son type
// Build et asigner la constant 
//            return LLVMConstInt(LLVMInt32Type(), v, 0);
//$$ = new_statement($1, CONST, NULL, NULL); }
			//| unary rvalue %prec UMINUS
			| lvalue assignation rvalue { /* ici on recupere $3 pour le mettre en rhs */
				handleAssignation($1, $3, builder, &current_scope->root_tree);
				$$ = $1;
				// CHEKER LVALUE DANS L'ARBRE EXTRN AUTO ? LABEL

//				AUTO tu check la ref si c'est null faut alloca comme label
//				LABEL c'est toujours allouer si on trouve la variable dans l'arbre

				//t_statement *lhs = $1;
				//t_statement *rhs = $3;
				//printf("wtf %p %i\n", rhs, rhs->kind);
				//$$ = new_statement(NULL, OP_ASSIGN, rhs, lhs);}
				// On a la ref dans lvalue
//				// Definir le type a droite
//				LLVMValueRef	content;
//				t_node_var	*lval = var_tree_find(&current_scope->root_tree, $1->name);
//				if (!lval) {
//					// C'est pour le label
//					content = LLVMBuildAlloca(builder, LLVMInt32Type(), $1->name);
//					$1->val.ref = content;
//					tsearch($1, &current_scope->root_tree, compare_var_name);
//					lval = $1;
//				} else {
//					content = lval->val.ref;// tmp = x
//				}
//				LLVMBuildStore(builder, $3->val.ref, content);                                      // x = 42;
//				$$ = lval;
//				if ($3->kind == CONST_OP) {
//					$1->val.ref = $3->val.ref;
//					$$ = $1;
//				}

				// Allocate i32 on stack
				// if var not in the tree
				//LLVMValueRef alloca_inst = LLVMBuildAlloca(builder, getTypeFromDef($3->kind), $1->name);

				printf("%i\n", $3->kind);
				//init = LLVMBuildGlobalStringPtr(value->val.str, )
//				LLVMValueRef loaded = LLVMBuildLoad2(builder, getTypeFromDef($3->kind), , "tmp");// tmp = x

			}
			//| inc_dec rvalue {/* $$ =  */}
			//| lvalue inc_dec
			//| '&' lvalue { /* Pour passer des addresses */ }
			| rvalue PLUS rvalue     {
				// Ici on va utiliser op, faut load la value si c'est une variable avant de l'envoyer dans le build
//				LLVMValueRef add = LLVMBuildAdd(builder, $1->val.ref, $3->val.ref, "add");
//				$$ = new_node("", (t_val){.ref=add}, CONST_OP);
				// Faut faire le do-op ici enft
			}
			| rvalue MINUS rvalue     {
				// op(builder, $1->val.ref, $3->val.ref, &LLVMBuildSub, name)

//				LLVMValueRef sub = LLVMBuildSub(builder, $1->val.ref, $3->val.ref, "sub");
//				$$ = new_node("", (t_val){.ref=sub}, CONST_OP);
			}
			//| rvalue MUL rvalue      {
			//	t_statement *lhs = $1; /* $$ =   */
			//	t_statement *rhs = $3;/* $$ =   */
			//	$$ = new_statement(NULL, OP_MUL, rhs, lhs);}
			//| rvalue DIV rvalue      {
			//	t_statement *lhs = $1; /* $$ =   */
			//	t_statement *rhs = $3;/* $$ =   */
			//	$$ = new_statement(NULL, OP_DIV, rhs, lhs);}
			//| rvalue MOD rvalue      {
			//	t_statement *lhs = $1; /* $$ =   */
			//	t_statement *rhs = $3;/* $$ =   */
			//	$$ = new_statement(NULL, OP_MOD, rhs, lhs);}
			//;

//| rvalue EQ rvalue       BINARY_OP(EQ)
//| rvalue NE rvalue       BINARY_OP(NE)
//| rvalue LT rvalue       BINARY_OP(LT)
//| rvalue GT rvalue       BINARY_OP(GT)
//| rvalue OR rvalue       BINARY_OP(OR)
//| rvalue AND rvalue      BINARY_OP(AND)
//			| rvalue binary rvalue {printf("OPERATION %i \n", $1->content->value->u.num/* ,$3->content->value->u.num */);

lvalue:
		IDENTIFIER {
			// Soit c'est déclaré dans ce cas
			// Check dans l'arbre si ya c'est ok
			// Sinon faut faire un label
			t_node_var *node = var_tree_find(&current_scope->root_tree, $1);
			if (!node) {
				node = new_node($1, NULL, NULL, T_LABEL);
				//t_node_var *new = new_node(strdup($1), (t_val){.ref=NULL}, CONST_VAR);
//				printf("%s %s\n", new->name, $1);
				//$$ = new;
			} else {
				printf("found %s\n", $1);
			}
			$$ = node;
	//		$$ = new_statement(new_node($1, (t_val){0}, CONST_VAR), VAR, NULL, NULL);
			// Géré les différences entre label, var-extrn et var auto plus tard
			// Peut etre rename en VAR tous cours
		}
//		| '*' rvalue { /* Pour déréférence des variables qui doivent stocker une ref ptr*/ }
//		| rvalue '[' rvalue ']' { /* Gestion des index d'un tableau */ } 
		

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
    : /* empty */   { $$ =(t_value){.val={.num=0}, .type=TYPE_INT} ;}
    | constant      { $$ = $1 }
	;

//opt_ival:		/* empty */
//				| ival

constant
    : NUMBER {
     { $$ =(t_value){.val={.num=$1}, .type=TYPE_INT};}
	}
    | CHARCONST  { 
     {
 $$ =(t_value){.val={.c=$1}, .type=TYPE_CHAR};}
	}
    | STRING  {
		//LLVMTypeRef array_type = LLVMArrayType(LLVMInt8Type(), strlen($1) + 1);
//		LLVMValueRef global = LLVMAddGlobal(module, array_type, "str");
//		LLVMSetInitializer(global, LLVMConstString("hello", 5, 0));
		//LLVMSetGlobalConstant(global, 0);  // MODIFIABLE (0 = pas constant)
//		LLVMValueRef str = LLVMConstString($1, strlen($1), 0);
//		LLVMValueRef arr = LLVMArrayType(LLVMInt8TypeInContext(context), strlen($1));
		// Array ?
		{ $$ =(t_value){.val={.str=$1}, .type=TYPE_STR}; }
		//$$ = new_node(NULL, (t_val){.str = $1}, CONST_STR);
//		printf("Before LLVMBuildGlobalString: %s %p, builder=%p\n", $1, $1, builder);
//		LLVMValueRef str = LLVMBuildGlobalString(builder, (const char *)$1, "str");
		// C'est une string, donc je cree un LLVMBuildGlobalStringPtr, builder, "Hello, world!", "str");
		// Et je stock la ref dans le node. 
	}

opt_number:
		/* empty */ { $$ = -1;}
		| NUMBER { $$ = (int)$1; }

//ival:	constant
//	| IDENTIFIER


%%


int main(void) {
	printf("Welcome to a little B compiler :) \n");
	g_funcs.head = NULL;
	ctx = LLVMContextCreate();
	mod = LLVMModuleCreateWithName("b");
	current_scope = NULL;
	create_scope();
	builder = LLVMCreateBuilderInContext(ctx);
//	printf("dEBUG: Current basic block = %p\n", current_bb);
	yyparse();

	LLVMContextDispose(ctx);
	return 1;
}

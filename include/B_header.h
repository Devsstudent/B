#ifndef B_HEADER_H
# define B_HEADER_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <search.h>
#include <string.h>
#include <llvm-c/Core.h>
#include <llvm-c/ExecutionEngine.h>
#include <llvm-c/Target.h>
#include <llvm-c/TargetMachine.h>
#include <llvm-c/Analysis.h>
#include <llvm-c/BitWriter.h>
#include <llvm-c/BitReader.h>
#include <llvm-c/Support.h>


struct s_array;

struct s_g_var_list;

typedef struct s_func {
	LLVMTypeRef	proto;
	char		*name;
	struct s_func *next;
}	t_func;

typedef struct s_funcs {
	t_func	*head;
}	t_funcs;

t_funcs	g_funcs;


// a modifier

typedef enum { T_AUTO, T_EXTRN, T_LABEL, T_CONST, T_GLOBAL } t_var_type;

typedef enum {T_IF, T_ELSE, T_WHILE, T_FUNC} t_scope_type;

typedef enum {
	TYPE_INT,
	TYPE_CHAR,
	TYPE_STR,
	TYPE_IDENTIFIER,
} t_value_type;

typedef enum { INT, CHAR, PTR } t_ttype;

typedef enum {CONST_INT, CONST_CHAR, CONST_STR, CONST_VAR, CONST_ARR, CONST_OP, CONST_VAR_AUTO, CONST_LABEL, CONST_EXTRN} t_const_kind;

//typedef enum {INT, CHAR, STR} t_type;

typedef union { char c; int num; char *str; LLVMValueRef ref; struct s_array *array;} t_val;

struct s_statement_list;

typedef struct {
	t_value_type	type;
	t_val			val;
}	t_value;

typedef struct s_value_list{
	t_value				elem;
	struct s_value_list *next;
}	t_value_list;

typedef struct {
//	t_val			val;
//	t_value			vall;
	t_const_kind	kind;
	bool			isArr;

	// The new stuct would more likely looks like
	// Structure qui va etre envoyé a assignation, pour permettre de genere le type, et la valueRef;

	char			*name;
	LLVMValueRef	val;
	LLVMTypeRef		type;
	LLVMTypeRef	baseType;
	int				ptrDepth;
	t_var_type		varType;
	// Ajouter un autre type peux etre pas mal

}	t_node_var;

// Structure qui va etre envoyé a assignation, pour permettre de genere le type, et la valueRef;
typedef struct s_to_assign {
	t_ttype				subjectiveType;
	t_var_type			defKeyword;
	struct s_to_assign	*ptr;
	int					size;
	int					layer;
}	t_to_assign;

typedef struct {
	char			*name;
	LLVMValueRef	val;
	LLVMTypeRef		type;
}	t_assigned;


typedef struct s_array {
	int				len;
	t_value_list		**values;
//	t_const_kind	kind;
}	t_array;

// J'hesite sur le nom ca pourrais presque etre block
typedef struct s_scope {
	t_scope_type		type;
//			**garbage;
	void				*root_tree;
//	LLVMBasicBlockRef	*block; // Necessaire le pointeur ?
	struct s_scope		*parent;
}	t_scope;

typedef struct s_param {
	int				len;
	char			**param;
}					t_param;

typedef enum {NEG, UNARY_NOT} t_unary;

typedef enum {
    /* Types de variables/constantes */
    VAR_EXTRN, VAR_AUTO, VAR, CONST, BOOL, RET, OP_ASSIGN,

    /* Opérateurs binaires */
    OP_OR, OP_ADD, OP_SUB, OP_MUL, OP_DIV, OP_MOD, 
    OP_BINARY_NOT, OP_INF, OP_SUP, OP_INF_EG, OP_SUP_EG, 
    OP_SHIFT_R, OP_SHIFT_L, OP_NOT_EG, OP_EG_EG, CALL
} t_kind;

typedef struct {
	char *name;
	t_node_var **params;
	int param_count;
	struct s_g_var_list **declared_vars;
	int declared_count;
}	t_func_definition;

typedef struct s_var_list {
	t_node_var			*elem;
	struct s_var_list	*next;
} t_var_list;

typedef struct s_g_var_list {
	t_node_var	*var;
	struct s_g_var_list	*next;
}	t_g_var_list;

typedef struct s_func_list {
	t_func_definition	*func;
	struct s_func_list	*next;
}	t_func_list;

typedef struct s_function_call {
	char				*name;
	struct s_statement_list	*head;
	int					nb_param;
}	t_function_call;

typedef struct s_statement{
	t_kind	kind;
	t_node_var	*content;
	struct s_statement	*lhs;
	struct s_statement	*rhs;
	t_function_call		*call;
}		t_statement;

typedef struct s_statement_elem {
	t_statement				*elem;
	struct s_statement_elem	*next;
}	t_statement_elem;


typedef struct s_statement_list {
	t_statement_elem	*head;
	int					size;
}	t_statement_list;



	void	pop_scope();
	void	create_scope();

	LLVMTypeRef	handleParam(t_param *params);

	bool	addGlobalVariable(t_node_var *def, LLVMModuleRef	mod);

	bool	handleAssignation(t_node_var *lhs, t_node_var *rhs, LLVMBuilderRef builder, void **root_tree);
	LLVMValueRef	op(LLVMBuilderRef builder, LLVMValueRef lhs, LLVMValueRef rhs, LLVMValueRef (*operation)(LLVMValueRef, LLVMValueRef, LLVMBuilderRef, char *), char *name);
	bool	handleRet(t_node_var *retNode, LLVMBuilderRef builder);

	LLVMValueRef getValue(t_to_assign toAss, t_assigned ass, t_val val);
	t_to_assign	buildPseudoVariableRep(t_value val);
	t_assigned	typeBuilder(t_to_assign pseudoType, int lays, /* il faut ajouter un parametre a la recursion pour stocker les infos intermediaire des pointeur*/ t_assigned *ass);
	t_value_list	**addToList(t_value val, t_value_list **list);
	bool	handleExternVars(t_value_list **names, void **current_tree, void **old_tree);
	bool	addAutoVarsToTree(t_value_list **names, void **root_tree);
	int	getDepth(t_to_assign var);
	t_node_var	*new_ptr(char *name, LLVMValueRef valRef, LLVMTypeRef type, LLVMTypeRef baseType, int depth);
#endif

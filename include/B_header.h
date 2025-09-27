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


typedef enum {CONST_INT, CONST_CHAR, CONST_STR, CONST_VAR, CONST_ARR} t_const_kind;


typedef union { int num; char *str; LLVMValueRef ref; struct s_array *array;} t_val;

struct s_statement_list;

typedef struct {
	t_val	val;
	t_const_kind	kind;
	char	*name;
}	t_node_var;

typedef struct s_var_list {
	t_node_var			*elem;
	struct s_var_list	*next;
} t_var_list;

typedef struct s_array {
	int				len;
	t_var_list		**values;
	t_const_kind	kind;
}	t_array;

typedef struct {
	char *name;
	t_node_var **params;
	int param_count;
	struct s_g_var_list **declared_vars;
	int declared_count;
}	t_func_definition;

typedef struct s_g_var_list {
	t_node_var	*var;
	struct s_g_var_list	*next;
}	t_g_var_list;

typedef struct s_func_list {
	t_func_definition	*func;
	struct s_func_list	*next;
}	t_func_list;


typedef enum {NEG, UNARY_NOT} t_unary;

typedef enum {
    /* Types de variables/constantes */
    VAR_EXTRN, VAR_AUTO, VAR, CONST, BOOL, RET, OP_ASSIGN,

    /* Opérateurs binaires */
    OP_OR, OP_ADD, OP_SUB, OP_MUL, OP_DIV, OP_MOD, 
    OP_BINARY_NOT, OP_INF, OP_SUP, OP_INF_EG, OP_SUP_EG, 
    OP_SHIFT_R, OP_SHIFT_L, OP_NOT_EG, OP_EG_EG, CALL
} t_kind;

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

typedef struct s_scope {
	t_statement_list	statements;
	struct s_scope		*parent;
}	t_scope;

typedef struct s_param {
	int				len;
	char			**param;
}					t_param;


	void	pop_scope();
	void	create_scope();


	LLVMTypeRef	handleParam(t_param *params);

	bool	addGlobalVariable(t_node_var *def, LLVMModuleRef	mod);



#endif

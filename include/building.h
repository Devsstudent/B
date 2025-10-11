#ifndef BUILDING_H
# define BUILDING_H

# include "B_header.h"

    void	generate_asm(LLVMModuleRef mod);
	void	handleStatements(t_statement_list *statements, LLVMValueRef function, LLVMBuilderRef builder, LLVMModuleRef mod);
    int	compare_var_name(const void *a, const void *b);
	LLVMValueRef	createValue(t_node_var *value, LLVMTypeRef type);
	LLVMTypeRef	getTypeFromDef(t_const_kind kind);

    void	print_var(const void *nodep, const VISIT which, const int depth);
    void	free_node(const void *nodep, const VISIT which, const int depth);
	bool	allocVariable(t_node_var *var, LLVMBuilderRef builder);
	bool	add_var(t_node_var *var, void **root_tree, LLVMValueRef *ref);
	bool	assign_ref_to_var(t_node_var *var, void **root_tree, LLVMValueRef *ref);

//	t_node_var *new_node(char *name, t_val val, t_const_kind kind);
    t_node_var *var_tree_find(void **root, const char *name);
	void	fillTypeFromKind(t_const_kind kind, t_node_var *var);

	t_node_var *new_node(char *name, LLVMValueRef valRef, LLVMTypeRef typeRef, t_var_type varType);

#endif

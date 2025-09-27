#ifndef PARSING_H
# define PARSING_H

# include "B_header.h"

t_g_var_list	**add_elem(t_g_var_list **head, t_node_var *def);
t_g_var_list	**add_many_elem(t_g_var_list **head, t_g_var_list **to_add);
t_statement *new_statement(t_node_var *def, t_kind kind, t_statement *rhs, t_statement *lhs);
bool	add_statement_elem_var(t_node_var *def, t_kind kind, t_statement_list *list);
t_statement_list *new_stm_list(void);
bool	add_statement_elem(t_statement *statement, t_statement_list *list);
bool	checkIfVarNameIsParamName(LLVMValueRef function, char *str);
t_var_list	**add_node(t_node_var *var, t_var_list **list);
t_param	*add_param(t_param *parameters, char *name);
t_array	*new_arr(int len, t_var_list **list);
bool	validArrayDeclaration(t_var_list **list, int len/* cmp function */);
void	adaptArraySize(t_array *arr/* cmp function */);
bool	checkElemArrayAllSameType(t_array *arr);

#endif

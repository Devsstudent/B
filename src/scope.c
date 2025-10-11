#include "parsing.h"


extern	t_scope	*current_scope;

void	create_scope() {
	t_scope *s = malloc(sizeof(t_scope));
	// Copie le tree du parent ici current_scope->tree = cpy_tree(&old->tree)
//	if (current_scope && current_scope->root_tree) {
//		s->root_tree = cpy_tree(current_scope->root_tree);
//	} else {
		s->root_tree = NULL;
///	}
	t_statement_list list;
	list.head = NULL;
	list.size = 0;
	//s->statements = list;
    s->parent = current_scope;
    current_scope = s;
}

void	pop_scope() {
	t_scope	*old = current_scope;

    current_scope = old->parent;
    // free vars if needed
//    clear_var_list(&(old->vars));
    free(old);
}

/*  gestion des variables dans le socope gloable (a revoir, priviligier le tsearch tree) */
t_g_var_list	**add_many_elem(t_g_var_list **head, t_g_var_list **to_add) {

	if (!to_add) {
		return NULL;
	}
	t_g_var_list *buff = *to_add;


// si !head : head= add_elem(head, buff->var)
	while (buff != NULL) {
		add_elem(head, buff->var);
		buff = buff->next;
	}
	return head;
}

/*  gestion des variables dans le socope gloable (a revoir, priviligier le tsearch tree) */
t_g_var_list	**add_elem(t_g_var_list **head, t_node_var *def) {
	t_g_var_list **list = head;

	if (!list) {
		list = malloc(sizeof(t_g_var_list*));
		*list = NULL;
	}
	if (!(*list)) {
		(*list) = malloc(sizeof(t_g_var_list));
		(*list)->var = def;
		(*list)->next = NULL;
		return list;
	}
	t_g_var_list	*elem = *list;
	while (elem->next) {
		elem = elem->next;
	}
	t_g_var_list *new_elem = malloc(sizeof(t_g_var_list));
	new_elem->var = def;
	new_elem->next = NULL;
	elem->next = new_elem;
	return list;
}

t_value_list	**addToList(t_value val, t_value_list **list) {
	if (!list) {
		list = malloc(sizeof(t_value_list*));
		*list = NULL;
	}
	if (!(*list)) {
		(*list) = malloc(sizeof(t_value_list));
		(*list)->elem = val;
		(*list)->next = NULL;
		return list;
	}
	t_value_list	*elem = *list;
	while (elem->next) {
		elem = elem->next;
	}
	t_value_list *new_elem = malloc(sizeof(t_value_list));
	new_elem->elem = val;
	new_elem->next = NULL;
	elem->next = new_elem;
	return list;
}


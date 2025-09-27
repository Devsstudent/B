#include "parsing.h"
#include "building.h"

t_statement_list *new_stm_list(void) {
	t_statement_list *new = malloc(sizeof(t_statement_list));
	if (!new)
		return NULL;
	new->size = 0;
	new->head = NULL;
	return new;
}


bool	add_statement_elem(t_statement *statement, t_statement_list *list) {
	t_statement_elem *new = malloc(sizeof(t_statement_elem));
	if (!new) {
		return false;
	}
	new->elem = statement;
	if (!new->elem) {
		return false;
	}
	new->next = NULL;
	
	t_statement_elem *buff = list->head;
	if (!buff) {
		list->head = new;
		return true;
	}
	while (buff && buff->next) {
		buff = buff->next;
	}
	buff->next = new;
	// LES CHANGEMENT DE KIND ETC 
	list->size += 1;
	return true;
}

bool	add_statement_elem_var(t_node_var *def, t_kind kind, t_statement_list *list) {
	t_statement_elem *new = malloc(sizeof(t_statement_elem));
	if (!new) {
		return false;
	}
	new->elem = new_statement(def, kind, NULL, NULL);
	if (!new->elem) {
		return false;
	}
	new->next = NULL;
	
	t_statement_elem *buff = list->head;
	if (!buff) {
		list->head = new;
		return true;
	}
	while (buff && buff->next) {
		buff = buff->next;
	}
	buff->next = new;
	list->size += 1;
	return true;
}

t_statement *new_statement(t_node_var *def, t_kind kind, t_statement *rhs, t_statement *lhs) {
	t_statement *new = malloc(sizeof(t_statement));
	if (!new) {
		printf("new statement crash\n");
		return NULL;
	}
	new->content = def;
	if (kind == CONST || kind == VAR_AUTO || kind == VAR_EXTRN) {
		new->lhs = new;
		new->rhs = NULL;
	} else {
		new->lhs = lhs;
		new->rhs = rhs;
	}
	new->kind = kind;
	return new;
}


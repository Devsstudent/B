#include "building.h"
#include "parsing.h"
#include "lexing.h"

void	handle_declaration(t_g_var_list	**head, t_statement_list *list, bool extrn) {
	if (!head) {
		return ;
	}
	t_g_var_list *buff = *head;
	while (buff) {
		add_statement_elem_var(buff->var, extrn ? VAR_EXTRN : VAR_AUTO, list);
		t_g_var_list *next = buff->next;
		free(buff);
		buff = next;
	}
}

void yyerror(const char *s) {
    fprintf(stderr, "Erreur: %s\n", s);
}

t_param *add_param(t_param *parameters, char *name) {
    t_param *new = parameters;

    if (parameters == NULL) {
        new = malloc(sizeof(t_param));
        new->len = 0;
        new->param = NULL;
    }

    // Allocate new array with space for the new string
    char **new_arr = malloc(sizeof(char *) * (new->len + 1));

    // Copy existing strings
    for (int i = 0; i < new->len; i++) {
        new_arr[i] = new->param[i];
    }

    // Add the new string
    new_arr[new->len] = strdup(name);

    // Free old array of pointers (not the strings themselves)
    if (new->param != NULL) {
        free(new->param);
    }

    new->param = new_arr;
    new->len++;

    return new;
}

t_var_list	**add_node(t_node_var *var, t_var_list **list) {
	t_var_list *buff;
	// Gérée les erreurs de malloc

	if (!list || !(*list)) {
		list = malloc(sizeof(t_var_list*));
		*list = malloc(sizeof(t_var_list));
		(*list)->elem = var;
		(*list)->next = NULL;
		return list;
	}

	buff = *list;

	while (buff->next) {
		buff = buff->next;
	}
	t_var_list *new = malloc(sizeof(t_var_list));
	new->elem = var;
	new->next = NULL;
	buff->next = new;
	return list;
}

t_array	*new_arr(int len, t_var_list **list) {
	t_array *arr = malloc(sizeof(t_array));
	arr->len = len;
	arr->values = list;

	return arr;
}

// Get la taill de la list
unsigned int getSizeVarList(t_var_list **list) {
	t_var_list		*buff;
	unsigned int	len;

	len = 0;

	if (!list || !(*list))
		return len;

	buff = *list;
	while (buff) {
		buff = buff->next;
		len += 1;
	}
	return len;
}

bool	validArrayDeclaration(t_var_list **list, int len/* cmp function */) {
	unsigned int len_list = getSizeVarList(list);
	if ((int) len_list > len) {
		return false;
	}
	return true;
}

static t_val	defineBasicVal(t_const_kind kind) {
	t_val	init;

	if (kind == CONST_STR) {
		init = (t_val){.str = NULL};
	} else if (kind == CONST_INT) {
		init = (t_val){.num = 0};
	} else {
		init = (t_val){.ref = NULL};
	}
	return init;
}

void	adaptArraySize(t_array *arr/* cmp function */) {
	unsigned int len_list = getSizeVarList(arr->values);
	t_const_kind type = arr->kind;
	t_val	init = defineBasicVal(type);
	
	if ((int) len_list < arr->len) {
		int i = len_list;
		while (i <= arr->len) {
			add_node(new_node(NULL, init, (*arr->values)->elem->kind), arr->values);
			i++;
		}
		// Fill avec des zeros le reste
	} else if (arr->len == -1) {
		arr->len = len_list;
	}
}




// function qui check le tmp_kind et le kind
//bool	checkSameType(t_const_kind kind, t_const_kind kind) {
//	if (kind != kind)
//		return false;
//	return true;
//}
//
//
//// function qui loop juste sur le array
//bool	loopArrayElem(t_var_list **list/* cmp function */) {
//
//	if (!list || !(*list))
//		return true;
//	t_var_list	*buff = *list;
//	while (buff) {
//		buff = buff->next;
//	}
//	return true;
//}

bool	checkElemArrayAllSameType(t_array *arr) {
	t_var_list *buff = *arr->values;
	t_const_kind buff_kind = -1;
	while (buff) {
		if ((int)buff_kind == -1) {
			buff_kind = buff->elem->kind;
		} else if (buff_kind != buff->elem->kind) {
			return false;
		}
		buff = buff->next;
	}
	// new len to set !
	return true;
}

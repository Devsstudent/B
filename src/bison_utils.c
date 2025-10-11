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

//bool	loop_var_list(t_g_var_list **list, void **root, LLVMValueRef *ref, bool (fn)(t_node_var *, void **, LLVMValueRef*)) {
//	t_g_var_list *buff;
//
//	if (!list || !(*list))
//		return false;
//	buff = *list;
//	while (buff) {
//		if (
//		if (!fn(buff->var, root, ref)) {
//			return false;
//		}
//		buff = buff->next;
//	}
//	return true;
//}

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

t_array	*new_arr(int len, t_value_list **list) {
	t_array *arr = malloc(sizeof(t_array));
	arr->len = len;
	arr->values = list;

	return arr;
}

// Get la taill de la list
unsigned int getSizeValList(t_value_list **list) {
	t_value_list		*buff;
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

bool	validArrayDeclaration(t_value_list **list, int len/* cmp function */) {
	unsigned int len_list = getSizeValList(list);
	if ((int) len_list > len && len != -1) {
		return false;
	}
	return true;
}

static t_val	defineBasicVal(t_value value) {
	t_val	init;

	if (value.type == TYPE_STR)  {
		init = (t_val){.str = ""};
	} else if (value.type == TYPE_CHAR) {
		init = (t_val){.c = '\0'};
	} else if (value.type == TYPE_INT) {
		init = (t_val){.num = 0};
	} else {
		init = (t_val){.ref = NULL};
	}

	return init;
}



void	adaptArraySize(t_array *arr/* cmp function */) {
	unsigned int len_list = getSizeValList(arr->values);
	// Ici elem ca peux etre une variable est dcp on est baisé quoi

	// Loop sur tous les elems pour prend le type et si c'est que des var faut faire une fonction pour LLVMtype to t_value_type

	t_val	init = defineBasicVal((*arr->values)->elem);

	if ((int) len_list < arr->len) {
		int i = len_list;
		while (i < arr->len) {
			addToList((t_value){.val=init, .type=(*arr->values)->elem.type}, arr->values);
			i++;
		}
		// Fill avec des zeros le reste
	} else if (arr->len == -1) {
		arr->len = len_list;
	}
}


// Ajouter des varibles facilement
// Utiliser pour les declaratoin auto, extern ou label + pour les params
//void	add_variable_to_tree(t_node_var *var, t_current_scope *scope) {
//	// var deja allouer donc on va l'ajouter dans l'arbr
//	// Le truc c'est qu'il faudrait ajouter aussi la reference de cette variable est l'initialiser
//	// Create value etc genre ou bien on l'as deja fait avant ? 
//	tsearch(var, &scope->root_var, compare_var_name);
//}
//
//void	assign_egal(char *varName, t_current_scope *scope, LLVMValueRef toAssign) {
//	// On repere le nom
//	t_node_var	*correspondingVar = var_tree_find(&scope->root_tree, varName);
//	correspondingVar->ref = toAssign;
//	// Checker si on a trouvé ou pas
//	// On récupere la ref qui correspond au resultat de l'operation
//}

/*
t_val	initRef() {

} */



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

//bool	checkElemArrayAllSameType(t_array *arr) {
//	t_value_list *buff = *arr->values;
//	t_const_kind buff_kind = -1;
//	while (buff) {
//		if ((int)buff_kind == -1) {
//			buff_kind = buff->elem->kind;
//		} else if (buff_kind != buff->elem->kind) {
//			return false;
//		}
//		buff = buff->next;
//	}
//	// new len to set !
//	return true;
//}

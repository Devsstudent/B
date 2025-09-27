#include "building.h"

#define PRINT_VAR(var) do { \
    if (_Generic((var->val.num), int: 1, default: 0)) \
        printf("var %s %i\n", var->name, var->val.num); \
    else if (_Generic((var->val.str), char *:1, default: 0)) \
    	printf("var %s %s\n", var->name, var->val.str); \
	else 				\
		printf("WTF\n"); \
} while(0)

int	compare_var_name(const void *a, const void *b) {
	const t_node_var *var_a = (const t_node_var *) a;
	const t_node_var *var_b = (const t_node_var *) b;

	if (var_a->name == NULL && var_b->name == NULL)
        return 0;    // both NULL, considered equal
    else if (var_a->name == NULL)
        return -1;   // NULL less than any string
    else if (var_b->name == NULL)
        return 1;    // any string greater than NULL
    else
        return strcmp(var_a->name, var_b->name);
}

void	print_var(const void *nodep, const VISIT which, const int depth) {
	t_node_var	*var;
	(void) depth;

	switch (which) {
		case postorder:
		case leaf:
			var = *(t_node_var **)nodep;
			PRINT_VAR(var);
			break ;
		case preorder:
		case endorder:
			break ;
	}
}

t_node_var *var_tree_find(void **root, const char *name) {
    t_node_var key = {0};
    key.name = (char*)name;          // pas besoin d'allouer
    void **pp = tfind(&key, root, compare_var_name);
    return pp ? *(t_node_var**)pp : NULL;
}

void free_node(const void *nodep, const VISIT which, const int depth) {
	(void) depth;
    if (which == postorder || which == leaf) {
        const void *key = *(const void **)nodep;
        // free key if dynamically allocated
        free((void *)key);
    }
}

t_node_var *new_node(char *name, t_val val, t_const_kind kind) {
	t_node_var *new = malloc(sizeof(t_node_var));
	if (!new)
		return NULL;
	new->name = name;
	new->val = val;
	new->kind = kind;
	return new;
}

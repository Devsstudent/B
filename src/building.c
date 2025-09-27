#include "building.h"
#include "parsing.h"

static int get_const_int(const t_node_var *def, int *out) {
    if (!def) return 0;
    if (def->kind != CONST_INT) return 0;
    *out = def->val.num;
    return 1;
}

static LLVMValueRef do_op(const t_statement *s, void **root, LLVMBuilderRef b) {
    if (!s) return NULL;

    switch (s->kind) {
    case CONST: {
		printf("const\n");
	 	int v;
        if (get_const_int(s->content, &v)) {
            return LLVMConstInt(LLVMInt32Type(), v, 0);
        }
        return NULL;
    }
	// Ici c'est var_auto, var_extern, et aussi label
    case VAR: {
		t_node_var *v = var_tree_find(root, s->content->name);
		// Return pour faire des instruction ??
		int va = 0;

    	if (_Generic((v->val.ref), LLVMValueRef:1, default: 0)) {
//			printf("generic %i %s\n", v->val.num, v->name);
			if (!v->val.ref) {
                return LLVMConstInt(LLVMInt32Type(), va, 0);
			}
			return v->val.ref;
		} else  if (get_const_int(s->content, &va)) {
                return LLVMConstInt(LLVMInt32Type(), va, 0);
        }
       return NULL;
	}
//    case VAR_EXTRN: {
//    //    /* Only accept variables that already hold a CONST_INT */
//		t_node_var *v = var_tree_find(root, s->content->name);
//		return v->val.ref;
//    }
    case OP_ADD: {
		LLVMValueRef l = s->lhs ? do_op(s->lhs, root, b) : LLVMConstInt(LLVMInt32Type(), 0, 0);
		LLVMValueRef r = s->rhs ? do_op(s->rhs, root, b) : LLVMConstInt(LLVMInt32Type(), 0, 0);
		return LLVMBuildAdd(b, l, r, "add");
    }
    case OP_SUB: {
		LLVMValueRef l = s->lhs ? do_op(s->lhs, root, b) : LLVMConstInt(LLVMInt32Type(), 0, 0);
		LLVMValueRef r = s->rhs ? do_op(s->rhs, root, b) : LLVMConstInt(LLVMInt32Type(), 0, 0);
		return LLVMBuildSub(b, l, r, "sub");
	}
    case OP_MUL: {
		LLVMValueRef l = s->lhs ? do_op(s->lhs, root, b) : LLVMConstInt(LLVMInt32Type(), 0, 0);
		LLVMValueRef r = s->rhs ? do_op(s->rhs, root, b) : LLVMConstInt(LLVMInt32Type(), 0, 0);
		return LLVMBuildMul(b, l, r, "mul");
    }
    case OP_DIV: {
		LLVMValueRef l = s->lhs ? do_op(s->lhs, root, b) : LLVMConstInt(LLVMInt32Type(), 0, 0);
		LLVMValueRef r = s->rhs ? do_op(s->rhs, root, b) : LLVMConstInt(LLVMInt32Type(), 0, 0);
		return LLVMBuildUDiv(b, l, r, "div");
    }
    case OP_MOD: {
		LLVMValueRef l = s->lhs ? do_op(s->lhs, root, b) : LLVMConstInt(LLVMInt32Type(), 0, 0);
		LLVMValueRef r = s->rhs ? do_op(s->rhs, root, b) : LLVMConstInt(LLVMInt32Type(), 0, 0);
		return LLVMBuildURem(b, l, r, "mod");
    }
    default:
        return NULL; /* unsupported kinds yield 0 */
	}
}

static void	addParametersToTree(void **root, LLVMValueRef function) {
	unsigned param_count = LLVMCountParams(function);
	for (unsigned i = 0; i < param_count; i++) {
		LLVMValueRef param = LLVMGetParam(function, i);
		char *name = (char *)LLVMGetValueName(param);
		tsearch(new_node(name, (t_val){.ref = param}, CONST_VAR), root, compare_var_name);
	}
}
static void handleRet(t_statement *return_stm, LLVMBuilderRef builder, void **root_tree) {
	// Vérifier si le basic block a déjà un terminator
	//LLVMBasicBlockRef current_bb = LLVMGetInsertBlock(builder);
	//if (LLVMGetBasicBlockTerminator(current_bb) != NULL) {
	//	printf("Basic block already has terminator, skipping return\n");
	//	return;
	//}

	int mask = OP_ADD | OP_OR | OP_ADD| OP_SUB| OP_MUL| OP_DIV| OP_MOD| OP_BINARY_NOT| OP_INF| OP_SUP| OP_INF_EG| OP_SUP_EG| OP_SHIFT_R| OP_SHIFT_L| OP_NOT_EG| OP_EG_EG;

	if (return_stm->rhs && (return_stm->rhs->kind & mask) != 0) {
		//	printf("VYAAO\n");
		LLVMValueRef res = do_op(return_stm->rhs, root_tree, builder);
		LLVMTypeRef void_ptr_type = LLVMPointerType(LLVMVoidType(), 0);
		LLVMValueRef int_ptr = LLVMBuildIntToPtr(builder, res, void_ptr_type, "inttoptr");
		LLVMValueRef ptr_as_void = LLVMBuildBitCast(builder, int_ptr, void_ptr_type, "cast_to_voidptr");
		LLVMBuildRet(builder, ptr_as_void);
	} else if (return_stm->rhs && return_stm->rhs->content && return_stm->rhs->content->name) {
		t_node_var *found = var_tree_find(root_tree, return_stm->rhs->content->name);
		LLVMTypeRef void_ptr_type = LLVMPointerType(LLVMVoidType(), 0);
		LLVMValueRef void_ptr = LLVMBuildIntToPtr(builder, found->val.ref, void_ptr_type, "inttoptr");
		LLVMBuildRet(builder, void_ptr);
	} else {
		LLVMTypeRef void_ptr_type = LLVMPointerType(LLVMVoidType(), 0);
		LLVMValueRef null_void_ptr = LLVMConstNull(void_ptr_type);
		LLVMBuildRet(builder, null_void_ptr);
	}
}

LLVMValueRef	handleCall(t_statement *call_stm, LLVMModuleRef mod, void **var_tree, LLVMBuilderRef builder) {
	t_statement_list *list = call_stm->call->head;

	// Recuperer la function et verifier qu'elle existe
	LLVMValueRef	func = LLVMGetNamedFunction(mod, call_stm->call->name);
	if (!func)
		return func;

	LLVMTypeRef func_type = LLVMGetElementType(LLVMTypeOf(func));

	t_func *buf = g_funcs.head;
			printf("PROOOOTOO %p\n", buf);
	while (buf) {
		if (strcmp(buf->name, call_stm->call->name) == 0) {
			func_type = buf->proto;
			printf("PROOOOTOO\n");
			break ;
		}
		buf = buf->next;
	}

	
	unsigned param_count = LLVMCountParamTypes(func_type);
//= call_stm->call->nb_param;
	// Preparer les arguemnts de la function
	LLVMValueRef args[param_count];
	
	
	int mask = OP_ADD | OP_OR | OP_ADD| OP_SUB| OP_MUL| OP_DIV| OP_MOD| OP_BINARY_NOT| OP_INF| OP_SUP| OP_INF_EG| OP_SUP_EG| OP_SHIFT_R| OP_SHIFT_L| OP_NOT_EG| OP_EG_EG;

	t_statement_elem	*buff = list->head;
	int i = 0;
	LLVMValueRef ref = NULL;

	while (buff) {
		ref = NULL;
		if (buff->elem->kind == CALL) {
			ref = handleCall(buff->elem, mod, var_tree, builder);
		} else if (buff->elem->kind == VAR_AUTO || buff->elem->kind == VAR_EXTRN) {
			t_node_var	*found = var_tree_find(var_tree, buff->elem->content->name); 
			ref = found->val.ref;
		} else if  (buff->elem->kind == CONST) {
			ref = createValue(buff->elem->content, getTypeFromDef(buff->elem->content->kind));
		} else if ((buff->elem->kind & mask) != 0 ) {
			ref = do_op(buff->elem, var_tree, builder);
		}
		printf("%i\n", i);
		args[i] = ref;
		i++;
		buff = buff->next;
	}
	return LLVMBuildCall2(builder, func_type, func, args, param_count, "call");
}

void	handleStatements(t_statement_list *statements, LLVMValueRef function, LLVMBuilderRef builder, LLVMModuleRef mod) {
	t_statement_elem	*buff = statements->head;
	void *root_var = NULL;

	addParametersToTree(&root_var, function);

	while (buff) {
		if (buff->elem->kind == CALL) {
			printf("CALL\n");
			/* LLVMValueRef ref = */ handleCall(buff->elem, mod, &root_var, builder);
			// Handle function call qui ferait les do_ophs, les ref est et puis ajouterais le call avec le builder
//			handleStatements(buff->elem->call->head, function, builder);
		}
		if (buff->elem->kind == RET) {
			// Check si ca return une variable ou une constant etc ?	
			//printf("VYAAO %i\n", buff->elem->rhs->kind);
			handleRet(buff->elem, builder, &root_var);
			return ;
			twalk(root_var, print_var);
			twalk(root_var, free_node);
            // Fonction pour cree le pointer de retour en utils TO DOOO
            // Case ou le return est un calcul ?? a gérer besoin d'un COMBO RET et OP_ADD par exemple
		}
		if (buff->elem->kind == VAR_AUTO || buff->elem->kind == VAR_EXTRN) {
			char *name = buff->elem->content->name;
			if (checkIfVarNameIsParamName(function, name)) {
				// Erreur local variable meme nom que param;
//				printf("contniue 1\n");
				continue ;
			}
			if (buff->elem->kind == VAR_AUTO) {
				printf("AUTO %s\n", name);
                // ALLOUE 
				t_node_var	*new = new_node(name, (t_val){.ref=NULL}, CONST_VAR);
				// AJOUTE A L'ARBRE
				/*void **result = */tsearch(new, &root_var, compare_var_name);
			} else if (buff->elem->kind == VAR_EXTRN) {
				printf("EXTRN %s\n", name);
				LLVMValueRef extrn_var = LLVMGetNamedGlobal(mod, name);
				if (!extrn_var) /* Handle erreur n'existe pas */
					continue ;
				t_node_var	*new = new_node(name, (t_val){.ref=extrn_var}, CONST_VAR);
/*void **result = */tsearch(new, &root_var, compare_var_name);
			}
		}
		if (buff->elem->kind == OP_ASSIGN) {

			printf("OP ASSIGN\n");
            //twalk(root_var, print_var);
            // On a un assignement donc a gauche c'est forcement le nom d'une variable.
			char *name= buff->elem->lhs->content->name;
			t_node_var *found = var_tree_find(&root_var, name);
			if (!found) { // C'est un label
				t_node_var	*new = new_node(name, (t_val){.ref=NULL}, CONST_VAR);
				tsearch(new, &root_var, compare_var_name);
				found = var_tree_find(&root_var, name);
			}
			LLVMValueRef ref = NULL;
			if (buff->elem->rhs->kind == CALL) {
				ref = handleCall(buff->elem->rhs, mod, &root_var, builder);
			} else {
				ref = do_op(buff->elem->rhs, &root_var, builder);
			}
			// Gestion du type ? TODOO
	        // en gros je stock la reference sur le calcul computed
/*LLVMValueRef calc =*/
			found->val.ref = ref;
			//addLocalVariable(found, builder, LLVMInt32Type());
		}
		buff = buff->next;
	}
	twalk(root_var, print_var);
	twalk(root_var, free_node);
}

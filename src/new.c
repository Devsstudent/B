#include "B_header.h"
#include "building.h"
#include "parsing.h"

extern	LLVMContextRef ctx;
extern	LLVMBuilderRef builder;
// Il faut un builder de type, avec une option PTR

LLVMTypeRef		buildType(t_const_kind kind, bool	wantPtr) {
	LLVMTypeRef type = getTypeFromDef(kind);
	if (wantPtr) {
		type = LLVMPointerType(type, 0);
	}
	return type;
}


// Une clone fonction pour attribuer le bon prototype


// Pour les alloca extern dans les function




// Get a pointer of globale value
LLVMValueRef	getPtrOnGlobalVar(char *name, LLVMModuleRef mod) {
	return LLVMGetNamedGlobal(mod, name);
}

// Load value
LLVMValueRef	loadFromPtr(LLVMBuilderRef builder, char *name, LLVMValueRef ptrRef,  LLVMTypeRef type) {
	return LLVMBuildLoad2(builder, type /* of the deferenced pointer */, ptrRef, name);
}

// Allocate locally
LLVMValueRef	allocOnStack(LLVMBuilderRef builder, LLVMTypeRef type, char *name) {
	return LLVMBuildAlloca(builder, type, name);
}

//Ajouter le build store ? 

LLVMValueRef	op(LLVMBuilderRef builder, LLVMValueRef lhs, LLVMValueRef rhs, LLVMValueRef (*operation)(LLVMValueRef, LLVMValueRef, LLVMBuilderRef, char *), char *name) {
	// Apres on peux géré aussi avec un node en parametre etc
	return operation(lhs, rhs, builder, name);
}


// En vrai function ou on peux pas alloca pck les auto c'est alloca au moment de leurs utilisation.
bool	addAutoVarsToTree(t_value_list **names, void **root_tree) {
	t_value_list *buff;

	if (!names || !*names)
		return false;
	buff = *names;
	while (buff) {
		char *name = buff->elem.val.str;
		t_node_var	*new = new_node(name, NULL, NULL, T_AUTO);
		void **result = tsearch(new, root_tree, compare_var_name);
		if (!result)
			return false;
		buff = buff->next;
	}
	return true;
}

bool	handleExternVars(t_value_list **names, void **current_tree, void **old_tree) {
	t_value_list *buff;
	LLVMValueRef extrn_var;
	t_node_var	*found;
	t_node_var	*new;
	LLVMTypeRef type;
	char		*name;
	LLVMValueRef ptr;

	if (!names || !*names)
		return false;

	buff = *names;
	while (buff) {
		// Functoin get name from t_value avec le check identifier et recupe le val.str
		name = buff->elem.val.str;

		found = var_tree_find(old_tree, name);
		printf(" handle extren %p %p\n", found, old_tree);
		char *type_str = LLVMPrintTypeToString(found->type);
		printf("Pointed-to type: %s \n", type_str);
		type = found->type;
		if (!found) {
			return false;
		}
		ptr = found->val;
		if (type == LLVMInt32Type()) {
			extrn_var = LLVMBuildLoad2(builder, LLVMInt32Type(), ptr, found->name);
		} else if (type == LLVMInt8Type()) {
			extrn_var = LLVMBuildLoad2(builder, LLVMInt8Type(), ptr, found->name);
		} else if (found->ptrDepth > 0){
			if (found->ptrDepth == 1) {
				LLVMValueRef zero = LLVMConstInt(LLVMInt32Type(), 0, false);
				LLVMValueRef indices[2] = { zero, zero };
				ptr = LLVMBuildInBoundsGEP2(builder, type, found->val, indices, 2, "ptr");
				LLVMValueRef x = LLVMBuildLoad2(builder, found->baseType, ptr, found->name);
				extrn_var = x;
			} else {
				LLVMValueRef zero = LLVMConstInt(LLVMInt32Type(), 0, false);
				LLVMValueRef indices[2] = { zero, zero };
				ptr = LLVMBuildInBoundsGEP2(builder, type, found->val, indices, 2, "ptr");
				LLVMValueRef x = LLVMBuildLoad2(builder, LLVMPointerType(LLVMPointerType(found->baseType, 0), 0), ptr, found->name);
				type_str = LLVMPrintTypeToString(found->baseType);
				extrn_var = x;
			}
			// C'est si c'est du pointer simple genre int array our char **
		}
		new = new_node(name, extrn_var, type, T_EXTRN);
		tsearch(new, current_tree, compare_var_name);

		// Vérifier que on pas deja une variable avec ce nom ? 

		buff = buff->next;
	}
	return true;
}

static inline bool	varIsAutoOrLabel(t_node_var *var) {
	return (!var || (var->varType == T_AUTO || var->varType == T_LABEL));
}

bool	handleAssignation(t_node_var *lhs, t_node_var *rhs, LLVMBuilderRef builder, void **root_tree) {
	// Alloca les labels et les auto
	// Store dans les ptr les resulat 

	t_node_var	*var;
	printf("this is %p \n", lhs);
	var = var_tree_find(root_tree, lhs->name);
	if (!var || varIsAutoOrLabel(var)) {

			// Probleme si a droite j'ai une addition ? faudra que je dise que c'est forcement un int16 en gros
		printf("this is %s \n", lhs->name);
		lhs->val = LLVMBuildAlloca(builder, rhs->type, lhs->name);
		// FAut aussi set le type
	} else { /* EXTRN */
		// Ajouter un check sur le var pour savoir si il est pas null etc
		// Dans ce cas la c'est un extrn donc on a un pointer on veux store dans ce pointer
		lhs->val = var->val;
	//	lhs->type = var->type; ??
	}
	LLVMBuildStore(builder, rhs->val, lhs->val);
	lhs->type = rhs->type;
//				AUTO tu check la ref si c'est null faut alloca comme label
//				LABEL c'est toujours allouer si on trouve la variable dans l'arbre
	return (true);
}

bool handleRet(t_node_var *retNode, LLVMBuilderRef builder) {
    LLVMTypeRef i32_ptr_type = LLVMPointerType(LLVMInt32Type(), 0);
		printf("return void\n");
		LLVMValueRef v = LLVMConstNull(LLVMPointerType(LLVMVoidType(), 0));

		// convert null pointer to i32 integer
		LLVMValueRef intVal = LLVMBuildPtrToInt(builder, v, LLVMInt32Type(), "ptr_to_int");
		LLVMBuildRet(builder, intVal);
		return true;
    if (retNode->varType == T_EXTRN || retNode->varType == T_AUTO || retNode->varType == T_LABEL) {
        if (retNode->type == LLVMInt32Type()) {
            // Retourner un int : convertir en pointeur
            LLVMBuildRet(builder, loadFromPtr(builder, retNode->name, retNode->val, retNode->type));
			return true;
            
        } else {
            // Retourner une string : GEP puis cast vers i32*
            LLVMValueRef zero = LLVMConstInt(LLVMInt32Type(), 0, 0);
            LLVMValueRef indices[] = { zero, zero };
            LLVMTypeRef global_type = LLVMGlobalGetValueType(retNode->val);
			//char *type_str = LLVMPrintTypeToString(type);
			
			//// Print using printf
			//printf("Pointed-to type: %s %i\n", type_str, buff->var->kind);
            
            // GEP pour obtenir i8*
            LLVMValueRef i8_ptr = LLVMBuildInBoundsGEP2(
                builder,
                global_type,
                retNode->val,
                indices,
                2,
                "str_ptr"
            );
            
            // Cast i8* vers i32*
            LLVMValueRef i32_ptr = LLVMBuildBitCast(
                builder, 
                i8_ptr, 
                i32_ptr_type, 
                "str_as_i32ptr"
            );
            
            LLVMBuildRet(builder, i32_ptr);
        }
    } else {
		printf("return void\n");
		LLVMValueRef v = LLVMConstNull(LLVMPointerType(LLVMVoidType(), 0));
		LLVMTypeRef i32PtrType = LLVMPointerType(LLVMInt32Type(), 0);
		LLVMValueRef i32Ptr = LLVMBuildBitCast(builder, v, i32PtrType, "cast_to_i32ptr");
		LLVMBuildRet(builder, i32Ptr);
	}
    return true;
}

// Nom de la function a revoir
//void	fillTypeFromKind(t_const_kind kind, t_node_var *var) {
//	if (kind == CONST_CHAR) {
//		var->type = LLVMInt8Type();
//	} else if (kind == CONST_STR) {
//		var->type = LLVMInt8Type();
////		var->isPtr = true;
//	} else if (kind == CONST_INT || kind == CONST_OP) {
//		var->type = LLVMInt32Type();
//	} else if (kind == CONST_ARR) {
//		// Definir si c'est du char string ou int
//		
//	} else {
//		var->type = LLVMInt8Type();
//	}
//}


//
//LLVMTypeRef	getTypeFromNode(t_node_var *node) {
//	// Get type from type
//	LLVMTypeRef	node->type;
//	if isArr
//		type = LLVMArrayType(type, 0);
//	return  type
//}

#include "B_header.h"

//t_to_assign	peudoTypeBuilder(t_node_var *var) {
//	if (var->kind == CONST_INT) {
//		return ((t_to_assign) {.subjectiveType = INT, .defKeyword=LABEL, .ptr=NULL, .size = 0, .layer = 0});
//	}
//	return ((t_to_assign) {.subjectiveType = INT, .defKeyword=LABEL, .ptr=NULL, .size = 0, .layer = 0});
////	return 
//}
extern LLVMModuleRef mod;

inline static LLVMTypeRef	getLllvmType(t_ttype kind) {
	if (kind == CHAR) {
		return	LLVMInt8Type();
	} else if (kind == INT) {
		return	LLVMInt32Type();
	}
	return LLVMVoidType();
}

// C'est ce qui va etre push dans l'arbre de la function local

// Definition du typeRef
t_assigned	typeBuilder(t_to_assign pseudoType, int lays, /* il faut ajouter un parametre a la recursion pour stocker les infos intermediaire des pointeur*/ t_assigned *ass) {
	if (pseudoType.ptr != NULL || lays != 0) {
		if (pseudoType.layer == 0) {
			ass->type = getLllvmType(pseudoType.subjectiveType);
		} else {
//			LLVMTypeRef ptr = LLVMPointerType(ass->type, 0);
			if (pseudoType.size == 0) {
				printf("typeBuilder, size of array 0 for %s\n", ass->name);
			}
			ass->type = LLVMArrayType(ass->type, pseudoType.size);
		}
		
		return pseudoType.ptr ? typeBuilder(*pseudoType.ptr, lays + 1, ass) : *ass;
	}

	// géré le cas des lays en 1er
	if (lays != 0) {
		LLVMTypeRef ptr = LLVMPointerType(ass->type, 0);
		ass->type = LLVMArrayType(ptr, pseudoType.size);
		// On est sur un tableau lays nous donne la profondeur
	} else {
		ass->type = getLllvmType(pseudoType.subjectiveType);
	}
	return *ass;
}

// Separer en deux la definition du type, et l'initialization de la variable

// On imagine qu'on a deja déclaré le t_to_assign et le type
LLVMValueRef getValue(t_to_assign toAss, t_assigned ass, t_val val) {
	(void) ass;
	if (toAss.ptr && toAss.subjectiveType == CHAR) {
		LLVMValueRef str = LLVMConstString(val.str, strlen(val.str), 0);
		return str;
	} else if (toAss.subjectiveType == INT) {
		return LLVMConstInt(LLVMInt32Type(), val.num, false);
	} else if (toAss.subjectiveType == CHAR) {
		return LLVMConstInt(LLVMInt8Type(), val.c, false);
	} else {
		return NULL;
	}
}

t_to_assign	newPseudoVarRep(t_ttype type, t_var_type var_type, t_to_assign *ptr, int size, int layer) {
	t_to_assign new = {.subjectiveType = type, .defKeyword=var_type, .ptr = ptr, .size = size, .layer = layer};
	return new;
}

int	getDepth(t_to_assign var) {
	int lay = 0;
	t_to_assign *buff = &var;
	while (buff->ptr) {
		buff = buff->ptr;
		lay += 1;
	}
	return lay;
}

t_to_assign	buildPseudoVariableRep(t_value val) {
	t_to_assign	glob; // Pseudo Representation de la variable

	if (val.type == TYPE_INT) {
		glob = newPseudoVarRep(INT, T_LABEL, NULL, 0, 0);
    } else if (val.type == TYPE_CHAR) {
		glob = newPseudoVarRep(CHAR, T_LABEL, NULL, 0, 0);
	} else if (val.type == TYPE_STR) {
		t_to_assign *ptrVar = malloc(sizeof(t_to_assign));
		*ptrVar = newPseudoVarRep(PTR, T_LABEL, NULL, strlen(val.val.str) + 1, 1);
//		printf("%lu\n", strlen(val.val.str));
		glob = newPseudoVarRep(CHAR, T_LABEL, ptrVar, 0, 0);
	} else {
		glob = (t_to_assign) {.subjectiveType = INT, .defKeyword=T_LABEL, .ptr=NULL, .size = 0, .layer = 0};
	}
	return glob;
}


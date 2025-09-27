#include "building.h"

LLVMTypeRef	getTypeFromDef(t_const_kind kind) {
	if (kind == CONST_CHAR) {
		return	LLVMInt8Type();
	} else if (kind == CONST_STR) {
		return LLVMPointerType(LLVMInt8Type(), 0);
	} else if (kind == CONST_INT) {
//		printf("INT\n");
		return	LLVMInt32Type();
	}
	return LLVMVoidType();
}

bool	checkIfVarNameIsParamName(LLVMValueRef function, char *str) {
		unsigned param_count = LLVMCountParams(function);
	for (unsigned i = 0; i < param_count; ++i) {
	    LLVMValueRef param = LLVMGetParam(function, i);
	    const char* pname = LLVMGetValueName(param);
	    // Compare pname with your desired name:
	    if (strcmp(pname, str) == 0) {
			return true;
	        // Found the parameter by name!
	    }
	}
	return false;
}

void	generate_asm(LLVMModuleRef mod) {
	// Our 4 context initializer to know which instruction to setup

	// For the architecture
	LLVMInitializeX86Target();

	// Load info of the architecture
	LLVMInitializeX86TargetInfo();

	// About the machine code of the target
	LLVMInitializeX86TargetMC();

	// Pour afficher de l'asm.
	LLVMInitializeX86AsmPrinter();


	// We set the tripleTarget of our architecture
	const char *targetTriple = "i386-pc-linux-gnu";
	LLVMSetTarget(mod, targetTriple);

	char			*error = NULL;
	LLVMTargetRef	target;

	// Find the corresping target type
	if (LLVMGetTargetFromTriple(targetTriple, &target, &error) != 0) {
		fprintf(stderr, "Error getting target: %s\n", error);
		LLVMDisposeMessage(error);
		return ;
	}
	// We create the reference to the targetMachine 
	LLVMTargetMachineRef targetMachine = LLVMCreateTargetMachine(target, targetTriple, "generic", "", LLVMCodeGenLevelDefault, LLVMRelocDefault, LLVMCodeModelDefault);

	// Then we set the targetTriple details to the module
	LLVMSetTarget(mod, targetTriple);
	
	LLVMMemoryBufferRef buf = NULL;
	LLVMTargetMachineEmitToMemoryBuffer(targetMachine, mod, LLVMAssemblyFile, &error, &buf);
	const char *output_asm = LLVMGetBufferStart(buf);
	printf("output %s\n", output_asm);
}

LLVMTypeRef	handleParam(t_param *params) {

	LLVMTypeRef *param_types = NULL;
	int param_count = 0;
	int i = 0;

	while (i < params->len) {
	    //char *name = params->param[i];
//	    while (statements[j] != NULL) {
//	        t_statement *statement = statements[j];
//			printf("aa %s %s\n", statement->content->name, name);
//	        if (strcmp(statement->content->name, name) == 0) {
	            // Grow the array by 1
	            LLVMTypeRef *tmp = realloc(param_types, sizeof(LLVMTypeRef) * (param_count + 1));
	            if (!tmp) {
	                // realloc failed -> free old array and handle error
	                free(param_types);
	                return NULL;
	            }
	            param_types = tmp;
	
	            // Push new type
				
	            param_types[param_count] = /* some LLVMTypeRef based on statement */ getTypeFromDef(CONST_INT);
	            param_count++;
	    i++;
	   }
	printf("%i parameters\n", params->len);
	LLVMTypeRef		proto = LLVMFunctionType(LLVMPointerType(LLVMVoidType(), 0), param_types, params->len, 0);
	
	return proto;
}

LLVMValueRef	createValue(t_node_var *value, LLVMTypeRef type) {
				LLVMValueRef init = NULL;

	if (type == LLVMInt32Type()) {
		init = LLVMConstInt(LLVMInt32Type(), value->val.num, 0);
	} else if (type == LLVMInt8Type()) {
		init = LLVMConstInt(LLVMInt8Type(), (char) value->val.num, 0);
	} else if (type == LLVMPointerType(LLVMInt8Type(), 0)) {
		if (value->val.str != 0) {
			printf("%p\n", value->val.str);
			init = LLVMConstString(value->val.str, strlen(value->val.str) + 1, 1);
		} else {
			init = LLVMConstString("", 1, 1);
		}
	}
	return init;
}

bool	assignToGlobalVar(t_node_var *value, LLVMTypeRef type,LLVMModuleRef	mod) {
	LLVMValueRef init = createValue(value, type);
	if (!init) {
		return false;
	}
	LLVMValueRef	global = LLVMAddGlobal(mod, LLVMTypeOf(init), value->name);
	if (global == NULL) {
		printf("Variable already exist.\n");
		return false;
	}
	LLVMSetInitializer(global, init);
	return true;
}

bool	addLocalVariable(t_node_var *var, LLVMBuilderRef builder, LLVMTypeRef type) {
	// Create an alloca for a 32-bit integer variable named "var"
	LLVMValueRef var_alloca = LLVMBuildAlloca(builder, type, var->name);
//
	// Optionally initialize with a value like 42
	LLVMValueRef init_val = LLVMConstInt(type, var->val.num, /* depending signed */0);
	LLVMBuildStore(builder, init_val, var_alloca);
	return true;
};

bool	addGlobalVariable(t_node_var *def, LLVMModuleRef	mod) {
	LLVMTypeRef		type;
 
	if (!def) 
		return false;
	// if c'est un tableau
	if (def->kind == CONST_ARR) {
		t_var_list **head = def->val.array->values;
		if (!head || !(*head)) {
			
		} else {
			type = getTypeFromDef((*head)->elem->kind);
			LLVMTypeRef arrayType = LLVMArrayType(type, def->val.array->len);
			// Surement mieux de malloc
			LLVMValueRef elements[def->val.array->len];
			t_var_list *buff = *head;
			int i = 0;
			while (buff) {
				elements[i] = createValue(buff->elem, type);
				buff = buff->next;
				i++;
			}
			LLVMValueRef arrayVal = LLVMConstArray(arrayType, elements, def->val.array->len);
			LLVMValueRef globalArray = LLVMAddGlobal(mod, arrayType, "arr");
			LLVMSetInitializer(globalArray, arrayVal);
			return true;
		}
		// Cree le global array
	}
	type = getTypeFromDef(def->kind);

	return assignToGlobalVar(def, type, mod);
}

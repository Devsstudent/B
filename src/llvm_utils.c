#include "building.h"

extern	t_scope	*current_scope;
extern	LLVMContextRef ctx;

LLVMTypeRef	getTypeFromDef(t_const_kind kind) {
	if (kind == CONST_CHAR) {
		return	LLVMInt8Type();
	} else if (kind == CONST_STR) {
		return LLVMArrayType(LLVMInt8Type(), 0);
	} else if (kind == CONST_INT || kind == CONST_OP) {
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
//	char *ir = LLVMPrintModuleToString(mod);
//	printf("%s\n", ir);
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
	LLVMTypeRef		proto = LLVMFunctionType(LLVMInt32Type(), param_types, params->len, 0);

	return proto;
}

LLVMValueRef	createValue(t_node_var *value, LLVMTypeRef type) {
				LLVMValueRef init = NULL;
	(void) value;
	(void) type;

	return init;
}

bool	assignToGlobalVar(t_node_var *value, LLVMModuleRef	mod) {
(void) value;
(void) mod;
//	LLVMValueRef init = value->val.ref;
//	if (!init) {
//		return false;
//	}
//	LLVMValueRef	global = LLVMAddGlobal(mod, value->type, value->name);
//	if (global == NULL) {
//		printf("Variable already exist.\n");
//		return false;
//	}
//	fillTypeFromKind(value->kind, value);
//	value->val = (t_val){.ref=global};
//	tsearch(value, &current_scope->root_tree, compare_var_name);
//	LLVMSetInitializer(global, init);
//	LLVMSetGlobalConstant(global, 0);
	return true;
}

bool	allocVariable(t_node_var *var, LLVMBuilderRef builder) {
	// Create an alloca for a 32-bit integer variable named "var"
	LLVMValueRef var_alloca = LLVMBuildAlloca(builder, var->type, var->name);
//
// Optionally initialize with a value like 42
//	LLVMValueRef init_val = LLVMConstInt(type, var->val.num, /* depending signed */0);
	LLVMBuildStore(builder, var->val, var_alloca);
	return true;
};

bool	addGlobalVariable(t_node_var	*def, LLVMModuleRef	mod) {
	(void) def;
	(void) mod;
	return (true);
//	LLVMTypeRef		type;
// 
//	if (!def) 
//		return false;
//	// if c'est un tableau
//	if (def->kind == CONST_ARR) {
//		t_var_list **head = def->val.array->values;
//		if (!head || !(*head)) {
//			
//		} else {
//			type = getTypeFromDef((*head)->elem->kind);
//			LLVMTypeRef arrayType = LLVMArrayType(type, def->val.array->len);
//			// Surement mieux de malloc
//			LLVMValueRef elements[def->val.array->len];
//			t_var_list *buff = *head;
//			int i = 0;
//			while (buff) {
//				elements[i] = createValue(buff->elem, type);
//				buff = buff->next;
//				i++;
//			}
//			LLVMValueRef arrayVal = LLVMConstArray(arrayType, elements, def->val.array->len);
//			LLVMValueRef globalArray = LLVMAddGlobal(mod, arrayType, "arr");
//			LLVMSetInitializer(globalArray, arrayVal);
//			return true;
//		}
//		// Cree le global array
//	} else if (def->kind == CONST_STR) {
//		LLVMTypeRef arrayType = LLVMArrayType(LLVMInt8TypeInContext(ctx), strlen(def->val.str) + 1);
//		LLVMValueRef constStr = LLVMConstStringInContext(ctx, def->val.str, strlen(def->val.str), 0);
//		LLVMValueRef globalVar = LLVMAddGlobal(mod, arrayType, /* faut concatener un . devant */def->name);
//		LLVMSetInitializer(globalVar, constStr);
//		LLVMSetAlignment(globalVar, 1);
//		def->val.ref = globalVar;
//		def->type = arrayType;
//		tsearch(def, &current_scope->root_tree, compare_var_name);
//		return true;
//	}
//	type = getTypeFromDef(def->kind);
//	
//	def->type = type;
//	char *type_str = LLVMPrintTypeToString(def->type);
//	printf("Pointed-to type: %s %i\n", type_str, def->kind);
//
//	// Stocker la ref dans le tree, beacoup plus simple genre on la remplace
//	return assignToGlobalVar(def, mod);
	}

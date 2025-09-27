#include <stdio.h>
#include <stdlib.h>
#include <llvm-c/Core.h>
#include <llvm-c/ExecutionEngine.h>
#include <llvm-c/Target.h>
#include <llvm-c/TargetMachine.h>
#include <llvm-c/Analysis.h>
#include <llvm-c/BitWriter.h>
#include <llvm-c/BitReader.h>
#include <llvm-c/Support.h>

void	generate_asm(LLVMModuleRef mod);


int main(int argc, char const *argv[]) {
	LLVMModuleRef mod = LLVMModuleCreateWithName("program");
	LLVMTypeRef param_types[] = { LLVMInt32Type(), LLVMInt32Type()};

	/* LLVM_C_ABI LLVMTypeRef LLVMFunctionType	(	LLVMTypeRef	ReturnType,
	LLVMTypeRef *	ParamTypes,
	unsigned	ParamCount,
	LLVMBool	IsVarArg ) */

	/*
		CREATION DUN FONCTION
	*/


	LLVMTypeRef		pointer = LLVMPointerType(LLVMVoidType(), 0);
	// Ici on on cree la function (on la définie)
	LLVMTypeRef		test_prototype = LLVMFunctionType(pointer, param_types, 2, 0);

	// On a ajoute la fonction a notre module
	LLVMValueRef	test = LLVMAddFunction(mod, "test", test_prototype);

	// On prepare "le scope" de la fonction test, avec un block d'instruction appelé entry
	LLVMBasicBlockRef	entry = LLVMAppendBasicBlock(test, "entry");

	// On initialise notre builder qui nous servira a nous deplacer dans les bloack d'instruction plus tard.
	LLVMBuilderRef builder = LLVMCreateBuilder();

	// On va mettre le cursor la la fin du block d'instriction entre cree plus tot
	LLVMPositionBuilderAtEnd(builder, entry);

	LLVMValueRef tmp = LLVMBuildAdd(builder, LLVMGetParam(test, 0), LLVMGetParam(test, 1), "test");

	LLVMBuildRet(builder, tmp);

// ICI LLVMBUILDADD est une function qui sert a génére les instruction pour une addition..
//	LLVMValueRef	tmp = LLVMBuildAdd(builder, "LHS", "RHS", "tmp");
//	LLVMBuildRetVoid(builder);

	/*
		TEST DU MODULE
	*/

	char	*error = NULL;

	// Function qui verifie l'intégrité du module
	LLVMVerifyModule(mod, LLVMReturnStatusAction, &error);

	if (error != NULL && *error) {
		printf("ERROR: %s\n", error);
	}
	// Free le message .
	LLVMDisposeMessage(error);

	LLVMValueRef instr = LLVMGetFirstInstruction(entry);

	while (instr != NULL) {
		char *instrStr = LLVMPrintValueToString(instr);
		printf("Instruction: %s\n", instrStr);
		instr = LLVMGetNextInstruction(instr);
		LLVMDisposeMessage(instrStr);
	}


	

//	LLVMExecutionEngineRef	engine;

	/*
		EXECUTION DES INSTRUCTIONS
	*/
	LLVMDisposeBuilder(builder);
	generate_asm(mod);
//	LLVMDisposeExecutionEngine(engine);
}

// Convertis le module en assembleur.
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

#ifndef _H_CODE_GEN
#define _H_CODE_GEN


#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Scalar/GVN.h"
#include "llvm/Transforms/Utils.h"
#include "KaleidoscopeJIT.h"
#include <unordered_map>


/**
 * Global Variable Declare
 */
// Add a flag to control whether to print out LLVM IR
extern bool g_enable_ir_print;

// Record the core "global" data of LLVM's core infrastructure, e.g. types and constants uniquing table
extern llvm::LLVMContext g_llvm_context;

// Used for creating LLVM IR (Intermediate Representation)
extern llvm::IRBuilder<> g_ir_builder;

// Used for managing functions and global variables. You can consider it as a compile unit (like single .cpp file)
extern std::unique_ptr<llvm::Module> g_module;

// Used for recording the local named variables
extern std::unordered_map<std::string, llvm::AllocaInst*> g_local_named_vars;

// Used for recording the global named variables
extern std::unordered_map<std::string, llvm::AllocaInst*> g_global_named_vars;

// Function Passes Manager for CodeGen Optimizer
extern std::unique_ptr<llvm::legacy::FunctionPassManager> g_fpm;

// Add JIT Compiler
extern std::unique_ptr<llvm::orc::KaleidoscopeJIT> g_jit;


/**
 * Function Declare
*/
// query function interface via function name
llvm::Function* GetFunction(const std::string& name);

// add memory allocate instruction in the entry-block of function
llvm::AllocaInst* CreateEntryBlockAlloca(llvm::Function* func, const std::string& var_name);

// find variable AllocaInst from local_variable_table and global_variable_table
llvm::AllocaInst* FindVariableAllocaInst(const std::string& name);

void ReCreateModule();

void ParseDefinitionToken();

void ParseExternToken();

void ParseTopLevel();


#endif // _H_CODE_GEN

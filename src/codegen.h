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


/**
 * Global Variable Declare
 */
// Record the core "global" data of LLVM's core infrastructure, e.g. types and constants uniquing table
extern llvm::LLVMContext g_llvm_context;

// Used for creating LLVM IR (Intermediate Representation)
extern llvm::IRBuilder<> g_ir_builder(g_llvm_context);

// Used for managing functions and global variables. You can consider it as a compile unit (like single .cpp file)
extern llvm::Module g_module("my cool jit", g_llvm_context);

// Used for recording the parameters of function
extern std::unordered_map<std::string, llvm::Value*> g_named_values;


#endif // _H_CODE_GEN

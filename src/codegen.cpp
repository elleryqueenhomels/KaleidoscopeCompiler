#include "codegen.h"
#include "parser.h"
#include "lexer.h"


// Record the core "global" data of LLVM's core infrastructure, e.g. types and constants uniquing table
llvm::LLVMContext g_llvm_context;

// Used for creating LLVM IR (Intermediate Representation)
llvm::IRBuilder<> g_ir_builder(g_llvm_context);

// Used for managing functions and global variables. You can consider it as a compile unit (like single .cpp file)
llvm::Module g_module("my cool jit", g_llvm_context);

// Used for recording the parameters of function
std::unordered_map<std::string, llvm::Value*> g_named_values;

llvm::Value* NumberExprAST::CodeGen() {
    return llvm::ConstantFP::get(g_llvm_context, llvm::APFloat(val_));
}

llvm::Value* VariableExprAST::CodeGen() {
    return g_named_values.at(name_);
}

llvm::Value* BinaryExprAST::CodeGen() {
    llvm::Value* lhs = lhs_->CodeGen();
    llvm::Value* rhs = rhs_->CodeGen();
    switch (op_) {
        case '<': {
            llvm::Value* tmp = g_ir_builder.CreateFCmpULT(lhs, rhs, "cmptmp");
            // convert 0/1 to 0.0/1.0
            return g_ir_builder.CreateUIToFP(tmp, llvm::Type::getDoubleTy(g_llvm_context), "booltmp");
        }
        case '+': return g_ir_builder.CreateFAdd(lhs, rhs, "addtmp");
        case '-': return g_ir_builder.CreateFSub(lhs, rhs, "subtmp");
        case '*': return g_ir_builder.CreateFMul(lhs, rhs, "multmp");
        case '/': return g_ir_builder.CreateFDiv(lhs, rhs, "divtmp");
        default: return nullptr;
    }
}

llvm::Value* CallExprAST::CodeGen() {
    // g_module stores global variables and functions
    llvm::Function* callee = g_module.getFunction(callee_);

    std::vector<llvm::Value*> args;
    for (std::unique_ptr<ExprAST>& arg_expr : args_) {
        args.push_back(arg_expr->CodeGen());
    }

    return g_ir_builder.CreateCall(callee, args, "calltmp");
}

llvm::Value* PrototypeAST::CodeGen() {
    // create kaleidoscope function type: double (doube, double, ..., double)
    std::vector<llvm::Type*> doubles(args_.size(), llvm::Type::getDoubleTy(g_llvm_context));

    // function is unique，so use 'get' not 'new'/'create'
    llvm::FunctionType* function_type = llvm::FunctionType::get(llvm::Type::getDoubleTy(g_llvm_context), doubles, false);

    // create function, ExternalLinkage means function may not be defined in current module
    // we register it using name_ in current module `g_module`, so that can query it using this name later
    llvm::Function* func = llvm::Function::Create(function_type, llvm::Function::ExternalLinkage, name_, &g_module);

    // increase IR readability，set argument name for function
    int index = 0;
    for (auto& arg : func->args()) {
        arg.setName(args_[index++]);
    }

    return func;
}

llvm::Value* FunctionAST::CodeGen() {
    // check if function declare has been finished codegen (e.g. previous `extern` declare), otherwise execute codegen
    llvm::Function* func = g_module.getFunction(proto_->name());
    if (func == nullptr) {
        func = (llvm::Function*) proto_->CodeGen();
    }

    // create a Block and set insert point
    // llvm block can be used for defining control flow graph.
    // since currently we don't implement control flow, so create a single block is good enough
    llvm::BasicBlock* block = llvm::BasicBlock::Create(g_llvm_context, "entry", func);
    g_ir_builder.SetInsertPoint(block);

    // register function arguments to `g_named_values`, so VariableExprAST can codegen
    g_named_values.clear();
    for (llvm::Value& arg : func->args()) {
        g_named_values[(std::string) arg.getName()] = &arg;
    }

    // codegen body then return
    llvm::Value* ret_val = body_->CodeGen();
    g_ir_builder.CreateRet(ret_val);
    llvm::verifyFunction(*func);
    return func;
}

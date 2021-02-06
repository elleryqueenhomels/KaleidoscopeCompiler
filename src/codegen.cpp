#include "codegen.h"
#include "parser.h"
#include "lexer.h"
#include <iostream>


// Record the core "global" data of LLVM's core infrastructure, e.g. types and constants uniquing table
llvm::LLVMContext g_llvm_context;

// Used for creating LLVM IR (Intermediate Representation)
llvm::IRBuilder<> g_ir_builder(g_llvm_context);

// Used for managing functions and global variables. You can consider it as a compile unit (like single .cpp file)
std::unique_ptr<llvm::Module> g_module = std::make_unique<llvm::Module>("my cool jit", g_llvm_context);

// Used for recording the parameters of function
std::unordered_map<std::string, llvm::Value*> g_named_values;

// Function Passes Manager for CodeGen Optimizer
std::unique_ptr<llvm::legacy::FunctionPassManager> g_fpm = std::make_unique<llvm::legacy::FunctionPassManager>(g_module.get());

// Add JIT Compiler
std::unique_ptr<llvm::orc::KaleidoscopeJIT> g_jit;

// Add dictionary for function name to function interface
std::unordered_map<std::string, std::unique_ptr<PrototypeAST>> name2proto_ast;


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
            llvm::Value* tmp = g_ir_builder.CreateFCmpULT(lhs, rhs, "ltcmptmp");
            // convert 0/1 to 0.0/1.0
            return g_ir_builder.CreateUIToFP(tmp, llvm::Type::getDoubleTy(g_llvm_context), "booltmp");
        }
        case '>': {
            llvm::Value* tmp = g_ir_builder.CreateFCmpUGT(lhs, rhs, "gtcmptmp");
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
    llvm::Function* callee = GetFunction(callee_);

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
    llvm::Function* func = llvm::Function::Create(function_type, llvm::Function::ExternalLinkage, name_, *g_module);

    // increase IR readability，set argument name for function
    int index = 0;
    for (auto& arg : func->args()) {
        arg.setName(args_[index++]);
    }

    return func;
}

llvm::Value* FunctionAST::CodeGen() {
    PrototypeAST& proto = *proto_;
    name2proto_ast[proto.name()] = std::move(proto_); // transfer ownership

    llvm::Function* func = GetFunction(proto.name());

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

    // add optimization for function codegen
    g_fpm->run(*func);

    return func;
}

llvm::Function* GetFunction(const std::string& name) {
    llvm::Function* callee = g_module->getFunction(name);

    // current module exists function definition
    if (callee != nullptr) {
        return callee;
    }
    
    // declare function
    return (llvm::Function*) name2proto_ast.at(name)->CodeGen();
}

void ReCreateModule() {
    // Open a new module.
    g_module = std::make_unique<llvm::Module>("my cool jit", g_llvm_context);
    g_module->setDataLayout(g_jit->getTargetMachine().createDataLayout());

    // Create a new pass manager attached to it.
    g_fpm = std::make_unique<llvm::legacy::FunctionPassManager>(g_module.get());

    // Do simple "peephole" optimizations and bit-twiddling optzns.
    g_fpm->add(llvm::createInstructionCombiningPass());
    // Reassociate expressions.
    g_fpm->add(llvm::createReassociatePass());
    // Eliminate Common SubExpressions.
    g_fpm->add(llvm::createGVNPass());
    // Simplify the control flow graph (deleting unreachable blocks, etc).
    g_fpm->add(llvm::createCFGSimplificationPass());

    g_fpm->doInitialization();
}

void ParseDefinitionToken() {
    auto ast = ParseDefinition();
    std::cout << "parsed a function definition" << std::endl;
    ast->CodeGen()->print(llvm::errs());
    std::cerr << std::endl;

    g_jit->addModule(std::move(g_module));
    ReCreateModule();
}

void ParseExternToken() {
    auto ast = ParseExtern();
    std::cout << "parsed an extern" << std::endl;
    ast->CodeGen()->print(llvm::errs());
    std::cerr << std::endl;

    name2proto_ast[ast->name()] = std::move(ast);
}

void ParseTopLevel() {
    auto ast = ParseTopLevelExpr();
    std::cout << "parsed a top level expr" << std::endl;
    ast->CodeGen()->print(llvm::errs());
    std::cout << std::endl;

    auto h = g_jit->addModule(std::move(g_module));

    // re-create g_module for next time using
    ReCreateModule();

    // find compiled function symbol through name
    auto symbol = g_jit->findSymbol(top_level_expr_name);

    // force cast to C function pointer
    double (*fp)() = (double (*)()) (symbol.getAddress().get());

    // execute and output
    std::cout << "Evaluated to:" << std::endl;
    std::cout << fp() << std::endl << std::endl;

    g_jit->removeModule(h);
}

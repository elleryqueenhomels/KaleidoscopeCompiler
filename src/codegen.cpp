#include "codegen.h"
#include "parser.h"
#include "lexer.h"
#include <iostream>


// Record the core "global" data of LLVM's core infrastructure, e.g. types and constants uniquing table
llvm::LLVMContext g_llvm_context;

// Used for creating LLVM IR (Intermediate Representation)
llvm::IRBuilder<> g_ir_builder(g_llvm_context);

// Used for managing functions and global variables. You can consider it as a compile unit (like single .cpp file)
std::unique_ptr<llvm::Module> g_module;

// Used for recording the parameters of function
std::unordered_map<std::string, llvm::AllocaInst*> g_named_values;

// Function Passes Manager for CodeGen Optimizer
std::unique_ptr<llvm::legacy::FunctionPassManager> g_fpm;

// Add JIT Compiler
std::unique_ptr<llvm::orc::KaleidoscopeJIT> g_jit;

// Add dictionary for function name to function interface
std::unordered_map<std::string, std::unique_ptr<PrototypeAST>> name2proto_ast;


llvm::Value* NumberExprAST::CodeGen() {
    return llvm::ConstantFP::get(g_llvm_context, llvm::APFloat(val_));
}

llvm::Value* VariableExprAST::CodeGen() {
    llvm::AllocaInst* val = g_named_values.at(name_);
    return g_ir_builder.CreateLoad(val, name_.c_str());
}

llvm::Value* BinaryExprAST::CodeGen() {
    llvm::Value* lhs = lhs_->CodeGen();
    llvm::Value* rhs = rhs_->CodeGen();

    // if (op_ == "&&") {
    //     g_ir_builder.CreateFCmpONE(lhs, llvm::ConstantFP::get(g_llvm_context, llvm::APFloat(0.0)), "andleftcond");
    //     return g_ir_builder.CreateAnd(lhs, rhs, "andtmp");
    // }

    // if (op_ == "||") {
    //     return g_ir_builder.CreateOr(lhs, rhs, "ortmp");
    // }

    if (op_ == "==") {
        llvm::Value* tmp = g_ir_builder.CreateFCmpUEQ(lhs, rhs, "eqcmptmp");
        // convert 0/1 to 0.0/1.0
        return g_ir_builder.CreateUIToFP(tmp, llvm::Type::getDoubleTy(g_llvm_context), "booltmp");
    }

    if (op_ == "!=") {
        llvm::Value* tmp = g_ir_builder.CreateFCmpUNE(lhs, rhs, "necmptmp");
        // convert 0/1 to 0.0/1.0
        return g_ir_builder.CreateUIToFP(tmp, llvm::Type::getDoubleTy(g_llvm_context), "booltmp");
    }

    if (op_ == "<=") {
        llvm::Value* tmp = g_ir_builder.CreateFCmpULE(lhs, rhs, "lecmptmp");
        // convert 0/1 to 0.0/1.0
        return g_ir_builder.CreateUIToFP(tmp, llvm::Type::getDoubleTy(g_llvm_context), "booltmp");
    }

    if (op_ == ">=") {
        llvm::Value* tmp = g_ir_builder.CreateFCmpUGE(lhs, rhs, "gecmptmp");
        // convert 0/1 to 0.0/1.0
        return g_ir_builder.CreateUIToFP(tmp, llvm::Type::getDoubleTy(g_llvm_context), "booltmp");
    }

    if (op_ == "<") {
        llvm::Value* tmp = g_ir_builder.CreateFCmpULT(lhs, rhs, "ltcmptmp");
        // convert 0/1 to 0.0/1.0
        return g_ir_builder.CreateUIToFP(tmp, llvm::Type::getDoubleTy(g_llvm_context), "booltmp");
    }

    if (op_ == ">") {
        llvm::Value* tmp = g_ir_builder.CreateFCmpUGT(lhs, rhs, "gtcmptmp");
        // convert 0/1 to 0.0/1.0
        return g_ir_builder.CreateUIToFP(tmp, llvm::Type::getDoubleTy(g_llvm_context), "booltmp");
    }

    if (op_ == "+") {
        return g_ir_builder.CreateFAdd(lhs, rhs, "addtmp");
    }

    if (op_ == "-") {
        return g_ir_builder.CreateFSub(lhs, rhs, "subtmp");
    }

    if (op_ == "*") {
        return g_ir_builder.CreateFMul(lhs, rhs, "multmp");
    }

    if (op_ == "/") {
        return g_ir_builder.CreateFDiv(lhs, rhs, "divtmp");
    }

    // user defined operator
    llvm::Function* func = GetFunction(std::string("binary") + op_);
    llvm::Value* operands[2] = { lhs, rhs };
    return g_ir_builder.CreateCall(func, operands, "binop");
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

    // register operator precedence if this is an operator define func
    if (proto.IsBinaryOp()) {
        g_binop_precedence[proto.GetOpName()] = proto.op_precedence();
    }

    // create a block and set insert point
    // llvm block can be used for defining control flow graph
    llvm::BasicBlock* block = llvm::BasicBlock::Create(g_llvm_context, "entry", func);
    g_ir_builder.SetInsertPoint(block);

    // register function arguments to `g_named_values`, so VariableExprAST can codegen
    g_named_values.clear();
    for (llvm::Value& arg : func->args()) {
        // create a variable on stack for each function argument & assign the initial value
        // set argument name and corresponding variable into g_named_values
        // so that in later code piece we can ref the on stack variable
        llvm::AllocaInst* var = CreateEntryBlockAlloca(func, (std::string) arg.getName());
        g_ir_builder.CreateStore(&arg, var);
        g_named_values[(std::string) arg.getName()] = var;
    }

    // codegen body then return
    llvm::Value* ret_val = body_->CodeGen();
    g_ir_builder.CreateRet(ret_val);
    llvm::verifyFunction(*func);

    // add optimization for function codegen
    g_fpm->run(*func);

    return func;
}

llvm::Value* IfExprAST::CodeGen() {
    llvm::Value* cond_value = cond_->CodeGen();

    // convert condition to a bool by comparing non-equal to 0.0
    cond_value = g_ir_builder.CreateFCmpONE(
        cond_value, llvm::ConstantFP::get(g_llvm_context, llvm::APFloat(0.0)), "ifcond");

    // since we will create a block for each function, so here we must be already inside a block
    // we can access the parent function via the current block
    llvm::Function* func = g_ir_builder.GetInsertBlock()->getParent();

    // create blocks for the then and else cases
    // insert the 'then' block at the end of the function
    llvm::BasicBlock* then_block =
        llvm::BasicBlock::Create(g_llvm_context, "then", func);
    llvm::BasicBlock* else_block =
        llvm::BasicBlock::Create(g_llvm_context, "else");
    llvm::BasicBlock* final_block =
        llvm::BasicBlock::Create(g_llvm_context, "ifcont");

    // create jump instruction, use cond_value to choose then_block/else_block
    g_ir_builder.CreateCondBr(cond_value, then_block, else_block);

    // emit then value
    g_ir_builder.SetInsertPoint(then_block);

    // codegen then_block, add instruction to jump to final_block
    llvm::Value* then_value = then_expr_->CodeGen();
    g_ir_builder.CreateBr(final_block);

    // inside then statement, there may be nested if-then-else,
    // with nested codegen, it will change the current block,
    // we use the block which has the final result as the current then_block
    then_block = g_ir_builder.GetInsertBlock();

    // we only add else_block here in order to guarantee
    // the else_block is put behind the most outer then_block above
    func->getBasicBlockList().push_back(else_block);

    // emit else value
    g_ir_builder.SetInsertPoint(else_block);

    // codegen else_block, similar to then_block
    llvm::Value* else_value = else_expr_->CodeGen();
    g_ir_builder.CreateBr(final_block);

    // same reason as then_block (nested if-then-else)
    else_block = g_ir_builder.GetInsertBlock();

    // same reason as else_block
    func->getBasicBlockList().push_back(final_block);

    // emit final block
    g_ir_builder.SetInsertPoint(final_block);

    // NumReservedValues is a hint for the number of incoming edges
    // that this phi node will have (use 0 if you really have no idea)
    llvm::PHINode* pn = g_ir_builder.CreatePHI(
        llvm::Type::getDoubleTy(g_llvm_context), 2, "iftmp");

    pn->addIncoming(then_value, then_block);
    pn->addIncoming(else_value, else_block);

    return pn;
}

llvm::Value* ForExprAST::CodeGen() {
    // get current function
    llvm::Function* func = g_ir_builder.GetInsertBlock()->getParent();

    // create variable on stack, no more phi node
    llvm::AllocaInst* var = CreateEntryBlockAlloca(func, var_name_);

    // codegen start
    llvm::Value* start_val = start_expr_->CodeGen();

    // assign the start_val to var
    g_ir_builder.CreateStore(start_val, var);

    // add a loop block into current function
    llvm::BasicBlock* loop_block = llvm::BasicBlock::Create(g_llvm_context, "forloop", func);

    // add instruction: jump to loop_block
    g_ir_builder.CreateBr(loop_block);

    // now begin to add instructions into loop_block
    g_ir_builder.SetInsertPoint(loop_block);

    // now we have a new variable, since it may be referenced in the later code piece
    // so we need to register it into g_named_values
    // NOTE: var_name may be duplicated with function argument names
    // currently we ignore this special case for convenience
    g_named_values[var_name_] = var;

    // add body instructions into loop_block
    body_expr_->CodeGen();

    // codegen step_expr
    llvm::Value* step_value = step_expr_->CodeGen();

    // var = var + step_value
    llvm::Value* cur_value = g_ir_builder.CreateLoad(var);
    llvm::Value* next_value = g_ir_builder.CreateFAdd(cur_value, step_value, "nextvar");
    
    // assign next_value back to var
    g_ir_builder.CreateStore(next_value, var);

    // codegen end_expr
    llvm::Value* end_value = end_expr_->CodeGen();

    // end_value = (end_value != 0.0)
    end_value = g_ir_builder.CreateFCmpONE(
        end_value, llvm::ConstantFP::get(g_llvm_context, llvm::APFloat(0.0)), "loopcond");

    // similar to if-then-else, the block may be changed due to `body_expr_->CodeGen()`
    // get the most outer block via `g_ir_builder.GetInsertBlock()`
    // llvm::BasicBlock* loop_end_block = g_ir_builder.GetInsertBlock();

    // create block for loop ends
    llvm::BasicBlock* after_block = llvm::BasicBlock::Create(g_llvm_context, "afterloop", func);

    // use end_value to choose enter loop_block again or finish loop
    g_ir_builder.CreateCondBr(end_value, loop_block, after_block);

    // add instructions into after_block
    g_ir_builder.SetInsertPoint(after_block);

    // erase var_name when loop ends
    g_named_values.erase(var_name_);

    // return 0
    return llvm::Constant::getNullValue(llvm::Type::getDoubleTy(g_llvm_context));
}

llvm::Function* GetFunction(const std::string& name) {
    llvm::Function* callee = g_module->getFunction(name);

    // current module exists function definition
    if (callee != nullptr) {
        return callee;
    }
    
    // declare function (use PrototypeAST to CodeGen)
    return (llvm::Function*) name2proto_ast.at(name)->CodeGen();
}

// add memory allocate instruction in the entry-block of function
llvm::AllocaInst* CreateEntryBlockAlloca(llvm::Function* func, const std::string& var_name) {
    llvm::IRBuilder<> ir_builder(&(func->getEntryBlock()), func->getEntryBlock().begin());
    return ir_builder.CreateAlloca(llvm::Type::getDoubleTy(g_llvm_context), nullptr, var_name.c_str());
}

void ReCreateModule() {
    // open a new module
    g_module = std::make_unique<llvm::Module>("my cool jit", g_llvm_context);
    g_module->setDataLayout(g_jit->getTargetMachine().createDataLayout());

    // create a new pass manager attached to g_module
    g_fpm = std::make_unique<llvm::legacy::FunctionPassManager>(g_module.get());

    // Promote allocas to registers.
    g_fpm->add(llvm::createPromoteMemoryToRegisterPass());
    // do simple "peephole" optimizations and bit-twiddling optzns
    g_fpm->add(llvm::createInstructionCombiningPass());
    // reassociate expressions
    g_fpm->add(llvm::createReassociatePass());
    // eliminate Common SubExpressions
    g_fpm->add(llvm::createGVNPass());
    // simplify the control flow graph (deleting unreachable blocks, etc)
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

    auto moduleKey = g_jit->addModule(std::move(g_module));

    // re-create g_module for next time using
    ReCreateModule();

    // find compiled function symbol through name
    auto symbol = g_jit->findSymbol(top_level_expr_name);

    // force cast to C function pointer
    double (*fp)() = (double (*)()) (symbol.getAddress().get());

    // execute and output
    std::cout << "Evaluated to:" << std::endl;
    std::cout << fp() << std::endl << std::endl;

    g_jit->removeModule(moduleKey);
}

// implement a printd function
extern "C" double printd(double x) {
    printf("%lf\n", x);
    return 0.0;
}

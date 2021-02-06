#include "codegen.h"
#include "parser.h"
#include "lexer.h"
#include <iostream>


int main() {
    // add passes for codegen optimization
    g_fpm.add(llvm::createInstructionCombiningPass());
    g_fpm.add(llvm::createReassociatePass());
    g_fpm.add(llvm::createGVNPass());
    g_fpm.add(llvm::createCFGSimplificationPass());
    g_fpm.doInitialization();

    GetNextToken();
    while (true) {
        switch (g_current_token) {
            case TOKEN_EOF: return 0;
            case TOKEN_DEF: {
                auto ast = ParseDefinition();
                std::cout << "parsed a function definition" << std::endl;
                ast->CodeGen()->print(llvm::errs());
                std::cerr << std::endl;
                break;
            }
            case TOKEN_EXTERN: {
                auto ast = ParseExtern();
                std::cout << "parsed a extern" << std::endl;
                ast->CodeGen()->print(llvm::errs());
                std::cerr << std::endl;
                break;
            }
            default: {
                auto ast = ParseTopLevelExpr();
                std::cout << "parsed a top level expr" << std::endl;
                ast->CodeGen()->print(llvm::errs());
                std::cerr << std::endl;
                break;
            }
        }
    }
    return 0;
}

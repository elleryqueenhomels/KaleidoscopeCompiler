#include "../src/codegen.h"
#include "../src/parser.h"
#include "../src/lexer.h"
#include <iostream>


int main() {
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

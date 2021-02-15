#include "codegen.h"
#include "parser.h"
#include "lexer.h"
#include <iostream>


int main() {
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();
    llvm::InitializeNativeTargetAsmParser();

    g_jit.reset(new llvm::orc::KaleidoscopeJIT);
    ReCreateModule();

    GetNextToken();
    while (true) {
        switch (g_current_token) {
            case TOKEN_EOF: return 0;
            case TOKEN_DEF: ParseDefinitionToken(); break;
            case TOKEN_EXTERN: ParseExternToken(); break;
            default: ParseTopLevel(); break;
        }
    }

    return 0;
}

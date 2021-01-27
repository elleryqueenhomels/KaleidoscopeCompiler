#include <iostream>
#include "../src/lexer.h"
#include "../src/parser.h"

int main() {
    GetNextToken();
    while (true) {
        switch (g_current_token) {
            case TOKEN_EOF:
                return 0;

            case TOKEN_DEF:
                ParseDefinition();
                std::cout << "parsed a function definition" << std::endl;
                break;

            case TOKEN_EXTERN:
                ParseExtern();
                std::cout << "parsed a extern" << std::endl;
                break;

            default:
                ParseTopLevelExpr();
                std::cout << "parsed a top level expr" << std::endl;
                break;
        }
    }
    return 0;
}

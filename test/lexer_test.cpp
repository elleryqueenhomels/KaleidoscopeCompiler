#include "../src/lexer.h"


int main() {
    int token;

    do {
        token = GetToken();
        if (token == TOKEN_DEF) {
            printf("def ");
        } else if (token == TOKEN_EXTERN) {
            printf("extern ");
        } else if (token == TOKEN_IDENTIFIER) {
            printf("%s ", g_identifier_str.c_str());
        } else if (token == TOKEN_NUMBER) {
            printf("%f ", g_number_val);
        } else if (token == TOKEN_IF) {
            printf("if ");
        } else if (token == TOKEN_THEN) {
            printf("then ");
        } else if (token == TOKEN_ELSE) {
            printf("else ");
        } else if (token == TOKEN_FOR) {
            printf("for ");
        } else if (token == TOKEN_IN) {
            printf("in ");
        } else if (token == TOKEN_BINARY) {
            printf("binary ");
        } else if (token == TOKEN_OPERATOR) {
            printf("%s ", g_operator_str.c_str());
        } else if (token != TOKEN_EOF) {
            printf("%s ", (char*) &token);
        }
    }
    while (token != TOKEN_EOF);

    printf("\n");

    return 0;
}

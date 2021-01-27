#include "../src/lexer.h"


int main() {
    int token;

    do {
        token = GetToken();
        if (token == TOKEN_NUMBER) {
            printf("%f ", g_number_val);
        } else if (token == TOKEN_DEF) {
            printf("def ");
        } else if (token == TOKEN_EXTERN) {
            printf("extern ");
        } else if (token == TOKEN_IDENTIFIER) {
            printf("%s ", g_identifier_str.c_str());
        }
    }
    while (token != TOKEN_EOF);

    printf("\n");

    return 0;
}

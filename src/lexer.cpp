#include "lexer.h"
#include <string>


// Filled in if TOKEN_IDENTIFIER
std::string g_identifier_str;

// Filled in if TOKEN_NUMBER
double g_number_val;

// extract a token from stdin
int GetToken() {
    static int last_char = ' ';

    // ignore white space
    while (isspace(last_char)) {
        last_char = getchar();
    }

    // identify character
    if (isalpha(last_char)) {
        g_identifier_str = last_char;

        while (isalnum((last_char = getchar()))) {
            g_identifier_str += last_char;
        }

        if (g_identifier_str == "def") {
            return TOKEN_DEF;
        }
        if (g_identifier_str == "extern") {
            return TOKEN_EXTERN;
        }
        if (g_identifier_str == "if") {
            return TOKEN_IF;
        }
        if (g_identifier_str == "then") {
            return TOKEN_THEN;
        }
        if (g_identifier_str == "else") {
            return TOKEN_ELSE;
        }
        return TOKEN_IDENTIFIER;
    }

    // identify number
    if (isdigit(last_char) || last_char == '.') {
        std::string num_str;

        do {
            num_str += last_char;
            last_char = getchar();
        }
        while (isdigit(last_char) || last_char == '.');

        g_number_val = strtod(num_str.c_str(), nullptr);

        return TOKEN_NUMBER;
    }

    // ignore comment
    if (last_char == '#') {
        do {
            last_char = getchar();
        }
        while (last_char != EOF && last_char != '\n' && last_char != '\r');

        if (last_char != EOF) {
            return GetToken();
        }
    }

    // identify end of file
    if (last_char == EOF) {
        return TOKEN_EOF;
    }

    // return ASCII directly
    int this_char = last_char;
    last_char = getchar();
    return this_char;
}

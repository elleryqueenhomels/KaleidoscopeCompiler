#include "lexer.h"
#include <string>
#include <unordered_set>


// Filled in if TOKEN_IDENTIFIER
std::string g_identifier_str;

// Filled in if TOKEN_NUMBER
double g_number_val;

// Filled in if TOKEN_OPERATOR
std::string g_operator_str;

// operator basic characters
const std::unordered_set<char> operator_char_set = {
    '<', '>', '=', '!', '&', '|', '~',
    '+', '-', '*', '/', '%', '$', '^'
};

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
        if (g_identifier_str == "end") {
            return TOKEN_END;
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
        if (g_identifier_str == "for") { 
            return TOKEN_FOR; 
        }
        if (g_identifier_str == "in") { 
            return TOKEN_IN; 
        }
        if (g_identifier_str == "binary") {
            return TOKEN_BINARY;
        }
        if (g_identifier_str == "unary") {
            return TOKEN_UNARY;
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

    // identify operator
    if (operator_char_set.count(last_char)) {
        g_operator_str = last_char;
        while (operator_char_set.count(last_char = getchar())) {
            g_operator_str += last_char;
        }
        return TOKEN_OPERATOR;
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

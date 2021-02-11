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

// is valid variable name element
bool isVarChar(const char& ch) {
    return isalnum(ch) || ch == '_';
}

// extract a token from stdin
int GetToken() {
    static int last_char = ' ';

    // ignore white space
    while (isspace(last_char)) {
        last_char = getchar();
    }

    // identify character
    if (isalpha(last_char) || last_char == '_') {
        g_identifier_str = last_char;

        while (isVarChar((last_char = getchar()))) {
            g_identifier_str += last_char;
        }

        if (g_token_mapping.find(g_identifier_str) != g_token_mapping.end()) {
            return g_token_mapping.at(g_identifier_str);
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

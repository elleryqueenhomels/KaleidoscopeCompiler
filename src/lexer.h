#ifndef _H_LEXER
#define _H_LEXER

#include <string>

/**
 * Enum Declare
 */
enum Token {
    TOKEN_EOF = -1,
    TOKEN_DEF = -2,
    TOKEN_EXTERN = -3,
    TOKEN_IDENTIFIER = -4,
    TOKEN_NUMBER = -5
};


/**
 * Global Variable Declare
 */
// Filled in if TOKEN_IDENTIFIER
std::string g_identifier_str;

// Filled in if TOKEN_NUMBER
double g_number_val;


/**
 * Function Declare
 */
// extract a token from stdin
int GetToken();

#endif // _H_LEXER
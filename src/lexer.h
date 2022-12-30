#ifndef _H_LEXER
#define _H_LEXER

#include <string>
#include <unordered_map>

/**
 * Global Variable Declare
 */
// Filled in if TOKEN_IDENTIFIER
extern std::string g_identifier_str;

// Filled in if TOKEN_NUMBER
extern double g_number_val;

// Filled in if TOKEN_OPERATOR
extern std::string g_operator_str;

/**
 * Enum Declare
 */
enum Token {
    TOKEN_EOF = -1,
    TOKEN_DEF = -2,
    TOKEN_EXTERN = -3,
    TOKEN_END = -4,
    TOKEN_IDENTIFIER = -5,
    TOKEN_NUMBER = -6,
    TOKEN_IF = -7,
    TOKEN_THEN = -8,
    TOKEN_ELSE = -9,
    TOKEN_FOR = -10,
    TOKEN_IN = -11,
    TOKEN_BINARY = -12,
    TOKEN_UNARY = -13,
    TOKEN_OPERATOR = -14,
    TOKEN_GLOBAL = -15
};

// Token Mapping Table
const std::unordered_map<std::string, Token> g_token_mapping = {
    {    "def", TOKEN_DEF    },
    { "extern", TOKEN_EXTERN },
    {    "end", TOKEN_END    },
    {     "if", TOKEN_IF     },
    {   "then", TOKEN_THEN   },
    {   "else", TOKEN_ELSE   },
    {    "for", TOKEN_FOR    },
    {     "in", TOKEN_IN     },
    { "binary", TOKEN_BINARY },
    {  "unary", TOKEN_UNARY  },
    { "global", TOKEN_GLOBAL },
};


/**
 * Function Declare
 */
// extract a token from stdin
int GetToken();

#endif // _H_LEXER

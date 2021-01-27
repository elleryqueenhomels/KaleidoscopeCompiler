#include "lexer.h"
#include "parser.h"


// current token need to be processed
int g_current_token;

int GetNextToken() {
    return g_current_token = GetToken();
}

// numberexpr ::= number
std::unique_ptr<ExprAST> ParseNumberExpr() {
    auto result = std::make_unique<NumberExprAST>(g_number_val);
    GetNextToken();
    return std::move(result);
}

// parenexpr ::= ( expression )
std::unique_ptr<ExprAST> ParseParenExpr() {
    GetNextToken();  // eat (
    auto expr = ParseExpression();
    GetNextToken();  // eat )
    return expr;
}

/// identifierexpr
///   ::= identifier
///   ::= identifier ( expression, expression, ..., expression )
std::unique_ptr<ExprAST> ParseIdentifierExpr() {
    std::string id = g_identifier_str;

    GetNextToken();
    if (g_current_token != '(') { 
        return std::make_unique<VariableExprAST>(id); 
    }

    GetNextToken();  // eat (
    std::vector<std::unique_ptr<ExprAST>> args;
    while (g_current_token != ')') {
        args.push_back(ParseExpression());
        if (g_current_token != ')') {
            GetNextToken();  // eat ,
        }
    }
    GetNextToken();  // eat )

    return std::make_unique<CallExprAST>(id, std::move(args)); 
}

/// primary
///   ::= identifierexpr
///   ::= numberexpr
///   ::= parenexpr
std::unique_ptr<ExprAST> ParsePrimary() {
  switch (g_current_token) {
    case TOKEN_IDENTIFIER: return ParseIdentifierExpr();
    case TOKEN_NUMBER: return ParseNumberExpr();
    case '(': return ParseParenExpr();
    default: return nullptr;
  }
}

// get current token precedence
int GetTokenPrecedence() {
    auto it = g_binop_precedence.find(g_current_token);
    return it == g_binop_precedence.end() ? -1 : it->second;
}

// parse
//   lhs [binop primary] [binop primary] ...
// stop if come across operator whose precedence is less than `min_precedence`
std::unique_ptr<ExprAST> ParseBinOpRhs(int min_precedence, std::unique_ptr<ExprAST> lhs) {
    while (true) {
        int current_precedence = GetTokenPrecedence();
        if (current_precedence < min_precedence) {
            // if current_token is not binop or current_precedence is -1, then stop
            // if come across a operator with lower precedence, also stop
            return lhs;
        }

        int binop = g_current_token;
        GetNextToken();  // eat binop

        auto rhs = ParsePrimary();
        // now we have two possible parsing method
        //   * (lhs binop rhs) binop unparsed
        //   * lhs binop (rhs binop unparsed)
        int next_precedence = GetTokenPrecedence();
        if (current_precedence < next_precedence) {
            // first process the next operator (with higher precedence)
            rhs = ParseBinOpRhs(current_precedence + 1, std::move(rhs));
        }

        lhs = std::make_unique<BinaryExprAST>(binop, std::move(lhs), std::move(rhs)); 
        // continue while-loop
    }
}

// expression
//   ::= primary [binop primary] [binop primary] ...
std::unique_ptr<ExprAST> ParseExpression() {
    auto lhs = ParsePrimary();
    return ParseBinOpRhs(0, std::move(lhs));
}

// prototype
//   ::= id ( id id ... id )
std::unique_ptr<PrototypeAST> ParsePrototype() {
    std::string function_name = g_identifier_str;

    GetNextToken();  // eat (
    std::vector<std::string> arg_names;
    while (GetNextToken() == TOKEN_IDENTIFIER) {
        arg_names.push_back(g_identifier_str);
    }
    GetNextToken();  // eat )

    return std::make_unique<PrototypeAST>(function_name, std::move(arg_names));
}

// definition ::= def prototype expression
std::unique_ptr<FunctionAST> ParseDefinition() {
    GetNextToken();  // eat def
    auto proto = ParsePrototype();
    auto expr = ParseExpression();
    return std::make_unique<FunctionAST>(std::move(proto), std::move(expr));
}

// external ::= extern prototype
std::unique_ptr<PrototypeAST> ParseExtern() {
    GetNextToken();  // eat extern
    return ParsePrototype();
}

// toplevelexpr ::= expression
std::unique_ptr<FunctionAST> ParseTopLevelExpr() {
    auto expr = ParseExpression();
    auto proto = std::make_unique<PrototypeAST>("", std::vector<std::string>());
    return std::make_unique<FunctionAST>(std::move(proto), std::move(expr));
}

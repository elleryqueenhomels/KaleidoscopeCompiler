#include "lexer.h"
#include "parser.h"


// current token need to be processed
int g_current_token;

int GetNextToken() {
    return g_current_token = GetToken();
}

// define precedence for operator
std::unordered_map<std::string, int> g_binop_precedence = {
    { "&&", 5 }, { "||", 5 }, { "==", 10 }, { "!=", 10 },
    { "<", 10 }, { ">", 10 }, { "<=", 10 }, { ">=", 10 },
    { "+", 20 }, { "-", 20 }, {  "*", 40 }, {  "/", 40 },
    { "=",  1 }
};

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
std::unique_ptr<ExprAST> ParseIdentifierExpr(bool is_global_scope) {
    std::string id = g_identifier_str;

    GetNextToken();  // eat identifier
    if (g_current_token != '(') {
        return std::make_unique<VariableExprAST>(id, is_global_scope);
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

/// global identifierexpr
///   ::= global identifier = expression
std::unique_ptr<ExprAST> ParseGlobalIdentifierExpr() {
    GetNextToken(); // eat global
    return ParseIdentifierExpr(true);
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
        case TOKEN_IF: return ParseIfExpr();
        case TOKEN_FOR: return ParseForExpr();
        case TOKEN_OPERATOR: return ParseUnary();
        case TOKEN_GLOBAL: return ParseGlobalIdentifierExpr();
        default: return nullptr;
    }
}

// get current token precedence
int GetOperatorPrecedence() {
    if (g_current_token != TOKEN_OPERATOR) {
        return -1;
    }

    auto it = g_binop_precedence.find(g_operator_str);
    return it == g_binop_precedence.end() ? -1 : it->second;
}

// parse
//   lhs [binop primary] [binop primary] ...
// stop if come across operator whose precedence is less than `min_precedence`
std::unique_ptr<ExprAST> ParseBinOpRhs(int min_precedence, std::unique_ptr<ExprAST> lhs) {
    while (true) {
        int current_precedence = GetOperatorPrecedence();
        if (current_precedence < min_precedence) {
            // if current_token is not binop or current_precedence is -1, then stop
            // if come across a operator with lower precedence, also stop
            return lhs;
        }

        std::string binop = g_operator_str;
        GetNextToken();  // eat binop

        auto rhs = ParsePrimary();
        // now we have two possible parsing method
        //   * (lhs binop rhs) binop unparsed
        //   * lhs binop (rhs binop unparsed)
        int next_precedence = GetOperatorPrecedence();
        if (current_precedence < next_precedence) {
            // first process the next operator (with higher precedence)
            rhs = ParseBinOpRhs(current_precedence + 1, std::move(rhs));
        }

        lhs = std::make_unique<BinaryExprAST>(binop, std::move(lhs), std::move(rhs)); 
        // continue while-loop
    }
}

// unary
//   ::= primary
//   ::= '!' unary
std::unique_ptr<ExprAST> ParseUnary() {
    // if the current token is not an operator, it must be a primary expr
    if (g_current_token != TOKEN_OPERATOR) {
        return ParsePrimary();
    }

    // if this is a unary operator, read it
    std::string unaryop = g_operator_str;
    GetNextToken();  // eat unary op

    if (auto operand = ParseUnary()) {
        return std::make_unique<UnaryExprAST>(unaryop, std::move(operand));
    }

    return nullptr;
}

// expression
//   ::= primary [binop primary] [binop primary] ...
std::unique_ptr<ExprAST> ParseExpression() {
    auto lhs = ParseUnary();
    if (!lhs) {
        return nullptr;
    }

    return ParseBinOpRhs(0, std::move(lhs));
}

// ifexpr
//   ::= if expr then expr else expr
std::unique_ptr<ExprAST> ParseIfExpr() {
    GetNextToken(); // eat if
    std::unique_ptr<ExprAST> cond = ParseExpression();
    GetNextToken(); // eat then
    std::unique_ptr<ExprAST> then_expr = ParseExpression();
    GetNextToken(); // eat else
    std::unique_ptr<ExprAST> else_expr = ParseExpression();
    GetNextToken(); // eat end
    return std::make_unique<IfExprAST>(std::move(cond), std::move(then_expr), std::move(else_expr));
}

// forexpr
//   ::= for var_name = start_expr, end_expr, step_expr in body_expr
std::unique_ptr<ExprAST> ParseForExpr() {
    GetNextToken(); // eat for
    std::string var_name = g_identifier_str;
    GetNextToken(); // eat var_name
    GetNextToken(); // eat =
    std::unique_ptr<ExprAST> start_expr = ParseExpression();
    GetNextToken(); // eat ,
    std::unique_ptr<ExprAST> end_expr = ParseExpression();
    GetNextToken(); // eat ,
    std::unique_ptr<ExprAST> step_expr = ParseExpression();
    GetNextToken(); // eat in
    std::unique_ptr<ExprAST> body_expr = ParseExpression();
    GetNextToken(); // eat end
    return std::make_unique<ForExprAST>(
        var_name, std::move(start_expr), std::move(end_expr), std::move(step_expr), std::move(body_expr));
}

// prototype
//   ::= id ( id id ... id )
std::unique_ptr<PrototypeAST> ParsePrototype() {
    std::string function_name;
    bool is_operator = false;
    int precedence = 0;

    switch (g_current_token) {
        case TOKEN_IDENTIFIER: {
            function_name = g_identifier_str;
            is_operator = false;
            GetNextToken(); // eat id
            break;
        }
        case TOKEN_UNARY: {
            GetNextToken(); // eat unary
            function_name = "unary";
            function_name += g_operator_str;
            is_operator = true;
            GetNextToken(); // eat unary op
            break;
        }
        case TOKEN_BINARY: {
            GetNextToken(); // eat binary
            function_name = "binary";
            function_name += g_operator_str;
            is_operator = true;
            GetNextToken();  // eat binary op
            precedence = g_number_val;
            GetNextToken();  // eat op precedence
            break;
        }
    }

    GetNextToken(); // eat (
    std::vector<std::string> arg_names;
    while (g_current_token != ')') {
        arg_names.push_back(g_identifier_str);
        GetNextToken(); // eat arg
        if (g_current_token == ',') {
            GetNextToken(); // eat ,
        }
    }
    GetNextToken(); // eat )

    return std::make_unique<PrototypeAST>(function_name, arg_names, is_operator, precedence);
}

// definition ::= def prototype expression
std::unique_ptr<FunctionAST> ParseDefinition() {
    GetNextToken();  // eat def
    auto proto = ParsePrototype();
    std::vector<std::unique_ptr<ExprAST>> body;
    while (g_current_token != TOKEN_END) {
        auto expr = ParseExpression();
        body.push_back(std::move(expr));
    }
    GetNextToken();  // eat end
    return std::make_unique<FunctionAST>(std::move(proto), std::move(body));
}

// external ::= extern prototype
std::unique_ptr<PrototypeAST> ParseExtern() {
    GetNextToken();  // eat extern
    return ParsePrototype();
}

// toplevelexpr ::= expression
std::unique_ptr<FunctionAST> ParseTopLevelExpr() {
    auto proto = std::make_unique<PrototypeAST>(top_level_expr_name, std::vector<std::string>());
    auto expr = ParseExpression();
    std::vector<std::unique_ptr<ExprAST>> body;
    body.push_back(std::move(expr));
    return std::make_unique<FunctionAST>(std::move(proto), std::move(body));
}

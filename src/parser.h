#ifndef _H_PARSER
#define _H_PARSER


#include "codegen.h"
#include <string>
#include <unordered_map>
#include <vector>


/**
 * Global Variable Declare
 */
// current token need to be processed
extern int g_current_token;

// define precedence for operator
const std::unordered_map<char, int> g_binop_precedence = {
    { '<', 10 }, { '>', 10 }, { '+', 20 }, { '-', 20 }, { '*', 40 }, { '/', 40 }
};

// symbol for top level expression
const std::string top_level_expr_name = "__anon_expr";


/**
 * CLASS DECLARE
 */
// base class for expression
class ExprAST {
  public:
    virtual ~ExprAST() {}

    virtual llvm::Value* CodeGen() = 0;
};

// number literal expression
class NumberExprAST : public ExprAST {
  public:
    NumberExprAST(double val) : val_(val) {}

    llvm::Value* CodeGen() override;

  private:
    double val_;
};

// variable expression
class VariableExprAST : public ExprAST {
  public:
    VariableExprAST(const std::string& name) : name_(name) {}

    llvm::Value* CodeGen() override;

  private:
    std::string name_;
};

// binary operation expression
class BinaryExprAST : public ExprAST {
  public:
    BinaryExprAST(char op, std::unique_ptr<ExprAST> lhs, std::unique_ptr<ExprAST> rhs)
        : op_(op), lhs_(std::move(lhs)), rhs_(std::move(rhs)) {}

    llvm::Value* CodeGen() override;

  private:
    char op_;
    std::unique_ptr<ExprAST> lhs_;
    std::unique_ptr<ExprAST> rhs_;
};

// function call expression
class CallExprAST : public ExprAST {
  public:
    CallExprAST(const std::string& callee, std::vector<std::unique_ptr<ExprAST>> args)
        : callee_(callee), args_(std::move(args)) {}

    llvm::Value* CodeGen() override;

  private:
    std::string callee_;
    std::vector<std::unique_ptr<ExprAST>> args_;
};

// if then else expression
class IfExprAST : public ExprAST {
  public:
    IfExprAST(std::unique_ptr<ExprAST> cond, std::unique_ptr<ExprAST> then_expr, std::unique_ptr<ExprAST> else_expr)
        : cond_(std::move(cond)), then_expr_(std::move(then_expr)), else_expr_(std::move(else_expr)) {}

    llvm::Value* CodeGen() override;

  private:
    std::unique_ptr<ExprAST> cond_;
    std::unique_ptr<ExprAST> then_expr_;
    std::unique_ptr<ExprAST> else_expr_;
};

// function interface
class PrototypeAST {
  public:
    PrototypeAST(const std::string& name, std::vector<std::string> args)
        : name_(name), args_(std::move(args)) {}
    
    const std::string& name() const { return name_; }

    llvm::Value* CodeGen();

  private:
    std::string name_;
    std::vector<std::string> args_;
};

// function implementation
class FunctionAST {
  public:
    FunctionAST(std::unique_ptr<PrototypeAST> proto, std::unique_ptr<ExprAST> body)
        : proto_(std::move(proto)), body_(std::move(body)) {}
    
    llvm::Value* CodeGen();

  private:
    std::unique_ptr<PrototypeAST> proto_;
    std::unique_ptr<ExprAST> body_;
};


/**
 * Function Declare
 */
// extract next token and store it in `g_current_token`
int GetNextToken();

// get current token precedence
int GetTokenPrecedence();

// numberexpr ::= number
std::unique_ptr<ExprAST> ParseNumberExpr();

// parenexpr ::= ( expression ) 
std::unique_ptr<ExprAST> ParseParenExpr();

// identifierexpr 
//   ::= identifier 
//   ::= identifier ( expression, expression, ..., expression ) 
std::unique_ptr<ExprAST> ParseIdentifierExpr();

// primary 
//   ::= identifierexpr 
//   ::= numberexpr 
//   ::= parenexpr 
std::unique_ptr<ExprAST> ParsePrimary();

// parse 
//   lhs [binop primary] [binop primary] ... 
// stop if come across operator whose precedence is less than `min_precedence`
std::unique_ptr<ExprAST> ParseBinOpRhs(int min_precedence, std::unique_ptr<ExprAST> lhs);

// expression 
//   ::= primary [binop primary] [binop primary] ... 
std::unique_ptr<ExprAST> ParseExpression();

// expression
//   ::= if expr then expr else expr
std::unique_ptr<ExprAST> ParseIfExpr();

// prototype 
//   ::= id ( id id ... id) 
std::unique_ptr<PrototypeAST> ParsePrototype();

// definition ::= def prototype expression 
std::unique_ptr<FunctionAST> ParseDefinition();

// external ::= extern prototype 
std::unique_ptr<PrototypeAST> ParseExtern();

// toplevelexpr ::= expression 
std::unique_ptr<FunctionAST> ParseTopLevelExpr();


#endif // _H_PARSER

//----------------------------------------------------------------------
// FILE: ast_parser.h
// DATE: CPSC 326, Spring 2023
// AUTH:
// DESC:
//----------------------------------------------------------------------

#ifndef AST_PARSER_H
#define AST_PARSER_H

#include "mypl_exception.h"
#include "lexer.h"
#include "ast.h"


class ASTParser
{
public:

  // create a new recursive descent parer
  ASTParser(const Lexer& lexer);

  // run the parser
  Program parse();
  
private:
  
  Lexer lexer;
  Token curr_token;
  
  // helper functions
  void advance();
  void eat(TokenType t, const std::string& msg);
  bool match(TokenType t);
  bool match(std::initializer_list<TokenType> types);
  void error(const std::string& msg);
  bool bin_op();
  bool base_type();
  bool base_rvalue();

  // recursive descent functions
  void struct_def(Program& p);
  void fun_def(Program& p);
  void fields(StructDef& s);
  void class_def(Program& p);
  void class_body(ClassDef& c);
  void class_method(FunDef& f);
  void data_type(VarDef &v);
  void params(FunDef &f);
  void stmt(std::vector<std::shared_ptr<Stmt>> &s);
  void vdecl_stmt(VarDeclStmt& vd);
  void assign_stmt(AssignStmt &s);
  void lvalue(AssignStmt &s);
  void if_stmt(IfStmt &i);
  void if_stmt_t(IfStmt &i);
  void while_stmt(WhileStmt &w);
  void for_stmt(ForStmt &f);
  void call_expr(CallExpr &c);
  void ret_stmt(ReturnStmt &r);
  void expr(Expr &e);
  void rvalue(SimpleTerm &s);
  void new_rvalue(NewRValue &n);
  void var_rvalue(VarRValue &v);

};


#endif

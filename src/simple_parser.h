//----------------------------------------------------------------------
// FILE: simple_parser.h
// DATE: CPSC 326, Spring 2023
// AUTH: Carolyn Bozin
// DESC: Basic interface for our Simple Parser (syntax checker)
//----------------------------------------------------------------------

#ifndef SIMPLE_PARSER_H
#define SIMPLE_PARSER_H

#include "mypl_exception.h"
#include "lexer.h"


class SimpleParser
{
public:

  // crate a new recursive descent parer
  SimpleParser(const Lexer& lexer);

  // run the parser
  void parse();
  
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
  void struct_def();
  void fun_def();
  void class_def();
  void class_body();
  void class_methods();
  void fields();
  void data_type();
  void params();
  void stmt();
  void vdecl_stmt();
  void assign_stmt();
  void lvalue();
  void if_stmt();
  void if_stmt_t();
  void while_stmt();
  void for_stmt();
  void call_expr();
  void ret_stmt();
  void expr();
  void rvalue();
  void new_rvalue();
  void var_rvalue();

};


#endif

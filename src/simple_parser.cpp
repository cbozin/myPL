//----------------------------------------------------------------------
// FILE: simple_parser.cpp
// DATE: CPSC 326, Spring 2023
// AUTH: Carolyn Bozin  
// DESC: Implementation file for SimpleParser class.
// Contains helper functions & recursive descent functions
// for a simple LL(k) parser.
//----------------------------------------------------------------------

#include "simple_parser.h"
#include <iostream>

/* Parametrized constructor taking a Lexer
* object by reference. Assigns given lexer to be the
* current parser's lexer
*/
SimpleParser::SimpleParser(const Lexer& a_lexer)
  : lexer {a_lexer}
{}

/*Input: void
* Output: void
* advances to the next token
*/
void SimpleParser::advance()
{
  curr_token = lexer.next_token();
}

/*Input: token type t, const string & msg
* Output: void
* advances if token matches given type, else throws error
*/
void SimpleParser::eat(TokenType t, const std::string& msg)
{
  if (!match(t))
    error(msg);
  advance();
}

/*Input: Token type t
* Output: bool
* returns true if token matches given token type
*/
bool SimpleParser::match(TokenType t)
{
  return curr_token.type() == t;
}

/*Input: initializer list of Token Types
* Output: bool
* Returns true if token matches any of the tokens types in the list
*/
bool SimpleParser::match(std::initializer_list<TokenType> types)
{
  for (auto t : types)
    if (match(t))
      return true;
  return false;
}

/*Input: const string &msg
* Output: void
* Outputs an error at the current token line & col
*/
void SimpleParser::error(const std::string& msg)
{
  std::string s = msg + " found '" + curr_token.lexeme() + "' ";
  s += "at line " + std::to_string(curr_token.line()) + ", ";
  s += "column " + std::to_string(curr_token.column());
  throw MyPLException::ParserError(s);
}

/*Input: void
* Output: bool
* helper function that returns true if there is a binary operator
*/
bool SimpleParser::bin_op()
{
  return match({TokenType::PLUS, TokenType::MINUS, TokenType::TIMES,
      TokenType::DIVIDE, TokenType::AND, TokenType::OR, TokenType::EQUAL,
      TokenType::LESS, TokenType::GREATER, TokenType::LESS_EQ,
      TokenType::GREATER_EQ, TokenType::NOT_EQUAL});
}

/*Input: void
* Output: bool
* helper function that returns true if there is a base type
*/
bool SimpleParser::base_type(){

  //if int, double, bool, char, or string type, then return true
  return match({TokenType::INT_TYPE, TokenType::DOUBLE_TYPE, TokenType::BOOL_TYPE, 
      TokenType::CHAR_TYPE, TokenType::STRING_TYPE});
  
}

/*Input: void
* Output: bool
* helper function that returns true if there is a base rvalue
*/
bool SimpleParser::base_rvalue(){
  //if int, double, bool, char, or string val, then return true
  return match({TokenType::INT_VAL, TokenType::DOUBLE_VAL, TokenType::BOOL_VAL, 
      TokenType::CHAR_VAL, TokenType::STRING_VAL});

}

/*Input: void
* Output: void
* Function that parses code, looking for struct
* definitions and function definitions.
*/
void SimpleParser::parse()
{
  advance();
  while (!match(TokenType::EOS)) {
    if (match(TokenType::STRUCT))
      struct_def();
    else if(match(TokenType::CLASS))
      class_def();
    else
      fun_def();
  }
  eat(TokenType::EOS, "expecting end-of-file");
}

/*Input: void
* Output: void
* Function that returns without error if
* there is a struct
*/
void SimpleParser::struct_def()
{
  eat(TokenType::STRUCT, "Expected struct reserved word");//if struct key word, eat
  eat(TokenType::ID, "Expected ID");//eat ID
  eat(TokenType::LBRACE, "Expected left brace");// eat left brace
  fields();//call fields function
  eat(TokenType::RBRACE, "Expected right brace");

}

/*Input: void
* Output: void
* Function that returns without error if
* there is a function definition
*/
void SimpleParser::fun_def()
{
  //if void type advance
  if(match(TokenType::VOID_TYPE)){
    advance();

  }else{//otherwise check for datatype
    data_type();

  }
  //check for ID
  eat(TokenType::ID, "Expected ID");
  //check for lparen
  eat(TokenType::LPAREN, "Expected lparen");
  //check for params
  params();
  //check for rparen
  eat(TokenType::RPAREN, "Expected rparen");
  //check for lbrace
  eat(TokenType::LBRACE, "Expected lbrace");
  while(!match(TokenType::RBRACE) && !match(TokenType::EOS)){// if not empty string, loop (checking for EOS)
    stmt();//check for stmt
    
  }
  if(match(TokenType::EOS)){//if EOS, throw error
    error("EOS before rbrace in function");

  }
  //check for rbrace
  eat(TokenType::RBRACE, "Expected rbrace");
}

/*Input: void
* Output: void
* Function that returns without error if
* there are fields
*/
void SimpleParser::fields(){

  if(!match(TokenType::RBRACE)){// if curr token not rbrace, continue
    data_type();//call datattype function

    if(match(TokenType::ID)){//if Id, advance
      advance();

      while(!match(TokenType::RBRACE)){//while curr token is not rbrace, loop

        if(match(TokenType::COMMA)){// if comma, advance
          advance();
          data_type();//check if datatype
          if(match(TokenType::ID)){// if id, advance
            advance();

          }else{//if no ID throw error
            error("Expected ID");
          }
        }else{// if no comma throw error
          error("Expected comma");

        }
      }
    }else{// if not ID, throw error
      error("Expected ID");

    }
  }
}


/*parsing a ClassDef*/
void SimpleParser::class_def(){

  if(!match(TokenType::CLASS)){
    error("class reserved word missing");

  }else{
    advance();
    eat(TokenType::ID, "Expected ID");
    eat(TokenType::LBRACE, "Expected lbrace");
    //parse class body
    class_body();
    eat(TokenType::RBRACE, "Expected rbrace");
  }
}

/*class bodies*/
void SimpleParser::class_body(){

  //check for public or private 
  while(!match(TokenType::RBRACE)){
    if(match({TokenType::PUBLIC, TokenType::PRIVATE})){
      advance();
      eat(TokenType::COLON, "Expected ':'");

      //check stmts
      while(!match({TokenType::PUBLIC, TokenType::PRIVATE, TokenType::RBRACE})){
        //k = 3 (check if method or member)

        //check if void (then must be a method)
        if(match(TokenType::VOID_TYPE)){
          advance();
          //check for ID
          eat(TokenType::ID, "Expected ID");
          class_methods();

        }else{
          data_type();
          eat(TokenType::ID, "Expected ID");
          if(match(TokenType::LPAREN)){
            class_methods();
    
          }
        }
      }
    }
  }
}

void SimpleParser::class_methods(){

  //check for lparen
  eat(TokenType::LPAREN, "Expected lparen");
  //update params vector
  params();
  //check for rparen
  eat(TokenType::RPAREN, "Expected rparen");

  //check for lbrace
  eat(TokenType::LBRACE, "Expected lbrace");
  while(!match(TokenType::RBRACE) && !match(TokenType::EOS)){// if not empty string, loop (checking for EOS)
    stmt();//check for stmt
    
  }
  if(match(TokenType::EOS)){//if EOS, throw error
    error("EOS before rbrace in function");

  }
  //check for rbrace
  eat(TokenType::RBRACE, "Expected rbrace");
}

/*Input: void
* Output: void
* Function that returns without error if
* there is a datatype
*/
void SimpleParser::data_type(){

  if(base_type()){// if base type, eat
    advance();

  }else if(match(TokenType::ID)){//if ID type, eat
    advance();

  }else if(match(TokenType::ARRAY)){//if array keyword, eat
    advance();

    //check for base type, if so advance
    if(base_type()){
      advance();
    
    }else if(match(TokenType::ID)){//else check for ID
      advance();

    }
  }else if(match(TokenType::VOID_TYPE)){// empty string not allowed, throw error
    advance();
    
  }else{
    error("Invalid data type");

  }
}

/*Input: void
* Output: void
* Function that returns without error if
* there are parameters
*/
void SimpleParser::params(){

  if(!match(TokenType::RPAREN)){// if params not empty, continue
    //check for data type
    data_type();
    //eat ID
    eat(TokenType::ID, "Expected ID");

    while(!match(TokenType::RPAREN)){//loop while not at rparen

      //check for comma, datatype, and ID
      eat(TokenType::COMMA, "Expected comma");
      data_type();
      eat(TokenType::ID, "Expected ID");  
    }
  }
}

/*Input: void
* Output: void
* Function that returns without error if
* there is a statement
*/
void SimpleParser::stmt(){
  //check first token
  if(match(TokenType::IF)){//check for IF token
    if_stmt();//check for IF statement

  }else if(match(TokenType::WHILE)){//check for WHILE token
    while_stmt();//check for WHILE statement

  }else if(match(TokenType::FOR)){//check for FOR token
    for_stmt();//check for FOR statement

  }else if(match(TokenType::RETURN)){//check for RETURN token
    ret_stmt();//check for RETURN statement

  }else if(match(TokenType::ID)){//if ID, check further (k = 1)
    advance();//eat ID token

    //check if next token is lparen
    if(match(TokenType::LPAREN)){
      //if curr token is lparen, check for call expr
      call_expr();
      
    }else if(match(TokenType::ID)){
      // if curr token is ID check for vdecl stmt
      vdecl_stmt();

    }else if(match(TokenType::DOT) || match(TokenType::LBRACKET) || match(TokenType::ASSIGN)){
      //if dot, lbracket, or assign token, check for assign stmt
      assign_stmt();
      
    }else{//else throw error
      error("Invalid token after ID in stmt");

    }

  }else{//else, check if data type
    data_type();
    //if datatype check for vdecl stmt
    vdecl_stmt();

  }
}

/*Input: void
* Output: void
* Function that returns without error if
* there is a variable declaration
*/
//ASSUME ON SECOND TOKEN
void SimpleParser::vdecl_stmt(){
  //eat ID token
  eat(TokenType::ID, "Expected ID");
  //eat Assign token
  eat(TokenType::ASSIGN, "Expected assign token");
  //check for expr
  expr();

}

/*Input: void
* Output: void
* Function that returns without error if
* there is an assign statement
*/
void SimpleParser::assign_stmt(){
  //check for lvalue
  lvalue();
  //eat assign token
  eat(TokenType::ASSIGN, "Expected assign token");
  //check for expr
  expr();

}

/*Input: void
* Output: void
* Function that returns without error if
* there is an lvalue
*/
//ASSUME STARTS ON SECOND TOKEN
void SimpleParser::lvalue(){
  
  //while not at ASSIGN, eat
  while(!match(TokenType::ASSIGN)){

    if(match(TokenType::DOT)){// if dot, advance
      advance();
      //eat ID
      eat(TokenType::ID, "Expected ID");
      //check for method

    }else if(match(TokenType::LBRACKET)){// else if lbracket advance
      advance();
      //check for expr
      expr();
      //check for RBRACKET
      eat(TokenType::RBRACKET, "Expected rbracket");

    }else{
      error("Invalid lvalue");

    }
  }
}

/*Input: void
* Output: void
* Function that returns without error if
* there is an if statement
*/
void SimpleParser::if_stmt(){
  //check for IF
  eat(TokenType::IF, "Expected IF token");
  //check for lparen
  eat(TokenType::LPAREN, "Expected lparen");
  //check for expr
  expr();
  //check for rparen
  eat(TokenType::RPAREN, "Expected rparen");
  //check for lbrace
  eat(TokenType::LBRACE, "Expected lbrace");

  //if curr token not RBRACE, loop and check for stmt
  while(!match(TokenType::RBRACE)){
    stmt();

  }
  //check for rbrace
  eat(TokenType::RBRACE, "Expected rbrace");
  //check for if_stmt_t (else if or else)
  if_stmt_t();

}

/*Input: void
* Output: void
* Function that returns without error if
* there is an else or elseif statement
*/
void SimpleParser::if_stmt_t(){

  //check if ELSEIF, if so advance
  if(match(TokenType::ELSEIF)){
    advance();
    //eat LPAREN
    eat(TokenType::LPAREN, "Expected lparen");
    //check for expr
    expr();
    //eat RPAREN
    eat(TokenType::RPAREN, "Expected rparen");
    //eat LBRACE
    eat(TokenType::LBRACE, "Expected lbrace");
    
    //while not at RBRACE, loop
    while(!match(TokenType::RBRACE)){
      stmt();//check for stmt

    }
    advance();//eat RBRACE
    //check for another if_stmt_t
    if_stmt_t();

  }else if(match(TokenType::ELSE)){//check if ELSE
    advance();//eat token
    //check for LBRACE
    eat(TokenType::LBRACE, "Expected lbrace");

    //while not at RBRACE, loop
    while(!match(TokenType::RBRACE)){
      stmt();//check for stmt

    }
    advance();//eat token

  }
}

/*Input: void
* Output: void
* Function that returns without error if
* there is a while statement
*/
void SimpleParser::while_stmt(){
  //eat while token
  eat(TokenType::WHILE, "Expected 'while' token");
  //eat lparen
  eat(TokenType::LPAREN, "Expected lparen");
  //check for expr
  expr();
  //eat rparen
  eat(TokenType::RPAREN, "Expected rparen");
  //eat lbrace
  eat(TokenType::LBRACE, "Expected lbrace");

  //while not rbrace, loop
  while(!match(TokenType::RBRACE)){
    stmt();//check for stmt

  }
  advance();//eat rbrace
}

/*Input: void
* Output: void
* Function that returns without error if
* there is a for statement
*/
void SimpleParser::for_stmt(){
  //eat for token
  eat(TokenType::FOR, "Expected 'for' token");
  //eat lparen
  eat(TokenType::LPAREN, "Expected lparen");
  
  //check for datatype
  data_type();
  //check for vdecl stmt
  vdecl_stmt();
  //eat semicolon
  eat(TokenType::SEMICOLON, "Expected semicolon");
  //check for expr
  expr();
  //eat semicolon
  eat(TokenType::SEMICOLON, "Expected semicolon");
  //eat ID
  eat(TokenType::ID, "Expected ID");
  //check for assign stmt
  assign_stmt();
  //eat rparen
  eat(TokenType::RPAREN, "Expected rparen");
  //eat lbrace
  eat(TokenType::LBRACE, "Expected lbrace");

  //while not at rbrace, loop
  while(!match(TokenType::RBRACE)){
    stmt();//check for stmt

  }
  advance();//eat rbrace

}

/*Input: void
* Output: void
* Function that returns without error if
* there is a call expression
*/
//ASSUME THAT STARTS WITH SECOND TOKEN
void SimpleParser::call_expr(){

  //eat lparen
  eat(TokenType::LPAREN, "Expected lparen");

  if(!match(TokenType::RPAREN)){//if not rparen, check if expr
    expr();

    //while not at rparen, loop
    while(!match(TokenType::RPAREN)){
      //eat comma
      eat(TokenType::COMMA, "Expected comma");
      //check for expr
      expr();
    }
  }
  advance();//eat rparen
}

void SimpleParser::ret_stmt(){
  //check for return token
  eat(TokenType::RETURN, "Expected return token");
  //check for expr
  expr();

}

/*Input: void
* Output: void
* Function that returns without error if
* there is an expression
*/
void SimpleParser::expr(){
  //check if NOT token
  if(match(TokenType::NOT)){
    //eat not token
    advance();
    expr();//check for expr

  }else if(match(TokenType::LPAREN)){//check if lparen
    //eat lparen
    advance();
    //check for expr and rparen
    expr();
    eat(TokenType::RPAREN, "Expected rparen");

  }else{// if not NOT or lparen, check for rvalue
    rvalue();
    
  }
  //check for bin op
  if(bin_op()){
    //eat bin op
    advance();
    //check for expr
    expr();

  }
}

/*Input: void
* Output: void
* Function that returns without error if
* there is an rvalue
*/
void SimpleParser::rvalue(){
  //check for NULL
  if(match(TokenType::NULL_VAL)){
    advance();//eat null

  }else if(match(TokenType::NEW)){//else check for NEW token
    new_rvalue();//check for new rvalue

    
  }else if(match(TokenType::ID)){//else check for ID (k=2)
    advance();//eat ID

    //check if next token is lparen
    if(match(TokenType::LPAREN)){
      //check for call expr
      call_expr();

    }else{//otherwise check for var_rvalue
      var_rvalue();

    }

  }else if(!base_rvalue()){//else if not base rvalue, throw error
    error("Invalid rvalue");

  }
  //if base rvalue, eat
  if(base_rvalue()){
    advance();
    
  }
}

/*Input: void
* Output: void
* Function that returns without error if
* there is a new_rvalue
*/
void SimpleParser::new_rvalue(){
  advance();//eat new token
  
  //check if ID
  if(match(TokenType::ID)){
    advance();//eat ID
    //check for lbracket
    if(match(TokenType::LBRACKET)){
      advance();
      //check for expr
      expr();
      //check for rbracket
      eat(TokenType::RBRACKET, "Expected rbracket");

    }
  }else{//else check for base type
    if(base_type()){
      advance();//eat base type

    }else{//otherwise throw error
      error("Expected base type");

    }
    //eat lbracket
    eat(TokenType::LBRACKET, "Expected lbracket");
    //check for expr
    expr();
    //check for rbracket
    eat(TokenType::RBRACKET, "Expected rbracket");

  }
}

/*Input: void
* Output: void
* Function that returns without error if
* there is a var_rvalue
*/
//ASSUME THAT ON SECOND TOKEN
void SimpleParser::var_rvalue(){

  //loop while next token is dot or lbracket
  while(match({TokenType::DOT, TokenType::LBRACKET})){

    if(match(TokenType::DOT)){// if dot, advance
      advance();
      //eat ID
      eat(TokenType::ID, "Expected ID");

    }else if(match(TokenType::LBRACKET)){// if lbracket, advance
      advance();
      //check for expr
      expr();
      //check for rbracket
      eat(TokenType::RBRACKET, "Expected rbracket");

    }
  }
   //check if lparen (method call)
    if(match(TokenType::LPAREN)){
      call_expr();
    }
}

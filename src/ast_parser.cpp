//----------------------------------------------------------------------
// FILE: ast_parser.cpp
// DATE: CPSC 326, Spring 2023
// AUTH: Carolyn Bozin
// DESC: Implementation file for AST parser.
//----------------------------------------------------------------------

#include "ast_parser.h"
#include <iostream>

using namespace std;


ASTParser::ASTParser(const Lexer& a_lexer)
  : lexer {a_lexer}
{}


void ASTParser::advance()
{
  curr_token = lexer.next_token();
}


void ASTParser::eat(TokenType t, const string& msg)
{
  if (!match(t))
    error(msg);
  advance();
}


bool ASTParser::match(TokenType t)
{
  return curr_token.type() == t;
}


bool ASTParser::match(initializer_list<TokenType> types)
{
  for (auto t : types)
    if (match(t))
      return true;
  return false;
}


void ASTParser::error(const string& msg)
{
  string s = msg + " found '" + curr_token.lexeme() + "' ";
  s += "at line " + to_string(curr_token.line()) + ", ";
  s += "column " + to_string(curr_token.column());
  throw MyPLException::ParserError(s);
}

//helper functions

bool ASTParser::bin_op()
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
bool ASTParser::base_type(){

  //if int, double, bool, char, or string type, then return true
  return match({TokenType::INT_TYPE, TokenType::DOUBLE_TYPE, TokenType::BOOL_TYPE, 
      TokenType::CHAR_TYPE, TokenType::STRING_TYPE});
  
}

/*Input: void
* Output: bool
* helper function that returns true if there is a base rvalue
*/
bool ASTParser::base_rvalue(){
  //if int, double, bool, char, or string val, then return true
  return match({TokenType::INT_VAL, TokenType::DOUBLE_VAL, TokenType::BOOL_VAL, 
      TokenType::CHAR_VAL, TokenType::STRING_VAL});

}


Program ASTParser::parse()
{
  Program p;
  advance();
  while (!match(TokenType::EOS)) {
    if (match(TokenType::STRUCT))
      struct_def(p);
    else if(match(TokenType::CLASS))
      class_def(p);
    else
      fun_def(p);
    //add class methods to fun_defs
    for(auto &c : p.class_defs){
      for(auto &m : c.public_methods){
        p.fun_defs.push_back(m);
        
      }
    }
  }
  eat(TokenType::EOS, "expecting end-of-file");
  return p;
}


// AST recursive descent functions

/*Input: Program &p
* Output: void
* Function that parses struct definition
* and adds to AST
*/
void ASTParser::struct_def(Program& p)
{
  StructDef s;
  eat(TokenType::STRUCT, "Expected struct reserved word");//if struct key word, eat
  s.struct_name = curr_token;// assign struct name to be current token
  eat(TokenType::ID, "Expected ID");//eat ID
  eat(TokenType::LBRACE, "Expected left brace");// eat left brace

  fields(s);//call fields function (passing in StructDef obj)
  eat(TokenType::RBRACE, "Expected right brace"); 
  //pushback to struct defs
  p.struct_defs.push_back(s);
}

/*Input: Program &p
* Output: void
* Function that parses function definition
* and adds to an AST
*/
void ASTParser::fun_def(Program &p)
{ 
  FunDef f;
  //create new VarDef obj
  VarDef v;

  //if void type advance
  if(match(TokenType::VOID_TYPE)){
    //set return type to void
    v.data_type.is_array = false;
    v.data_type.type_name = curr_token.lexeme();
    //return type is given datatype
    f.return_type = v.data_type;
    advance();

  }else{//otherwise check for datatype
    data_type(v);
    //return type is vardef datatype
    f.return_type = v.data_type;

  }

  //assign curr token to func name
  f.fun_name = curr_token;
  //check for ID
  eat(TokenType::ID, "Expected ID");

  //check for lparen
  eat(TokenType::LPAREN, "Expected lparen");
  //update params vector
  params(f);
  //check for rparen
  eat(TokenType::RPAREN, "Expected rparen");

  //check for lbrace
  eat(TokenType::LBRACE, "Expected lbrace");
  while(!match(TokenType::RBRACE) && !match(TokenType::EOS)){// if not empty string, loop (checking for EOS)
    stmt(f.stmts);//check for stmt
    
  }
  if(match(TokenType::EOS)){//if EOS, throw error
    error("EOS before rbrace in function");

  }
  //check for rbrace
  eat(TokenType::RBRACE, "Expected rbrace");
  
  p.fun_defs.push_back(f);
}

/*Input: StructDef &s
* Output: void
* Function that parses struct fields
* and adds to AST
*/
void ASTParser::fields(StructDef& s){

  if(!match(TokenType::RBRACE)){// if curr token not rbrace, continue
    //create VarDef object
    VarDef v;
    data_type(v);//check for datatype and update type

    if(match(TokenType::ID)){//if ID, assign to current vardef name
      v.var_name = curr_token;
      advance();
      //push back vardef
      s.fields.push_back(v);

      while(!match(TokenType::RBRACE)){//while curr token is not rbrace, loop

        if(match(TokenType::COMMA)){// if comma, advance
          advance();
          data_type(v);//check if datatype & update type
          
          if(match(TokenType::ID)){// if id, update vardef
            v.var_name = curr_token;
            advance();

            //push back vardef
            s.fields.push_back(v);

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
void ASTParser::class_def(Program& p){
  //make classdef obj
  ClassDef c;
  if(!match(TokenType::CLASS)){
    error("class reserved word missing");

  }else{
    advance();
    //get class name
    c.class_name = curr_token;
    eat(TokenType::ID, "Expected ID");
    eat(TokenType::LBRACE, "Expected lbrace");
    //parse class body
    class_body(c);
    eat(TokenType::RBRACE, "Expected rbrace");
  }
  //add to class_defs
  p.class_defs.push_back(c);
}

/*class bodies*/
void ASTParser::class_body(ClassDef& c){
  string vis;
  //check for public or private 
  while(!match(TokenType::RBRACE)){
    if(match({TokenType::PUBLIC, TokenType::PRIVATE})){
      vis = curr_token.lexeme();
      advance();
      eat(TokenType::COLON, "Expected ':'");
      //check stmts
      while(!match({TokenType::PUBLIC, TokenType::PRIVATE, TokenType::RBRACE})){
        //save token to peek ahead (k=3)
        Token tmp = curr_token;

        //check if void 
        if(match(TokenType::VOID_TYPE)){
          FunDef f;
          VarDef v;

          //set return type to void
          v.data_type.is_array = false;
          v.data_type.type_name = curr_token.lexeme();
          //return type is given datatype
          f.return_type = v.data_type;
          advance();

          //assign curr token to func name
          f.fun_name = curr_token;
          //check for ID
          eat(TokenType::ID, "Expected ID");
          class_method(f);

          if(vis == "private"){
            c.private_methods.push_back(f);
          }else{
            c.public_methods.push_back(f);
          }

        }else{
          VarDef v;
          data_type(v);
          Token tmp2 = curr_token;//either method or member name
          eat(TokenType::ID, "expected ID");

          if(match(TokenType::LPAREN)){
            FunDef f;
            f.return_type = v.data_type;
            f.fun_name = tmp2;
            class_method(f);
            
            if(vis == "private"){
              c.private_methods.push_back(f);
            }else{
              c.public_methods.push_back(f);
            }
          }else{
            v.var_name = tmp2;
            //check visibility
            if(vis == "private"){
              c.private_members.push_back(v);
            }else{
              c.public_members.push_back(v);
            }
          }
        }
      }
    }
  }
}

void ASTParser::class_method(FunDef& f){

  //check for lparen
  eat(TokenType::LPAREN, "Expected lparen");
  //update params vector
  params(f);
  //check for rparen
  eat(TokenType::RPAREN, "Expected rparen");

  //check for lbrace
  eat(TokenType::LBRACE, "Expected lbrace");
  while(!match(TokenType::RBRACE) && !match(TokenType::EOS)){// if not empty string, loop (checking for EOS)
    stmt(f.stmts);//check for stmt
    
  }
  if(match(TokenType::EOS)){//if EOS, throw error
    error("EOS before rbrace in function");

  }
  //check for rbrace
  eat(TokenType::RBRACE, "Expected rbrace");
}

/*Input: VarDef &v
* Output: void
* Function that parses data type
* and adds to AST
*/
void ASTParser::data_type(VarDef &v){
  //create DataType obj
  DataType d;
  //set bool to false
  d.is_array = false;

  if(base_type()){// if base type, set type name to curr token lexeme
    d.type_name = curr_token.lexeme();
    advance();

  }else if(match(TokenType::ID)){//if ID type, set type name to curr token lexeme
    d.type_name = curr_token.lexeme();
    advance();

  }else if(match(TokenType::ARRAY)){//if array keyword, set bool to true
    d.is_array = true;
    advance();

    //check for base type, if so advance
    if(base_type()){// set type name to curr token lexeme
      d.type_name = curr_token.lexeme();
      advance();

    
    }else if(match(TokenType::ID)){//set type name to curr token lexeme
      d.type_name = curr_token.lexeme();
      advance();

    }

  }else if(match(TokenType::VOID_TYPE)){
    d.type_name = curr_token.lexeme();
    advance();

  }else{
    error("Invalid data type");
  }
  //set curr datatype to be vardef's datatype
  v.data_type = d;
}

/*Input: FunDef &f
* Output: void
* Function that parses function params
* and adds to AST
*/
void ASTParser::params(FunDef &f){

  //new vardef
  VarDef v;

  if(!match(TokenType::RPAREN)){// if params not empty, continue

    //get datatype for vardef
    data_type(v);
    //set curr token to be the var name
    v.var_name = curr_token;
    //eat ID
    eat(TokenType::ID, "Expected ID");
    //add vardef to fundef vector
    f.params.push_back(v);

    while(!match(TokenType::RPAREN)){//loop while not at rparen

      //check for comma, datatype, and ID
      eat(TokenType::COMMA, "Expected comma");
      //get datatype
      data_type(v);
      //set curr token to be var name
      v.var_name = curr_token;
      eat(TokenType::ID, "Expected ID");  
      //add vardef to fundef vector
      f.params.push_back(v);
    }
  }
}

/*Input: vector of shared ptrs to stmts
* Output: void
* Function that parses statements and 
* adds to AST 
*/
void ASTParser::stmt(vector<std::shared_ptr<Stmt>> &stmts){
  //check first token
  if(match(TokenType::IF)){//check for IF token

  //new if stmt
    IfStmt i;
    if_stmt(i);//check for IF statement
    //make ptr tp of stmt
    shared_ptr<IfStmt> i_ptr = make_shared<IfStmt>(i);
    //push back if stmt
    stmts.push_back(i_ptr);

  }else if(match(TokenType::WHILE)){//check for WHILE token

    //create ptr to while stmt
    shared_ptr<WhileStmt> w_ptr = make_shared<WhileStmt>();
    while_stmt(*w_ptr);//check for WHILE statement

    //add while stmt to stmts
    stmts.push_back(w_ptr);

  }else if(match(TokenType::FOR)){//check for FOR token

    //pointer to for_stmt
    shared_ptr<ForStmt> f_ptr = make_shared<ForStmt>();
    for_stmt(*f_ptr);//check for FOR statement

    //add for stmt to stmts
    stmts.push_back(f_ptr);

  }else if(match(TokenType::RETURN)){//check for RETURN token

  //pointer to ret stmt
    shared_ptr<ReturnStmt> r_ptr = make_shared<ReturnStmt>();
    ret_stmt(*r_ptr);//check for RETURN statement
    //add ret stmt to stmts
    stmts.push_back(r_ptr);

  }else if(match(TokenType::ID)){//if ID, check further (k = 1)

    //save curr token
    Token temp = curr_token;
    advance();//eat ID token

    //check if next token is lparen
    if(match(TokenType::LPAREN)){

      //point to callexpr
      shared_ptr<CallExpr> c_ptr = make_shared<CallExpr>();
      //assign temp to fun name
      c_ptr->fun_name = temp;
      call_expr(*c_ptr);

      //add to stmts
      stmts.push_back(c_ptr);
      
    }else if(match(TokenType::ID)){
   
      //new vardecl stmt & varDef
      VarDeclStmt vd;
      VarDef v;

      //set datatype to be previous token
      v.data_type.is_array = false;
      v.data_type.type_name = temp.lexeme();

      //assign vardef to vdecl stmt
      vd.var_def = v;

      //pass in vdecl stmt
      vdecl_stmt(vd);

      //point to vdecl stmt
      shared_ptr<VarDeclStmt> vd_ptr = make_shared<VarDeclStmt>(vd);

      //add vdecl stmt to stmts
      stmts.push_back(vd_ptr);

    }else if(match(TokenType::DOT) || match(TokenType::LBRACKET) || match(TokenType::ASSIGN)){

      //create new AssignStmt
      AssignStmt a;
      // create varref
      VarRef vr;
      vr.var_name = temp;
      a.lvalue.push_back(vr);

      //if dot, lbracket, or assign token, check for assign stmt
      assign_stmt(a);

      //point to vdecl stmt
      shared_ptr<AssignStmt> a_ptr = make_shared<AssignStmt>(a);
      //push back assign stmt
      stmts.push_back(a_ptr);
      
    }else{//else throw error
      error("Invalid token after ID in stmt");

    }

  }else{//else, make vardef and check for data type
    VarDef v;
    //update vardef datatype
    data_type(v);

    //make vardecl stmt
    VarDeclStmt vd;
    //assign vardef to vardecl stmt
    vd.var_def = v;
    //check for vdecl stmt
    vdecl_stmt(vd);
    //point to vdecl stmt
    shared_ptr<VarDeclStmt> vd_ptr = make_shared<VarDeclStmt>(vd);

    //add vdecl stmt to stmts
    stmts.push_back(vd_ptr);

  }
}

/*Input: VarDeclStmt &vd
* Output: void
* Function that parses variable declaration
* statement and adds to AST
*/
//ASSUME ON SECOND TOKEN
void ASTParser::vdecl_stmt(VarDeclStmt &vd){

  //create new expr obj
  Expr e;
  //set curr token to be vardef varname
  vd.var_def.var_name = curr_token;
  //eat ID token
  eat(TokenType::ID, "Expected ID");
  //eat Assign token
  eat(TokenType::ASSIGN, "Expected assign token");
  //add onto expr
  expr(e);

  //add expr to vardeclstmt
  vd.expr = e;
  //add vardecl

}

/*Input: void
* Output: void
* Function that returns without error if
* there is an assign statement
*/
void ASTParser::assign_stmt(AssignStmt &s){

  //check for lvalue
  lvalue(s);
  //eat assign token
  eat(TokenType::ASSIGN, "Expected assign token");
  //check for expr
  expr(s.expr);

}

/*Input: AssignStmt &s
* Output: void
* Function that parses an assign statement
* and adds to AST
*/
//ASSUME STARTS ON SECOND TOKEN
void ASTParser::lvalue(AssignStmt &s){
  
  //while not at ASSIGN, eat
  while(!match(TokenType::ASSIGN)){
    //new VarRef
    VarRef vr;

    if(match(TokenType::DOT)){// if dot, advance
      advance();
      vr.var_name = curr_token;
      //eat ID
      eat(TokenType::ID, "Expected ID");
      //pushback varref
      s.lvalue.push_back(vr);

    }else if(match(TokenType::LBRACKET)){// else if lbracket advance
      advance();
      //new expr
      Expr e;
      //check for expr
      expr(e);
      //update last varref value
      s.lvalue.back().array_expr = e;
      //check for RBRACKET
      eat(TokenType::RBRACKET, "Expected rbracket");

    }else{
      error("Invalid lvalue");

    }
  }
}

/*Input: IfStmt &i
* Output: void
* Function that parses an if statement
* and adds to AST
*/
void ASTParser::if_stmt(IfStmt &i){

  //new BasicIf
  BasicIf b;
  //check for IF
  eat(TokenType::IF, "Expected IF token");
  //check for lparen
  eat(TokenType::LPAREN, "Expected lparen");
  //new expr obj
 
  //check for expr
  expr(b.condition);
  
  //check for rparen
  eat(TokenType::RPAREN, "Expected rparen");
  //check for lbrace
  eat(TokenType::LBRACE, "Expected lbrace");

  //if curr token not RBRACE, loop and check for stmt
  while(!match(TokenType::RBRACE)){
    //append basic if stmts
    stmt(b.stmts);

  }
  //check for rbrace
  eat(TokenType::RBRACE, "Expected rbrace");

  //add basic if to if stmt
  i.if_part = b;
  //check for if_stmt_t (else if or else)
  if_stmt_t(i);

}

/*Input: IfStmt &i
* Output: void
* Function that parses else if/else
* statements and adds to AST
*/
void ASTParser::if_stmt_t(IfStmt &i){

  //check if ELSEIF, if so advance
  if(match(TokenType::ELSEIF)){
    //new basic if
    BasicIf b;    
    advance();
    //eat LPAREN
    eat(TokenType::LPAREN, "Expected lparen");
    //check for condition
    expr(b.condition);
    //eat RPAREN
    eat(TokenType::RPAREN, "Expected rparen");
    //eat LBRACE
    eat(TokenType::LBRACE, "Expected lbrace");
    
    //while not at RBRACE, loop
    while(!match(TokenType::RBRACE)){
      stmt(b.stmts);//check for stmt

    }
    advance();//eat RBRACE
    //add else if stmt to if stmt
    i.else_ifs.push_back(b);
    //check for another if_stmt_t
    if_stmt_t(i);

  }else if(match(TokenType::ELSE)){//check if ELSE
    advance();//eat token
    //check for LBRACE
    eat(TokenType::LBRACE, "Expected lbrace");

    //while not at RBRACE, loop
    while(!match(TokenType::RBRACE)){
      stmt(i.else_stmts);//check for stmt

    }
    advance();//eat token


  }
}

/*Input: WhileStmt &w
* Output: void
* Function that parses while statement
* and adds to AST
*/
void ASTParser::while_stmt(WhileStmt &w){
  //eat while token
  eat(TokenType::WHILE, "Expected 'while' token");
  //eat lparen
  eat(TokenType::LPAREN, "Expected lparen");
  //create new expr
  expr(w.condition);
  //eat rparen
  eat(TokenType::RPAREN, "Expected rparen");
  //eat lbrace
  eat(TokenType::LBRACE, "Expected lbrace");

  //while not rbrace, loop
  while(!match(TokenType::RBRACE)){
    stmt(w.stmts);//check for stmt

  }
  advance();//eat rbrace
}

/*Input: ForStmt &f
* Output: void
* Function that parses for statement
* and adds to AST
*/
void ASTParser::for_stmt(ForStmt &f){

  //eat for token
  eat(TokenType::FOR, "Expected 'for' token");
  //eat lparen
  eat(TokenType::LPAREN, "Expected lparen");

  //new vdeclstmt
  VarDeclStmt vd;
  //new vardef
  VarDef v;
  //check for datatype
  data_type(v);
  //add vardef to vdeclstmt
  vd.var_def = v;

  //check for vdecl stmt
  vdecl_stmt(vd);
  //add vdecl stmt to for stmt
  f.var_decl = vd;

  //eat semicolon
  eat(TokenType::SEMICOLON, "Expected semicolon");
  //new expr obj
  Expr e;
  //check for expr
  expr(e);
  //assign expr to forstmt condition
  f.condition = e;

  //eat semicolon
  eat(TokenType::SEMICOLON, "Expected semicolon");
  //create assign stmt
  AssignStmt a;
  // create varref
  VarRef vr;
  vr.var_name = curr_token;
  a.lvalue.push_back(vr);
  //eat ID
  eat(TokenType::ID, "Expected ID");
  //check assign stmt
  assign_stmt(a);
  //add assign stmt to for stmt
  f.assign_stmt = a;

  //eat rparen
  eat(TokenType::RPAREN, "Expected rparen");
  //eat lbrace
  eat(TokenType::LBRACE, "Expected lbrace");

  //while not at rbrace, loop
  while(!match(TokenType::RBRACE)){
    stmt(f.stmts);//check for stmt

  }
  advance();//eat rbrace

}

/*Input: CallExpr &c
* Output: void
* Function that parses call expression
* and adds to AST
*/
//ASSUME THAT STARTS WITH SECOND TOKEN
void ASTParser::call_expr(CallExpr &c){

  //eat lparen
  eat(TokenType::LPAREN, "Expected lparen");

  if(!match(TokenType::RPAREN)){//if not rparen, check if expr
    //new expr
    Expr e;
    expr(e);
    //append to vector
    c.args.push_back(e);

    //while not at rparen, loop
    while(!match(TokenType::RPAREN)){
      //new expr
      Expr e;
      //eat comma
      eat(TokenType::COMMA, "Expected comma");
      //check for expr
      expr(e);
      //add expr tp vector
      c.args.push_back(e);
    }
  }
  advance();//eat rparen
}

void ASTParser::ret_stmt(ReturnStmt &r){
  //create Expr obj
  Expr e;
  //check for return token
  eat(TokenType::RETURN, "Expected return token");
  //check for expr
  expr(e);
   //asign expr to ret stmt
  r.expr = e;

}

/*Input: Expr &e
* Output: void
* Function that parses expression and
* adds to AST
*/
void ASTParser::expr(Expr &e){

  //check if NOT token
  if(match(TokenType::NOT)){

    //set 'negated' to true
    e.negated = true;
    //eat not token
    advance();

    expr(e);//check for expr
    

  }else if(match(TokenType::LPAREN)){//check if lparen

    //create complexTerm shared ptr
    shared_ptr<ComplexTerm> c_ptr = make_shared<ComplexTerm>();
    //eat lparen
    advance();
    //check for expr, pass in shared ptr's expr
    expr(c_ptr->expr);
    //add shared ptr as 'first' expr in expr e
    e.first = c_ptr;
    eat(TokenType::RPAREN, "Expected rparen");

  }else{// if not NOT or lparen, check for rvalue

    //create simpleterm
    SimpleTerm s;
    rvalue(s);
    //add simple term ptr
    e.first = make_shared<SimpleTerm>(s);
    
  }
  //check for bin op
  if(bin_op()){

    //assign to expr's op
    e.op = curr_token;
    //eat bin op
    advance();
 
    //assign expr r to be rest
    e.rest = make_shared<Expr>();
    //check for expr
    expr(*e.rest);

  }
}

/*Input: SimpleTerm &s
* Output: void
* Function that parses an rvalue and 
* adds to AST
*/
void ASTParser::rvalue(SimpleTerm &s){

  //check for NULL
  if(match(TokenType::NULL_VAL)){

    //create new simple rvalue to point to
    SimpleRValue r;
    r.value = curr_token;
    //point to simple rvalue
    s.rvalue = make_shared<SimpleRValue>(r);
   
    advance();//eat null

  }else if(match(TokenType::NEW)){//else check for NEW token

    NewRValue n;
    new_rvalue(n);//check for new rvalue
    s.rvalue = make_shared<NewRValue>(n);
    
  }else if(match(TokenType::ID)){//else check for ID (k=2)

  //save curr token
    Token temp = curr_token;
   
    advance();//eat ID

    //check if next token is lparen
    if(match(TokenType::LPAREN)){
      //create new call expr
      CallExpr c;
      //assign fun name to be temp token
      c.fun_name = temp;
      //check for call expr
      call_expr(c);
      //point to call expr
      s.rvalue = make_shared<CallExpr>(c);

    }else{//otherwise check for var_rvalue

     //new varrvalue obj
      VarRValue v;
      //create new VarRef, assign var name to be temp
      VarRef vr;
      vr.var_name = temp;
      //add varref to varrvalue
      v.path.push_back(vr);
      var_rvalue(v);
      //add varrvalue to simpleterm
      s.rvalue = make_shared<VarRValue>(v);

    }

  }else if(base_rvalue()){//check for base rvalue
    
    //make new simplervalue
    SimpleRValue r;
    r.value = curr_token;
    //point to simplervalue
    s.rvalue = make_shared<SimpleRValue>(r);
    advance();

  }else {//otherwise throw error
    error("Invalid rvalue");

  }
}

/*Input: NewRValue &n
* Output: void
* Function that parses a new rvalue
* and adds to AST
*/
void ASTParser::new_rvalue(NewRValue &n){
  advance();//eat new token
 
  //check if ID
  if(match(TokenType::ID)){
    //set type to curr token
    n.type = curr_token;
    advance();//eat ID
    //check for lbracket
    if(match(TokenType::LBRACKET)){
      advance();
      //new expr
      Expr e;
      expr(e);
      //assign expr to array expr
      n.array_expr = e;
      //check for rbracket
      eat(TokenType::RBRACKET, "Expected rbracket");

    }
  }else{//else check for base type
    if(base_type()){
      //set type tp curr token
      n.type = curr_token;
      advance();//eat base type

    }else{//otherwise throw error
      error("Expected base type");

    }
    //eat lbracket
    eat(TokenType::LBRACKET, "Expected lbracket");
     //new expr
      Expr e;
      expr(e);
      //assign expr to array expr
      n.array_expr = e;
    //check for rbracket
    eat(TokenType::RBRACKET, "Expected rbracket");

  }

}

/*Input: VarRValue &v
* Output: void
* Function that parses a var rvalue
* and adds to AST
*/
//ASSUME THAT ON SECOND TOKEN
void ASTParser::var_rvalue(VarRValue &v){

  VarRef vr;
  while(match({TokenType::DOT, TokenType::LBRACKET})){

    if(match(TokenType::DOT)){// if dot, advance
      advance();
      vr.var_name = curr_token;
      //eat ID
      eat(TokenType::ID, "Expected ID");
      //add varref to vector
      v.path.push_back(vr);

    }else if(match(TokenType::LBRACKET)){// if lbracket, advance
      advance();
      //check for expr
      Expr e;
      expr(e);
      //add expr to previous varref
      v.path.back().array_expr = e;
      //check for rbracket
      eat(TokenType::RBRACKET, "Expected rbracket");

    }
  }

  //check for method call (only allowed at the end)
  if(match(TokenType::LPAREN)){
    v.path.back().is_method = true;
    advance();

    if(!match(TokenType::RPAREN)){//if not rparen, check if expr
      //new expr
      Expr e;
      expr(e);
      //append to vector
      v.path.back().method_params.push_back(e);

      //while not at rparen, loop
      while(!match(TokenType::RPAREN)){
        //new expr
        Expr e;
        //eat comma
        eat(TokenType::COMMA, "Expected comma");
        //check for expr
        expr(e);
        //add expr tp vector
        v.path.back().method_params.push_back(e);
      }
    }
    advance();//eat rparen
  }
}



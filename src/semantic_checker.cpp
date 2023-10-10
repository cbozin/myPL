//----------------------------------------------------------------------
// FILE: semantic_checker.cpp
// DATE: CPSC 326, Spring 2023
// AUTH: Carolyn Bozin
// DESC: Implementation file for semantic checker
//----------------------------------------------------------------------

#include <unordered_set>
#include "mypl_exception.h"
#include "semantic_checker.h"
#include <iostream>


using namespace std;

// hash table of names of the base data types and built-in functions
const unordered_set<string> BASE_TYPES {"int", "double", "char", "string", "bool"};
const unordered_set<string> BUILT_INS {"print", "input", "to_string",  "to_int",
  "to_double", "length", "get", "concat"};


// helper functions

optional<VarDef> SemanticChecker::get_field(const StructDef& struct_def,
                                            const string& field_name)
{
  for (const VarDef& var_def : struct_def.fields)
    if (var_def.var_name.lexeme() == field_name)
      return var_def;
  return nullopt;
}

optional<VarDef> SemanticChecker::get_member(const ClassDef& class_def, const string& member_name, const string& vis){

  if(vis == "public"){
    for(const VarDef& v : class_def.public_members){
      if(v.var_name.lexeme() == member_name){
        return v;
      }
    }
  }else{
    for(const VarDef& v : class_def.private_members){
      if(v.var_name.lexeme() == member_name){
        return v;
      }
    }
  }
  return nullopt;
}

optional<FunDef> SemanticChecker::get_method(const ClassDef& class_def, const std::string& method_name, const std::string& vis){

  if(vis == "public"){
    for(const FunDef& f : class_def.public_methods){
      if(f.fun_name.lexeme() == method_name){
        return f;
      }
    }
  }else{
    for(const FunDef& f : class_def.private_methods){
      if(f.fun_name.lexeme() == method_name){
        return f;
      }
    }
  }
  return nullopt;
}


void SemanticChecker::error(const string& msg, const Token& token)
{
  string s = msg;
  s += " near line " + to_string(token.line()) + ", ";
  s += "column " + to_string(token.column());
  throw MyPLException::StaticError(s);
}


void SemanticChecker::error(const string& msg)
{
  throw MyPLException::StaticError(msg);
}

// visitor functions


void SemanticChecker::visit(Program& p)
{
  // record each struct def
  for (StructDef& d : p.struct_defs) {
    string name = d.struct_name.lexeme();
    if (struct_defs.count(name))
      error("multiple definitions of '" + name + "'", d.struct_name);
    struct_defs[name] = d;
  }
  // record each function def (need a main function)
  bool found_main = false;
  for (FunDef& f : p.fun_defs) {
    string name = f.fun_name.lexeme();
    if (BUILT_INS.count(name))
      error("redefining built-in function '" + name + "'", f.fun_name);
    //if (fun_defs.count(name))
     // error("multiple definitions of '" + name + "'", f.fun_name);
    if (name == "main") {
      if (f.return_type.type_name != "void")
        error("main function must have void type", f.fun_name);
      if (f.params.size() != 0)
        error("main function cannot have parameters", f.params[0].var_name);
      found_main = true;
    }
    fun_defs[name] = f;
  }
  if (!found_main)
    error("program missing main function");

  //record each class def
  for(ClassDef& c : p.class_defs){
    string name = c.class_name.lexeme();
    if(class_defs.count(name))
      error("multiple definitions of '" + name + "'", c.class_name);
    class_defs[name] = c;
  }

  // check each struct
  for (StructDef& d : p.struct_defs)
    d.accept(*this);
  // check each function
  for (FunDef& d : p.fun_defs)
    d.accept(*this);
  //check eah class
  for (ClassDef& c: p.class_defs)
    c.accept(*this);

}


void SemanticChecker::visit(SimpleRValue& v)
{
  if (v.value.type() == TokenType::INT_VAL)
    curr_type = DataType {false, "int"};
  else if (v.value.type() == TokenType::DOUBLE_VAL)
    curr_type = DataType {false, "double"};    
  else if (v.value.type() == TokenType::CHAR_VAL)
    curr_type = DataType {false, "char"};    
  else if (v.value.type() == TokenType::STRING_VAL)
    curr_type = DataType {false, "string"};    
  else if (v.value.type() == TokenType::BOOL_VAL)
    curr_type = DataType {false, "bool"};    
  else if (v.value.type() == TokenType::NULL_VAL)
    curr_type = DataType {false, "void"};    
}

void SemanticChecker::visit(FunDef& f)
{
  //push new environment
  symbol_table.push_environment();
  //save return type
  DataType return_type = f.return_type;
  //add return type to symbol table
  symbol_table.add("return", return_type);

  //check for undefined struct return type
  if(!BASE_TYPES.count(return_type.type_name) && return_type.type_name != "void"){
    if(!struct_defs.count(return_type.type_name)){
      error("Undefined function return type", f.fun_name);
    }
  }
  //add fun name to fun_defs
  fun_defs[f.fun_name.lexeme()] = f;

  //make table for param names
  unordered_set<string> param_names;

  //iterate through params and add to set
  for(auto &p : f.params){
  
    //check if multiple params have same name
    if(param_names.count(p.var_name.lexeme())){
      error("multiple definitions of param", p.var_name);
    }

    if(p.data_type.type_name == "void"){
      error("Null field in struct", p.first_token());
    }
    //otherwise, insert param
    param_names.insert(p.var_name.lexeme());

    //if param is a struct type, check existing struct defs
    if(!BASE_TYPES.count(p.data_type.type_name) && !struct_defs.count(p.data_type.type_name) && !class_defs.count(p.data_type.type_name)){
      error("Undefined type param", p.first_token());
      
    }
    //check if data type is array
    if(p.data_type.is_array){
      curr_type = DataType {true, p.data_type.type_name};
    }else{
      curr_type = DataType {false, p.data_type.type_name};
    }

    //add to symbol table
    symbol_table.add(p.var_name.lexeme(), curr_type);
  }

  //check each stmt
  for(auto &s : f.stmts){
    s->accept(*this);
  }

  //pop environment
  symbol_table.pop_environment();
}


void SemanticChecker::visit(StructDef& s)
{
  //push new environment
  symbol_table.push_environment();

  //add struct name to struct_defs
  struct_defs[s.struct_name.lexeme()] = s;

  //create unordered set for fields
  unordered_set<string> fields;

  //go through fields
  for(auto &f : s.fields){
    //check if name has already been used
    if(fields.count(f.var_name.lexeme())){
      error("Multiple definitions of field", f.first_token());
    }

    if(f.data_type.type_name == "void"){
      error("Null field in struct", f.first_token());
    }
    //otherwise add to set
    fields.insert(f.var_name.lexeme());

    //if field is a struct type, check existing struct defs
    if(!BASE_TYPES.count(f.data_type.type_name) && !struct_defs.count(f.data_type.type_name)){
      error("Undefined type", f.first_token());
    
    }
  }
  //pop environment
  symbol_table.pop_environment();

}

void SemanticChecker::visit(ClassDef& c){

  //push new env
  symbol_table.push_environment();

  unordered_set<string> members;
  unordered_set<string> methods;

  //add class name to class defs
  class_defs[c.class_name.lexeme()] = c;

  //go through private members
  for(auto m : c.private_members){
    if(members.count(m.var_name.lexeme())){
      error("Multiple definitions of data member", m.var_name);
    }
    if(m.var_name.lexeme() == "void"){
      error("Null data member in class", m.first_token());
    }
    members.insert(m.var_name.lexeme());

    //check if field is struct/class type
    if(!BASE_TYPES.count(m.data_type.type_name)){
      if(!struct_defs.count(m.data_type.type_name)){
        if(!class_defs.count(m.data_type.type_name)){
          error("undefined type member", m.first_token());
        }
      }
    }
  }
  //go through public members
  for(auto m : c.public_members){
    if(members.count(m.var_name.lexeme())){
      error("Multiple definitions of data member", m.first_token());
    }
    if(m.data_type.type_name == "void"){
      error("Null data member in class", m.first_token());
    }
    members.insert(m.var_name.lexeme());

    //check if field is struct/class type
    if(!BASE_TYPES.count(m.data_type.type_name)){
      if(!struct_defs.count(m.data_type.type_name)){
        if(!class_defs.count(m.data_type.type_name)){
          error("undefined type member", m.first_token());
        }
      }
    }
  }
  //go through public methods
  for(auto &m : c.public_methods){

    if(methods.count(m.fun_name.lexeme())){
      error("Multiple definitions of class method", m.fun_name);
    }

    methods.insert(m.fun_name.lexeme());
    //check inside function
    m.accept(*this);
  }
  //go through private methods
  for(auto &m : c.private_methods){

    if(methods.count(m.fun_name.lexeme())){
      error("Multiple definitions of class method", m.fun_name);
    }

    methods.insert(m.fun_name.lexeme());
    //check inside function
    m.accept(*this);
  }
  symbol_table.pop_environment();
}


void SemanticChecker::visit(ReturnStmt& s)
{
  //get return type from symbol table
  DataType return_type = symbol_table.get("return").value();

  //type check return expr
  s.expr.accept(*this);

  //compare return type against curr type
  if(curr_type.type_name != return_type.type_name && curr_type.type_name != "void"){
    error("Incompatible function return type", s.expr.first_token());
    
  } 
  //compare array type
  if(curr_type.is_array != return_type.is_array){
    error("Incompatible array function return type", s.expr.first_token());
  }
}


void SemanticChecker::visit(WhileStmt& s)
{
  //push env
  symbol_table.push_environment();

  //check that condition is bool and is not array
  s.condition.accept(*this);

  if(curr_type.type_name != "bool"){
    error("while stmt condition not bool type", s.condition.first_token());

  }
  if(curr_type.is_array){
    error("while stmt condition is an array", s.condition.first_token());
  }
  //check stmts
  for(auto stmt : s.stmts){
    stmt->accept(*this);
  }
  //pop env
  symbol_table.pop_environment();
}


void SemanticChecker::visit(ForStmt& s)
{ 
  //push env
  symbol_table.push_environment();

  //check that vardecl stmt is type int
  s.var_decl.accept(*this);
  if(curr_type.type_name != "int" && curr_type.type_name != "void"){
    error("non integer in for stmt var decl", s.assign_stmt.expr.first_token());
  }

  //check that condition is bool and is not array
  s.condition.accept(*this);

  if(curr_type.type_name != "bool"){
    error("while stmt condition not bool type", s.condition.first_token());

  }
  if(curr_type.is_array){
    error("while stmt condition is an array", s.condition.first_token());
  }
  //check that assign stmt var is type int
  s.assign_stmt.accept(*this);
  if(curr_type.type_name != "int"){
    error("non integer in for loop assign stmt", s.assign_stmt.expr.first_token());
  }
  //check stmts
  for(auto stmt : s.stmts){
    stmt->accept(*this);
  }
  //pop env
  symbol_table.pop_environment();
}

void SemanticChecker::visit(IfStmt& s)
{

  //check that condition is a bool & is not an array
  s.if_part.condition.accept(*this);

  if(curr_type.type_name != "bool"){
    error("if stmt condition not bool type", s.if_part.condition.first_token());

  }
  if(curr_type.is_array){
    error("if stmt condition is an array", s.if_part.condition.first_token());
  }
  //push new env
  symbol_table.push_environment();

  //for each stmt, type check
  for(auto stmt : s.if_part.stmts){
    stmt->accept(*this);
  }
  //push new env
  symbol_table.pop_environment();

  //if else if part, repeat same steps as for if
  for(auto elseif : s.else_ifs){

    symbol_table.push_environment();

    //check that condition is a bool & is not array
    elseif.condition.accept(*this);

    if(curr_type.type_name != "bool"){
      error("if stmt condition not bool type", elseif.condition.first_token());

    }
    if(curr_type.is_array){
      error("if stmt condition is an array", s.if_part.condition.first_token());
    }
    //for each stmt, type check
    for(auto stmt : elseif.stmts){
      stmt->accept(*this);
    }
    symbol_table.pop_environment();
  }
  //if else part, check stmts
  if(!s.else_stmts.empty()){
    symbol_table.push_environment();
    for(auto else_part : s.else_stmts){
    
      else_part->accept(*this);
    }
      symbol_table.pop_environment();
  }
}


void SemanticChecker::visit(VarDeclStmt& s)
{ 
  DataType lhs_type = s.var_def.data_type;
  //check if datatype exists
  if(!BASE_TYPES.count(lhs_type.type_name)){
    if(!struct_defs.count(lhs_type.type_name)){
      if(!class_defs.count(lhs_type.type_name)){
        error("undefined variable type", s.var_def.first_token());
      }
    }
  }
  //check if var name is in curr env
  if(symbol_table.name_exists_in_curr_env(s.var_def.var_name.lexeme())){
    error("Var previously declared in curr env", s.var_def.var_name);
  }

  //check expr
  s.expr.accept(*this);

  //check if vardecl expr has op
  if(s.expr.op.has_value()){

    string op_val = s.expr.op.value().lexeme();

    //check if expr is comparison, equality, or logical op
    if(op_val != "+" && op_val != "-" && op_val != "*" && op_val != "/"){
      //check that var def has bool type
      if(s.var_def.data_type.type_name != "bool"){
        error("Boolean expr with invalid type", s.var_def.var_name);
      } 
    }
 
    // else check that var def type is same as expr type
  }else if(curr_type.type_name != lhs_type.type_name){
    if(curr_type.type_name != "void"){
      error("Mismatched types in var declaration", s.var_def.var_name);
    
    }
  }

  if(curr_type.type_name != "void" && (curr_type.is_array != lhs_type.is_array)){
    error("Mismatched types in array var declaration", s.var_def.var_name);

  }
  //add var to symbol table
  symbol_table.add(s.var_def.var_name.lexeme(), lhs_type);
}


void SemanticChecker::visit(AssignStmt& s)
{
  DataType lhs_type, prev_type;

  //check lval
  for(int i = 0; i < s.lvalue.size(); i++){

    if(i == 0){
      //check if var exists
      if(!symbol_table.name_exists(s.lvalue[i].var_name.lexeme())){
        error("undefined lhs var in assign stmt", s.expr.first_token());
      }else{
        lhs_type = symbol_table.get(s.lvalue[i].var_name.lexeme()).value();
      }
    }

    if(i > 0){//if longer path check for struct & fields (+ class members/methods)

      prev_type = lhs_type;

      if(struct_defs.count(prev_type.type_name)){
        const StructDef &sd = struct_defs[prev_type.type_name];

        //check field
        if(!get_field(sd, s.lvalue[i].var_name.lexeme())){
          error("Field does not exist in lvalue in assignstmt", s.lvalue[i].var_name);

        }else{
          lhs_type = get_field(sd, s.lvalue[i].var_name.lexeme()).value().data_type;
        }

      }else if(class_defs.count(prev_type.type_name)){
        const ClassDef &cd = class_defs[prev_type.type_name];
 
        //check public members & methods
        if(!s.lvalue[i].is_method){
          if(get_member(cd, s.lvalue[i].var_name.lexeme(), "private")){
            error("member is private", s.lvalue[i].var_name);

          }else if(!get_member(cd, s.lvalue[i].var_name.lexeme(), "public")){
            error("public member does not exist", s.lvalue[i].var_name);

          }else{
            lhs_type = get_member(cd, s.lvalue[i].var_name.lexeme(), "public").value().data_type;
          }
        }else if((s.lvalue[i].is_method)){
          if(get_method(cd, s.lvalue[i].var_name.lexeme(), "private")){
            error("method is private", s.lvalue[i].var_name);

          }else if(!get_method(cd, s.lvalue[i].var_name.lexeme(), "public")){
            error("public method does not exist", s.lvalue[i].var_name);
          }else{
            lhs_type = get_method(cd, s.lvalue[i].var_name.lexeme(), "public").value().return_type;
          }
        }

      }else{
        error("non Struct/class object in assignstmt path", s.lvalue[i - 1].var_name);

      } 
    }
    if(s.lvalue[i].array_expr.has_value()){

      //check that var is array
      if(!lhs_type.is_array){
        error("Non array with array expr in assign stmt", s.expr.first_token());
      }
      //check index is int
      s.lvalue[i].array_expr.value().accept(*this);
      if(curr_type.type_name != "int"){
        error("non int type index in array assign stmt", s.expr.first_token());
      }

      //change is array to false
      lhs_type.is_array = false;
    }
  }
  //check that lhs and rhs match
  s.expr.accept(*this);

  if(lhs_type.type_name != curr_type.type_name) {
    if(curr_type.type_name != "void"){
      error("Mismatched types in assign stmt", s.expr.first_token());

    }else{
      curr_type = {curr_type.is_array, lhs_type.type_name};

    }
  }
  if(lhs_type.is_array != curr_type.is_array){
    error("mismatched array types in assign stmt", s.expr.first_token());
  }
  
}


void SemanticChecker::visit(CallExpr& e)
{
  //store fun name
  string fun_name = e.fun_name.lexeme();

  //compare fun name against built ins
  if(fun_name == "print"){

    //check there is only one arg
    if(e.args.size() != 1){
      error("Wrong number of args in print()", e.first_token());
    }

    e.args[0].accept(*this);

    //set curr type to func return type
    curr_type = DataType{false, "void"};
    
  }else if(fun_name == "input"){
    //check arg sz
    if(e.args.size() != 0){
      error("Args in input()", e.first_token());
    }
    //cur type is string
    curr_type = DataType {false, "string"};

  }else if(fun_name == "to_string"){
    //check arg sz
    if(e.args.size() != 1){
      error("Wrong number of args in to_string()", e.first_token());
    }
    //check that type is int or dbl or char
    e.args[0].accept(*this);

    if(curr_type.type_name != "double" && curr_type.type_name != "int" && curr_type.type_name != "char"){
      error("non int/dbl type arg in to_string()", e.first_token());
    }
    //check that is not array
    if(curr_type.is_array){
      error("Array type arg in to_string()", e.first_token());
    }
    //return type is string
    curr_type = DataType {false, "string"};

  }else if(fun_name == "to_int"){
    //check arg size is 1
    if(e.args.size() != 1){
      error("Wrong number of args in to_int()", e.first_token());
    }

    //check for dbl or str type
    e.args[0].accept(*this);
    if(curr_type.type_name != "string" && curr_type.type_name != "double"){
      error("non string/dbl arg in to_int()", e.first_token());
    }
    //check that is not array
    if(curr_type.is_array){
      error("Array type arg in to_string()", e.first_token());
    }
    //set return type to int
    curr_type = DataType {false, "int"};
    
  }else if(fun_name == "to_double"){
    //check arg size is 1
    if(e.args.size() != 1){
      error("Wrong number of args in to_int()", e.first_token());
    }

    //check for dbl or str type
    e.args[0].accept(*this);
    if(curr_type.type_name != "string" && curr_type.type_name != "int"){
      error("non string/int arg in to_double()", e.first_token());
    }
    //check that is not arrays
    if(curr_type.is_array){
      error("Array type arg in to_double()", e.first_token());
    }
    //set return type to dbl
    curr_type = DataType {false, "double"};
    
  }else if(fun_name == "length"){
    
    //check arg size is 1
    if(e.args.size() != 1){
      error("Wrong number of args in length()", e.first_token());
    }

    //check for dbl or str type
    e.args[0].accept(*this);
  
    //check if is not array
    if(!curr_type.is_array && curr_type.type_name != "string"){
      error("Non array/non string arg in length()", e.first_token());
      
    }

    if(curr_type.is_array){
      e.fun_name.lexeme() = "length@array";
    }
    //set return type to int
    curr_type = DataType {false, "int"};
    
  }else if(fun_name == "get"){
    //check arg size is 2
    if(e.args.size() != 2){
      error("Wrong number of args in get()", e.first_token());
    }

    //check for int and string args
    e.args[0].accept(*this);
    if(curr_type.type_name != "int" || curr_type.is_array){
      error("array or non int arg in get()", e.first_token());
    }

    e.args[1].accept(*this);
    if(curr_type.type_name != "string" || curr_type.is_array){
      error("array or non string arg in get()", e.first_token());
    }

    //set return type to char
    curr_type = DataType {false, "char"};
    
  }else if(fun_name == "concat"){
    //check arg size is 2
    if(e.args.size() != 2){
      error("Wrong number of args in concat()", e.first_token());
    }

    //check for two strings
    e.args[0].accept(*this);
    if(curr_type.type_name != "string" || curr_type.is_array){
      error("array or non string arg in concat()", e.first_token());
    }

    e.args[1].accept(*this);
    if(curr_type.type_name != "string" || curr_type.is_array){
      error("array or non string arg in concat()", e.first_token());
    }

    //set return type to string
    curr_type = DataType {false, "string"};

    
  }else{//user made func

    //check that func is in fundefs vector
    if(!fun_defs.count(e.fun_name.lexeme())){
      error("Undefined function call", e.first_token());
    }
    //grab relevant function
    const FunDef &f = fun_defs[e.fun_name.lexeme()];

    //check that func call has right amnt of params
    if(e.args.size() != f.params.size()){
      error("Function call with wrong number of params", e.first_token());
    }
    //type check params
    for(int i = 0; i < e.args.size(); i++){

      DataType param_type = f.params[i].data_type;
      //check param
      e.args[i].accept(*this);
      //compare param type against curr type

      if(curr_type.type_name != param_type.type_name){
        if(curr_type.type_name != "void"){
          error("Func call param with incorrect type", e.args[i].first_token());
        }
      }
      if(curr_type.is_array != param_type.is_array){
        error("Func call param with incorrect array status", e.args[i].first_token());
      }

    }
    //set curr type to func return type
    curr_type = f.return_type;
  }
}


void SemanticChecker::visit(Expr& e)
{
  //check first part
  e.first->accept(*this);
 
  //set lhstype to curr_type
  DataType lhs_type = curr_type;

  //check if op
  if(e.op.has_value()){
    //check rest
    e.rest->accept(*this);

    //set rhstype to curr_type
    DataType rhs_type = curr_type;

    //store operator str
    string op_val = e.op.value().lexeme();
    
    //check if op is MATHEMATICAL
    if(op_val == "+" || op_val == "-" || op_val == "*" || op_val == "/"){
      //check that lhs type is int or double
      if(lhs_type.type_name != "int" && lhs_type.type_name != "double"){
        error("Illegal var type in math expr", e.op.value());
      }
      //check that lhs and rhs are not arrays
      if(lhs_type.is_array || rhs_type.is_array){
        error("Array type in math expr", e.first_token());
      }
      //check if 'rest' has an op
      if(e.rest->op.has_value()){
        string op_val = e.rest->op.value().lexeme();
        //check if op is math op
        if(op_val != "+" && op_val != "-" && op_val != "*" && op_val != "/"){
          error("Non mathematical operator in math expr", e.rest->op.value());
        }
      }
    }else if(op_val == "<" || op_val == ">" || op_val == "<=" || op_val == ">="){//COMPARISON ops
      //check that lhs is int, double, char, string
      if(lhs_type.type_name != "int" && lhs_type.type_name != "double" && lhs_type.type_name != "char" && lhs_type.type_name != "string"){
        error("Illegal var type in comparison expr", e.op.value());

      }
      //check that lhs and rhs not arrays
      if(lhs_type.is_array || rhs_type.is_array){
        error("Array type in comparison expr", e.first_token());
      }
       //check if 'rest' has an op
      if(e.rest->op.has_value()){
        string op_val = e.rest->op.value().lexeme();
        //check if op is math op
        // if(op_val == "+" || op_val == "-" || op_val == "*" || op_val == "/" || op_val == "and" || op_val == "or"){
        //   error("Invalid operator in comparison expr", e.rest->op.value());
        // }
      }
      //set curr type to bool
      curr_type = DataType {false, "bool"};

    }else if(op_val == "==" || op_val == "!="){//EQUALITY ops
      //check if types are same
      if(lhs_type.type_name != rhs_type.type_name){
        //if not same, check if one or both are void
        if(!(lhs_type.type_name != "void" || rhs_type.type_name != "void")){
          //if neither are void, throw error
          error("Invalid type in equality expr", e.op.value());

        }
      }
      //check if 'rest' has an op
      if(e.rest->op.has_value()){
        string op_val = e.rest->op.value().lexeme();
        //check if op is math op (if yes throw error)
        if(op_val == "+" || op_val == "-" || op_val == "*" || op_val == "/" || op_val == "and" || op_val == "or"){
          error("Invalid operator in equality expr", e.rest->op.value());
        }
      }
      //set curr type to bool
      curr_type = DataType {false, "bool"};

    }else if(op_val == "and" || op_val == "or"){//check for LOGICAL ops

      //check if 'rest' has op
      if(e.rest->op.has_value()){
        string op_val = e.rest->op.value().lexeme();
        //check if op is logical op
        if(op_val == "+" || op_val == "-" || op_val == "*" || op_val == "/"){
          error("Invalid operator in logical expr", e.rest->op.value());
        }
      }
      //set curr type to bool
      curr_type = DataType {false, "bool"};
    }

    //skip for equality ops (one side could be void)
    if(op_val != "==" && op_val != "!="){
      //check if rhs_type and lhs_type are the same!
      if(lhs_type.type_name != rhs_type.type_name){
        error("Mismatched types in expr", e.op.value());
      }
    }

    if(e.negated){
      //check that expr type is bool
      if(curr_type.type_name != "bool"){
        error("Non bool negated expr", e.first_token());
      }
    }
  }
}


void SemanticChecker::visit(SimpleTerm& t)
{ 
  //check rvalue
  t.rvalue->accept(*this);
} 


void SemanticChecker::visit(ComplexTerm& t)
{
  //check expr
  t.expr.accept(*this);
  //check if complex term has logical, comparison, or equality ops
  if(t.expr.op.has_value()){
    string op_val = t.expr.op.value().lexeme();
    if(op_val != "+" && op_val != "-" && op_val != "*" && op_val != "/"){
      curr_type = DataType {false, "bool"};
    }
  }
}


void SemanticChecker::visit(NewRValue& v)
{ 
  //set curr type, check if array
  if(v.array_expr.has_value()){
    
    v.array_expr.value().accept(*this); 

    if(curr_type.type_name != "int"){
      error("non int array length in new array", v.first_token());
    }else{
      curr_type = DataType {true, v.type.lexeme()};
    }

  }else{
    curr_type = DataType {false, v.type.lexeme()};
  }
  
}


void SemanticChecker::visit(VarRValue& v)
{
  DataType prev_type, rhs_type;
  
  //check lval
  for(int i = 0; i < v.path.size(); i++){

    if(i == 0){
      if(!symbol_table.name_exists(v.path[i].var_name.lexeme())){
        error("undefined var in varrval path", v.first_token());

      }else{
        rhs_type = symbol_table.get(v.path[i].var_name.lexeme()).value();
      }
    }
    if(i > 0){

      prev_type = rhs_type;

      if(struct_defs.count(prev_type.type_name)){
        const StructDef &sd = struct_defs[prev_type.type_name];

        //check field
        if(!get_field(sd, v.path[i].var_name.lexeme())){
          error("Field does not exist in varrval path", v.path[i].var_name);

        }else{
          rhs_type = get_field(sd, v.path[i].var_name.lexeme()).value().data_type;
        }
      }else if(class_defs.count(prev_type.type_name)){
        const ClassDef &cd = class_defs[prev_type.type_name];
 
        //check public members & methods
        if(!v.path[i].is_method){
          if(get_member(cd, v.path[i].var_name.lexeme(), "private")){
            error("member is private", v.path[i].var_name);

          }else if(!get_member(cd, v.path[i].var_name.lexeme(), "public")){
            error("public member does not exist", v.path[i].var_name);

          }else{
            rhs_type = get_member(cd, v.path[i].var_name.lexeme(), "public").value().data_type;
          }
        }else if((v.path[i].is_method)){
          if(get_method(cd, v.path[i].var_name.lexeme(), "private")){
            error("method is private", v.path[i].var_name);

          }else if(!get_method(cd, v.path[i].var_name.lexeme(), "public")){
            error("public method does not exist", v.path[i].var_name);
          }else{
            rhs_type = get_method(cd, v.path[i].var_name.lexeme(), "public").value().return_type;
          }
        }

      }else{
        error("non Struct/Class object in varrval path", v.path[i - 1].var_name);

      } 
    }
  }
  if(v.path.back().array_expr.has_value()){

    //check that var is array
    if(!rhs_type.is_array){
      error("Non array with array expr in varrvalue", v.first_token());
    }
    //check index is int
    v.path.back().array_expr.value().accept(*this);
    if(curr_type.type_name != "int"){
      error("non int type index in rvarvalue", v.first_token());
    }

    //change is array to false
    rhs_type.is_array = false;
  }
  curr_type = rhs_type;
}    


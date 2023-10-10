//----------------------------------------------------------------------
// FILE: code_generator.cpp
// DATE: CPSC 326, Spring 2023
// AUTH: Carolyn Bozin
// DESC: Implementation file for code generator
//----------------------------------------------------------------------

#include <iostream>  
#include <unordered_set>           // for debugging
#include "code_generator.h"

using namespace std;

// helper function to replace all occurrences of old string with new
void replace_all(string& s, const string& old_str, const string& new_str)
{
  while (s.find(old_str) != string::npos)
    s.replace(s.find(old_str), old_str.size(), new_str);
}


CodeGenerator::CodeGenerator(VM& vm)
  : vm(vm)
{
}


void CodeGenerator::visit(Program& p)
{
  for (auto& struct_def : p.struct_defs)
    struct_def.accept(*this);
  for (auto& class_def : p.class_defs)
    class_def.accept(*this);
  for (auto& fun_def : p.fun_defs)
    fun_def.accept(*this);
}


void CodeGenerator::visit(FunDef& f)
{ 
  //set curr frame
  curr_frame = VMFrameInfo {f.fun_name.lexeme(), (int)f.params.size()};
  //push new env
  var_table.push_environment();

  //store each arg passed
  for(int i = 0; i < f.params.size(); i++){
    curr_frame.instructions.push_back(VMInstr::STORE(i));
    //add to var table
    var_table.add(f.params[i].var_name.lexeme());
  }

  //visit body stmts
  for(auto& s : f.stmts){
    s->accept(*this);
  }
  if(f.stmts.size() > 0){
    //check if last instruction is a return
    if(to_string(curr_frame.instructions.back()) != "RET()"){
      curr_frame.instructions.push_back(VMInstr::PUSH(nullptr));
      curr_frame.instructions.push_back(VMInstr::RET());

    }
  }else {//if no stmts push null and return
    curr_frame.instructions.push_back(VMInstr::PUSH(nullptr));
    curr_frame.instructions.push_back(VMInstr::RET());

  }
  //pop env
  var_table.pop_environment();

  //add frame info to vm
  vm.add(curr_frame);


}

void CodeGenerator::visit(StructDef& s)
{
  struct_defs[s.struct_name.lexeme()] = s;//add struct to map
}

void CodeGenerator::visit(ClassDef& c)
{
  class_defs[c.class_name.lexeme()] = c;//add class to map
}

void CodeGenerator::visit(ReturnStmt& s)
{
  //generate code for expr
  s.expr.accept(*this);

  curr_frame.instructions.push_back(VMInstr::RET());
}


void CodeGenerator::visit(WhileStmt& s)
{
  int i = curr_frame.instructions.size(); //current instr index
  //check condition
  s.condition.accept(*this);
  curr_frame.instructions.push_back(VMInstr::JMPF(10)); //dummy index
  int j = curr_frame.instructions.size() - 1;
  var_table.push_environment();
  for(auto &st : s.stmts){
    st->accept(*this);//visit stmts
  }
  var_table.pop_environment();
  curr_frame.instructions.push_back(VMInstr::JMP(i));//jmp to start
  curr_frame.instructions.push_back(VMInstr::NOP());//nop for end of loop
  int k = curr_frame.instructions.size(); //get current instr index
  curr_frame.instructions.at(j) = VMInstr::JMPF(k);//update JMPF
}


void CodeGenerator::visit(ForStmt& s)
{
  //push env for 1st vardecl
  var_table.push_environment();
  //visit vardecl
  s.var_decl.accept(*this);
  int i = curr_frame.instructions.size();
  //check condition
  s.condition.accept(*this);
  curr_frame.instructions.push_back(VMInstr::JMPF(10));//dummy index
  int j = curr_frame.instructions.size() - 1;
  //push env
  var_table.push_environment();
  //visit stmts
  for(auto &st : s.stmts){
    st->accept(*this);
  }
  var_table.pop_environment();
  //visit assign stmt
  s.assign_stmt.accept(*this);
  var_table.pop_environment();
  curr_frame.instructions.push_back(VMInstr::JMP(i));//jmp to start
  curr_frame.instructions.push_back(VMInstr::NOP());// for end of loop
  int k = curr_frame.instructions.size();//get curr instr index
  curr_frame.instructions.at(j) = VMInstr::JMPF(k);//update jmpf
}


void CodeGenerator::visit(IfStmt& s)
{
  //create vector for jmp indeces
  vector<int> jmps(1);
  //check if cond
  s.if_part.condition.accept(*this);
  //add jmpf
  curr_frame.instructions.push_back(VMInstr::JMPF(10));//dummy val
   //get index
  int i = curr_frame.instructions.size() - 1;
  //visit stmts
  for(auto &st : s.if_part.stmts){
    st->accept(*this);
  }
  //add jmp with dummy val
  curr_frame.instructions.push_back(VMInstr::JMP(10));
  jmps.push_back(curr_frame.instructions.size() - 1);

  //check for elseifs
  for(auto &e : s.else_ifs){
    curr_frame.instructions.push_back(VMInstr::NOP());
    int j = curr_frame.instructions.size() - 1;
    //update previous jmpf
    curr_frame.instructions.at(i) = VMInstr::JMPF(j);
    e.condition.accept(*this);
    //add jmpf
    curr_frame.instructions.push_back(VMInstr::JMPF(10));//dummy index
    i = curr_frame.instructions.size() - 1;
    //visit stmts
    for(auto &st : e.stmts){
      st->accept(*this);
    }
    //add jmp with dummy val
    curr_frame.instructions.push_back(VMInstr::JMP(10));
    jmps.push_back(curr_frame.instructions.size() - 1);
  }
  //check for elses
  if(!s.else_stmts.empty()){
    //add nop
    curr_frame.instructions.push_back(VMInstr::NOP());
    int j = curr_frame.instructions.size() - 1;
    //update previous jmpf
    curr_frame.instructions.at(i) = VMInstr::JMPF(j);
    for(auto &st : s.else_stmts){
      st->accept(*this);
    }
  }
  curr_frame.instructions.push_back(VMInstr::NOP());//after if/elses
  int k = curr_frame.instructions.size() - 1;
  //update jmpf if no elses
  if(s.else_ifs.empty() && s.else_stmts.empty()){
    curr_frame.instructions.at(i) = VMInstr::JMPF(k);
  }
  //update jmps
  for(int i = 0; i < jmps.size(); i++){
    curr_frame.instructions.at(jmps.back()) = VMInstr::JMP(k);
    jmps.pop_back();
  }
}


void CodeGenerator::visit(VarDeclStmt& s)
{ 
  //visit expr
  s.expr.accept(*this);
  //TODO: remove next var index

  //add to var table
  var_table.add(s.var_def.var_name.lexeme());

  //add store instr for next ind
  curr_frame.instructions.push_back(VMInstr::STORE(var_table.get(s.var_def.var_name.lexeme())));
}


void CodeGenerator::visit(AssignStmt& s)
{
  //get index of var
  int i = var_table.get(s.lvalue[0].var_name.lexeme());
  //load var
  curr_frame.instructions.push_back(VMInstr::LOAD(i));
  
  if(s.lvalue[0].array_expr.has_value()){
    //visit array expr
    s.lvalue[0].array_expr.value().accept(*this);
    //check if more values
    if(s.lvalue.size() > 1){
      curr_frame.instructions.push_back(VMInstr::GETI());
    }
  }
  
  //get fields
  for(int i = 1; i < s.lvalue.size() - 1; i++){
    curr_frame.instructions.push_back(VMInstr::DUP());
    if(s.lvalue[i].array_expr.has_value()){
      curr_frame.instructions.push_back(VMInstr::GETF(s.lvalue[i].var_name.lexeme()));
      //visit array expr
      s.lvalue[i].array_expr.value().accept(*this);

      curr_frame.instructions.push_back(VMInstr::GETI());

    }else if(struct_defs.count(s.lvalue[i - 1].var_name.lexeme())){
      
      curr_frame.instructions.push_back(VMInstr::GETF(s.lvalue[i].var_name.lexeme()));
    }else{

      curr_frame.instructions.push_back(VMInstr::GETMEM(s.lvalue[i].var_name.lexeme()));
    }
  }

  if(s.lvalue.size() != 1 && s.lvalue.back().array_expr.has_value()){
    curr_frame.instructions.push_back(VMInstr::GETF(s.lvalue.back().var_name.lexeme()));
    s.lvalue.back().array_expr.value().accept(*this);

  }
  //else if(s.lvalue.size() != 1){
  //   curr_frame.instructions.push_back(VMInstr::GETMEM(s.lvalue.back().var_name.lexeme()));
  // }

  //visit expr
  s.expr.accept(*this);

  if(s.lvalue.back().array_expr.has_value()){
    
    curr_frame.instructions.push_back(VMInstr::SETI());

  }else if(s.lvalue.size() > 1){
    if(struct_defs.count(s.lvalue[s.lvalue.size() - 2].var_name.lexeme())){
      
      curr_frame.instructions.push_back(VMInstr::SETF(s.lvalue[s.lvalue.size() - 1].var_name.lexeme()));
    }else{
      curr_frame.instructions.push_back(VMInstr::SETMEM(s.lvalue[s.lvalue.size() - 1].var_name.lexeme()));
    }
    
  }else{
    //store var
    curr_frame.instructions.push_back(VMInstr::STORE(i));
    
  }
}


void CodeGenerator::visit(CallExpr& e)
{
  //go through args
  for(auto &a : e.args){
    a.accept(*this);
  }

  string f = e.fun_name.lexeme();
  //check for built in func
  if(f == "print"){
    curr_frame.instructions.push_back(VMInstr::WRITE());

  }else if(f == "input"){
    curr_frame.instructions.push_back(VMInstr::READ());
    
  }else if(f == "to_string"){
    curr_frame.instructions.push_back(VMInstr::TOSTR());

  }else if(f == "to_int"){
    curr_frame.instructions.push_back(VMInstr::TOINT());

  }else if(f == "to_double"){
    curr_frame.instructions.push_back(VMInstr::TODBL());

  }else if(f == "length"){
    //get slen
    curr_frame.instructions.push_back(VMInstr::SLEN());

  }else if(f == "length@array"){
    //get array len
    curr_frame.instructions.push_back(VMInstr::ALEN());
  
  }else if(f == "get"){
    curr_frame.instructions.push_back(VMInstr::GETC());

  }else if(f == "concat"){
    curr_frame.instructions.push_back(VMInstr::CONCAT());

  }else{
    //call function
    curr_frame.instructions.push_back(VMInstr::CALL(e.fun_name.lexeme()));
  } 
}


void CodeGenerator::visit(Expr& e)
{
  //visit first
  e.first->accept(*this);

  if(e.op.has_value()){
    //visit rest
    e.rest->accept(*this);

    Token op_val = e.op.value();
    //check which op
    if(op_val.type() == TokenType::PLUS){
      curr_frame.instructions.push_back(VMInstr::ADD());

    }else if(op_val.type() == TokenType::MINUS){
      curr_frame.instructions.push_back(VMInstr::SUB());
      
    }else if(op_val.type() == TokenType::DIVIDE){
      curr_frame.instructions.push_back(VMInstr::DIV());
      
    }else if(op_val.type() == TokenType::TIMES){
      curr_frame.instructions.push_back(VMInstr::MUL());
      
    }else if(op_val.type() == TokenType::AND){
      curr_frame.instructions.push_back(VMInstr::AND());
      
    }else if(op_val.type() == TokenType::OR){
      curr_frame.instructions.push_back(VMInstr::OR());
      
    }else if(op_val.type() == TokenType::LESS){
      curr_frame.instructions.push_back(VMInstr::CMPLT());
      
    }else if(op_val.type() == TokenType::GREATER){
      curr_frame.instructions.push_back(VMInstr::CMPGT());
      
    }else if(op_val.type() == TokenType::LESS_EQ){
      curr_frame.instructions.push_back(VMInstr::CMPLE());
      
    }else if(op_val.type() == TokenType::GREATER_EQ){
      curr_frame.instructions.push_back(VMInstr::CMPGE());
      
    }else if(op_val.type() == TokenType::EQUAL){
      curr_frame.instructions.push_back(VMInstr::CMPEQ());
      
    }else if(op_val.type() == TokenType::NOT_EQUAL){
      curr_frame.instructions.push_back(VMInstr::CMPNE());
      
    }
  }
  //check if negated
  if(e.negated){
    curr_frame.instructions.push_back(VMInstr::NOT());
  }
}


void CodeGenerator::visit(SimpleTerm& t)
{
  t.rvalue->accept(*this);
}
 

void CodeGenerator::visit(ComplexTerm& t)
{
  t.expr.accept(*this);
}

void CodeGenerator::visit(SimpleRValue& v)
{
  //check type of token
  if(v.value.type() == TokenType::INT_VAL){
    int val = stoi(v.value.lexeme());
    curr_frame.instructions.push_back(VMInstr::PUSH(val));

  }else if(v.value.type() == TokenType::DOUBLE_VAL){
    double val = stod(v.value.lexeme());
    curr_frame.instructions.push_back(VMInstr::PUSH(val));

  }else if(v.value.type() == TokenType::STRING_VAL || v.value.type() == TokenType::CHAR_VAL){
    string val = v.value.lexeme();
    replace_all(val, "\\n", "\n");
    replace_all(val, "\\t", "\t");
    curr_frame.instructions.push_back(VMInstr::PUSH(val));
    
  }else if(v.value.type() == TokenType::BOOL_VAL){
    if(v.value.lexeme() == "true"){
      curr_frame.instructions.push_back(VMInstr::PUSH(true));
    }else{
      curr_frame.instructions.push_back(VMInstr::PUSH(false));
    }
   
  }else{//null
    curr_frame.instructions.push_back(VMInstr::PUSH(nullptr));
  }
}


void CodeGenerator::visit(NewRValue& v)
{
  if(v.array_expr.has_value()){//array
    //get sz
    v.array_expr.value().accept(*this);
    //init to null vall
    curr_frame.instructions.push_back(VMInstr::PUSH(nullptr));
    //create and add ALLOCA
    curr_frame.instructions.push_back(VMInstr::ALLOCA());

  }else if(struct_defs.count(v.type.lexeme())){//struct
    //create and add ALLOCS
    curr_frame.instructions.push_back(VMInstr::ALLOCS());

    //get struct from map
    StructDef &s = struct_defs[v.type.lexeme()];

    for(auto &f : s.fields){

      curr_frame.instructions.push_back(VMInstr::DUP());//dup obj id
      curr_frame.instructions.push_back(VMInstr::ADDF(f.var_name.lexeme()));//add field
      curr_frame.instructions.push_back(VMInstr::DUP());//dup obj id
      curr_frame.instructions.push_back(VMInstr::PUSH(nullptr));
      curr_frame.instructions.push_back(VMInstr::SETF(f.var_name.lexeme()));//set field to null
    }
  }else{
    //create and add ALLOCC
    curr_frame.instructions.push_back(VMInstr::ALLOCC());

    //get class from map
    ClassDef &c = class_defs[v.type.lexeme()];

    for(auto &m : c.private_members){
      curr_frame.instructions.push_back(VMInstr::DUP());//dup obj id
      curr_frame.instructions.push_back(VMInstr::ADDMEM(m.var_name.lexeme()));//add field
      curr_frame.instructions.push_back(VMInstr::DUP());//dup obj id
      curr_frame.instructions.push_back(VMInstr::PUSH(nullptr));
      curr_frame.instructions.push_back(VMInstr::SETMEM(m.var_name.lexeme()));//set field to null
    }

    for(auto &m : c.public_members){
      curr_frame.instructions.push_back(VMInstr::DUP());//dup obj id
      curr_frame.instructions.push_back(VMInstr::ADDMEM(m.var_name.lexeme()));//add field
      curr_frame.instructions.push_back(VMInstr::DUP());//dup obj id
      curr_frame.instructions.push_back(VMInstr::PUSH(nullptr));
      curr_frame.instructions.push_back(VMInstr::SETMEM(m.var_name.lexeme()));//set field to null
    }

    for(auto &m : c.private_methods){
      
    }

    for(auto &m : c.public_methods){
      
    }
  }
}


void CodeGenerator::visit(VarRValue& v)
{
  //get var index
  int i = var_table.get(v.path[0].var_name.lexeme());
  //generate load instr
  curr_frame.instructions.push_back(VMInstr::LOAD(i));

  //check for array expr
  if(v.path[0].array_expr.has_value()){
    v.path[0].array_expr.value().accept(*this);
    curr_frame.instructions.push_back(VMInstr::GETI());
  }

  //get fields and/or class members
  for(int i = 1; i < v.path.size(); i++){
    curr_frame.instructions.push_back(VMInstr::DUP());

    if(v.path[i].array_expr.has_value()){
      curr_frame.instructions.push_back(VMInstr::GETF(v.path[i].var_name.lexeme()));
      //visit array expr
      v.path[i].array_expr.value().accept(*this);
      //get i
      curr_frame.instructions.push_back(VMInstr::GETI());
    }else if(struct_defs.count(v.path[i - 1].var_name.lexeme())){

      curr_frame.instructions.push_back(VMInstr::GETF(v.path[i].var_name.lexeme()));
    }else{
      curr_frame.instructions.push_back(VMInstr::GETMEM(v.path[i].var_name.lexeme()));
    }
  }
  //check for method call
  if(v.path.back().is_method){
    curr_frame.instructions.push_back(VMInstr::CALL(v.path.back().var_name.lexeme()));
  }
}
  


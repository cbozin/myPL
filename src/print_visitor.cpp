//----------------------------------------------------------------------
// FILE: print_visitor.cpp
// DATE: CPSC 326, Spring 2023
// AUTH: Carolyn Bozin
// DESC: Program that pretty prints myPL code through navigation
//       of an AST. Utilizes ast classes.
//----------------------------------------------------------------------

#include "print_visitor.h"
#include <iostream>

using namespace std;


PrintVisitor::PrintVisitor(ostream& output)
  : out(output)
{
}


void PrintVisitor::inc_indent()
{
  indent += INDENT_AMT;
}


void PrintVisitor::dec_indent()
{
  indent -= INDENT_AMT;
}


void PrintVisitor::print_indent()
{
  out << string(indent, ' ');
}


void PrintVisitor::visit(Program& p)
{
  inc_indent();//increment indent amount once

  for (auto struct_def : p.struct_defs)
    struct_def.accept(*this);
  for (auto class_def : p.class_defs)
    class_def.accept(*this);
  for (auto fun_def : p.fun_defs)
    fun_def.accept(*this);
}


void PrintVisitor::visit(FunDef& f)
{ cout << "\n";
  //print return type and func name
  cout << f.return_type.type_name << " ";
  cout << f.fun_name.lexeme() << "(";

  if(f.params.size() > 0){
    //print params
    for(int i = 0; i < f.params.size() - 1; i++){
      cout << f.params[i].data_type.type_name << " " << f.params[i].var_name.lexeme();
      cout << ", ";
    }

    cout << f.params.back().data_type.type_name << " " << f.params.back().var_name.lexeme();
  }
  cout << ") {\n";

  //print stmts in func body
  for(auto stmt : f.stmts){
    print_indent();
    stmt->accept(*this);
    cout << "\n";
  }

  cout << "}\n";

}

void PrintVisitor::visit(StructDef& s)
{  cout << endl;
  //print struct keyword
  cout << "struct ";
  //print struct name
  cout << s.struct_name.lexeme() << " {\n"; 

  if(s.fields.size() > 0){
    //print out struct fields
    for(int i = 0; i < s.fields.size() - 1; i++){
      //indent
      print_indent();
      cout << s.fields[i].data_type.type_name << " ";
      cout << s.fields[i].var_name.lexeme() << ",\n";
      
    }
    print_indent();
    cout << s.fields.back().data_type.type_name << " ";
    cout << s.fields.back().var_name.lexeme() << "\n";
  }
  cout << "}\n";
}

void PrintVisitor::visit(ClassDef& c)
{
  cout << endl;
  cout << "class ";
  cout << c.class_name.lexeme() << "{\n";

  if(c.private_members.size() > 0 || c.private_methods.size() > 0){
    cout << "private:\n";
  }
  //print all private members
  for(auto m : c.private_members){
    print_indent();
    cout << m.data_type.type_name << " " << m.var_name.lexeme() << endl;
  }
  //print all private methods
  for(auto m: c.private_methods){
    print_indent();
    m.accept(*this);
  }
  //print all public members & methods
  if(c.public_members.size() > 0 || c.public_methods.size() > 0){
    cout << "public:\n";
  }
  for(auto m : c.public_members){
    print_indent();
    cout << m.data_type.type_name << " " << m.var_name.lexeme() << endl;

  }
  for(auto m: c.public_methods){
    print_indent();
    m.accept(*this);
  }
  cout << "\n}\n";
}

void PrintVisitor::visit(ReturnStmt& s)
{
 //print return token
  cout << "return ";

  //visit expr
  s.expr.accept(*this);
}

void PrintVisitor::visit(WhileStmt& s)
{
  cout << "while (";
  //print condition
  s.condition.accept(*this);
  cout << ") ";

  //print body (& indent)
  cout << "{\n";
  inc_indent();
  for(auto stmt : s.stmts){
    print_indent();
    stmt->accept(*this);
    cout << "\n";
  }
  dec_indent();
  print_indent();
  cout << "}";
 
}

void PrintVisitor::visit(ForStmt& s)
{
  cout << "for (";
  //print vardecl
  s.var_decl.accept(*this);
  cout << "; ";

  //print condition
  s.condition.accept(*this);
  cout << "; ";

  //print assign stmt
  s.assign_stmt.accept(*this);
  cout << ") {\n";

  //indent
  inc_indent();
  //print body
  for(auto stmt : s.stmts){
    print_indent();
    //call stmt to print itself
    stmt->accept(*this);
  }
  cout << "\n";
  dec_indent();
  print_indent();
  cout << "}";
  
}

void PrintVisitor::visit(IfStmt& s)
{
  cout << "if (";
 //print basic if
  s.if_part.condition.accept(*this);
  cout << ") {\n";

  inc_indent();
  //print stmts
  for(auto stmt : s.if_part.stmts){
    print_indent();
    stmt->accept(*this);
    cout << "\n";
  }
  dec_indent();
  print_indent();
  cout << "}";
  //if else ifs or elses, add newline
  if(s.else_ifs.size() > 0 || s.else_stmts.size() > 0){
    cout << "\n";
  }
 
  if(s.else_ifs.size() > 0){//check if else ifs are not empty
    for(auto else_if : s.else_ifs){
      print_indent();
      cout << "elseif (";
      //print condition
      else_if.condition.accept(*this);
      cout << ") {\n";

      inc_indent();
      //print stmts
      for(auto stmt : else_if.stmts){
        print_indent();
        stmt->accept(*this);
        cout << "\n";
      }
      dec_indent();
      print_indent();
      cout << "}";
      
      //print newline if elses
      if(s.else_stmts.size() > 0){
        cout << "\n";
      }

    }
  }
  //check for else stmt
  if(s.else_stmts.size() > 0){
    print_indent();
    cout << "else {\n";
    inc_indent();
    //go through else stmts
    for(auto else_stmt : s.else_stmts){
      print_indent();
      else_stmt->accept(*this);
      cout << "\n";

    }
    dec_indent();
    print_indent();
    cout << "}";
  }
}

void PrintVisitor::visit(VarDeclStmt& s)
{
  //print vardef datatype
  cout << s.var_def.data_type.type_name << " ";
  //print vardef token
  cout << s.var_def.var_name.lexeme() << " ";

  //print assign token
  cout << "= ";

  //visit expr
  s.expr.accept(*this);
}

void PrintVisitor::visit(AssignStmt& s)
{
  //print lvalues
  for(int i = 0; i < s.lvalue.size() - 1; i++){
    cout << s.lvalue[i].var_name.lexeme();
    //check if array
    if(s.lvalue[i].array_expr.has_value()){
      cout << "[";
      s.lvalue[i].array_expr.value().accept(*this);
      cout << "].";

    }else{
      cout << ".";
    }
  }
  cout << s.lvalue.back().var_name.lexeme();
  if(s.lvalue.back().array_expr.has_value()){
    cout << "[";
    s.lvalue.back().array_expr.value().accept(*this);
    cout << "] ";
  }else{
    cout << " ";
  }

 //print assign token
  cout << "= ";
 //visit expr
  s.expr.accept(*this);
}

void PrintVisitor::visit(CallExpr& e)
{ 
  //print func name
  cout << e.fun_name.lexeme() << "(";
  if(e.args.size() > 0){
    //print func args
    for(int i = 0; i < e.args.size() - 1; i++){
      //visit exprs
      e.args[i].accept(*this);
      cout << ", ";
      
    }
    e.args.back().accept(*this);
  }
  cout << ")";
 
}

void PrintVisitor::visit(Expr& e)
{  
  //if negated print 'not'
  if(e.negated){
    cout << "not (";
  }

  //visit expr term
  if(e.first != nullptr){

    //visit first expr term
    e.first->accept(*this);

    //if op has val print op
    if(e.op.has_value()){
      cout << " " <<  e.op.value().lexeme() << " ";
      //print last expr
      e.rest->accept(*this);
    }
  }else{// if nullptr, print 'null'
    cout << "null";
    
  }
  //if negated, add right paren
  if(e.negated){
    cout << ")";
  }
}

void PrintVisitor::visit(SimpleTerm& t)
{ 
  //visit rvalue
  t.rvalue->accept(*this);
  
}

void PrintVisitor::visit(ComplexTerm& t)
{ 
  //print parens
  cout << "(";

  //visit expr
  t.expr.accept(*this);

  cout << ")";
  
}

void PrintVisitor::visit(SimpleRValue& v)
{ 
  //if string val, add quotation marks
  if(v.value.type() == TokenType::STRING_VAL){
    cout << "\"" << v.value.lexeme() << "\"";

  }else if(v.value.type() == TokenType::CHAR_VAL){
    //if char val add single quotes
    cout << "'" << v.value.lexeme() << "'";
  }else{
    cout << v.value.lexeme();

  }
}

void PrintVisitor::visit(NewRValue& v)
{
  //print 'new'
  cout << "new ";
  //print type
  cout << v.type.lexeme();
  //print array exprs if applicable
  if(v.array_expr.has_value()){
    cout << "[";
    v.array_expr->accept(*this);
    cout << "]";
    
  }
}

void PrintVisitor::visit(VarRValue& v)
{ 
  //for each varref in path, print
  for(int i = 0; i < v.path.size() - 1; i++){
    cout << v.path[i].var_name.lexeme();
    //if array, print array expr
    if(v.path[i].array_expr.has_value()){
      cout << "[";
      v.path[i].array_expr->accept(*this);
      cout << "]";

    }
    cout << ".";
  }
  cout << v.path.back().var_name.lexeme();
  //if array, print array expr
  if(v.path.back().array_expr.has_value()){
    cout << "[";
    v.path.back().array_expr->accept(*this);
    cout << "]";

  }
  //check if method call
  if(v.path.back().is_method){
    cout << "(";
    if(v.path.back().method_params.size() > 0){
      for(int i = 0; i < v.path.back().method_params.size() - 1; i++){
        v.path.back().method_params[i].value().accept(*this);
        cout << ", "; 
      }
      v.path.back().method_params.back().value().accept(*this);
    }
    cout << ")";
  }
}

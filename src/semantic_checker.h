//----------------------------------------------------------------------
// FILE: static_checker.h
// DATE: Spring 2023
// AUTH: S. Bowers
// DESC: Class API for the MyPL semantic checker.
//----------------------------------------------------------------------

#ifndef SEMANTIC_CHECKER_H
#define SEMANTIC_CHECKER_H

#include <unordered_map>
#include "ast.h"
#include "symbol_table.h"


class SemanticChecker : public Visitor
{
public:

  // visitor functions
  void visit(Program& p);
  void visit(FunDef& f);
  void visit(StructDef& s);
  void visit(ClassDef& c);
  void visit(ReturnStmt& s);
  void visit(WhileStmt& s);
  void visit(ForStmt& s);
  void visit(IfStmt& s);
  void visit(VarDeclStmt& s);
  void visit(AssignStmt& s);
  void visit(CallExpr& e);
  void visit(Expr& e);
  void visit(SimpleTerm& t); 
  void visit(ComplexTerm& t);
  void visit(SimpleRValue& v);
  void visit(NewRValue& v);
  void visit(VarRValue& v);    

private:

  // symbol table
  SymbolTable symbol_table;

  // current inferred type
  DataType curr_type;

  // mapping from struct names to corresponding ast objects
  std::unordered_map<std::string, StructDef> struct_defs;

  // mapping from function names to corresponding ast objects
  std::unordered_map<std::string, FunDef> fun_defs;

  //mapping from class names to corresponding ast objects
  std::unordered_map<std::string, ClassDef> class_defs;

  // helper function to get field in struct def
  std::optional<VarDef> get_field(const StructDef& struct_def,
                                  const std::string& field_name);
  // helper function to get member in class_def
  std::optional<VarDef> get_member(const ClassDef&, const std::string& member_name, const std::string& visibility);
  //helper function to get method name in class def
  std::optional<FunDef> get_method(const ClassDef&, const std::string& method_name, const std::string& visibility);

  // error helper functions
  void error(const std::string& msg, const Token& token);
  void error(const std::string& msg);

};

#endif

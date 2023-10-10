//----------------------------------------------------------------------
// FILE: class_tests.cpp
// DATE: CPSC 326, Spring 2023
// AUTH: Carolyn Bozin
// DESC: myPL class tests
//----------------------------------------------------------------------

#include <iostream>
#include <sstream>
#include <string>
#include <gtest/gtest.h>
#include "mypl_exception.h"
#include "lexer.h"
#include "simple_parser.h"
#include "ast_parser.h"
#include "semantic_checker.h"
#include "vm.h"
#include "vm_frame.h"
#include "code_generator.h"

using namespace std;


streambuf* stream_buffer;


void change_cout(stringstream& out)
{
  stream_buffer = cout.rdbuf();
  cout.rdbuf(out.rdbuf());
}

void restore_cout()
{
  cout.rdbuf(stream_buffer);
}

string build_string(initializer_list<string> strs)
{
  string result = "";
  for (string s : strs)
    result += s + "\n";
  return result;
}


//----------------------------------------------------------------------
// Simple lexer tests
//----------------------------------------------------------------------
TEST(BasicClassTests, TypeGivenIsReturned) {
  Token token1(TokenType::CLASS, "class", 0, 0);
  Token token2(TokenType::PUBLIC, "public", 0, 0);
  Token token3(TokenType::PRIVATE, "private", 0, 0);
  Token token4(TokenType::COLON, ":", 0, 0);

  ASSERT_EQ(TokenType::CLASS, token1.type());
  ASSERT_EQ(TokenType::PUBLIC, token2.type());
  ASSERT_EQ(TokenType::PRIVATE, token3.type());
  ASSERT_EQ(TokenType::COLON, token4.type());

}

TEST(BasicClassTests, LexemeGivenIsReturned) {
  Token token1(TokenType::CLASS, "class", 0, 0);
  Token token2(TokenType::PUBLIC, "public", 0, 0);
  Token token3(TokenType::PRIVATE, "private", 0, 0);
  Token token4(TokenType::COLON, ":", 0, 0);

  ASSERT_EQ("class", token1.lexeme());
  ASSERT_EQ("public", token2.lexeme());
  ASSERT_EQ("private", token3.lexeme());
  ASSERT_EQ(":", token4.lexeme());
}

//----------------------------------------------------------------------
// Simple parser tests
//----------------------------------------------------------------------

TEST(BasicClassTests, EmptyClassSimple) {
  stringstream in("class F {}");
  SimpleParser(Lexer(in)).parse();  
}

TEST(BasicClassTests, ClassSimple) {
  stringstream in("class F {private:}");
  SimpleParser(Lexer(in)).parse();  
}

TEST(BasicClassTests, ClassSimple2) {
  stringstream in("class F {public:}");
  SimpleParser(Lexer(in)).parse();  
}

TEST(BasicClassTests, ClassSimple3) {
  stringstream in("class F {public: private:}");
  SimpleParser(Lexer(in)).parse();  
}

TEST(BasicClassTests, ClassSimple4) {
  stringstream in("class F {public: private: public:}");
  SimpleParser(Lexer(in)).parse();  
}

TEST(BasicClassTests, ClassSimpleMembers) {
   stringstream in(build_string({
        "class F {",
        "  private:",
        "    int x",
        "    bool b",
        "  public: ",
        "   string s",
        "}"
      }));
  SimpleParser(Lexer(in)).parse();  
}

TEST(BasicClassTests, ClassSimpleMethods) {
   stringstream in(build_string({
        "class F {",
        "  private:",
        "    void func() {}",
        "  public: ",
        "    int func2() {return 1}",
        "}"
      }));
  SimpleParser(Lexer(in)).parse();  
}


TEST(BasicClassTests, ClassSimpleMethodsAndMembers) {
   stringstream in(build_string({
        "class F {",
        "  private:",
        "    void func() {}",
        "     int z",
        "  public: ",
        "    int func2() {return 1}",
        "     string str",
        "}"
      }));
  SimpleParser(Lexer(in)).parse();  
}


TEST(BasicClassTests, SimpleClassInstantiation) {
   stringstream in(build_string({
        "class F {",
        "  private:",
        "    void func() {}",
        "     int z",
        "  public: ",
        "    int func2() {return 1}",
        "     string str",
        "}",
        " void main() {",
        "    F f = new F",
        "}"
      }));
  SimpleParser(Lexer(in)).parse();  
}


TEST(BasicClassTests, SimpleClassMemberRval) {
   stringstream in(build_string({
        "class F {",
        "  public: ",
        "    int x",
        "}",
        " void main() {",
        "    F f = new F",
        "    int y = f.x"
        "}"
      }));
  SimpleParser(Lexer(in)).parse();  
}

TEST(BasicClassTests, SimpleClassMemberLval) {
   stringstream in(build_string({
        "class F {",
        "  public: ",
        "    int x",
        "}",
        " void main() {",
        "    F f = new F",
        "    f.x = 0"
        "}"
      }));
  SimpleParser(Lexer(in)).parse();  
}

TEST(BasicClassTests, SimpleClassMethodCallNoParams) {
   stringstream in(build_string({
        "class F {",
        "  public: ",
        "    int fun() {return 0}",
        "}",
        " void main() {",
        "    F f = new F",
        "    int y = f.fun()",
        "}"
      }));
  SimpleParser(Lexer(in)).parse();  
}

TEST(BasicClassTests, SimpleClassMethodCallWithParams) {
   stringstream in(build_string({
        "class F {",
        "  public: ",
        "    int fun(double db, bool b) {return 0}",
        "}",
        " void main() {",
        "    F f = new F",
        "    int y = f.fun(4.5, true)",
        "}"
      }));
  SimpleParser(Lexer(in)).parse();  
}


TEST(BasicClassTests, BadSimpleClassMethodCallWithParams) {
   stringstream in(build_string({
        "class F {",
        "  public: ",
        "    int fun(double db, bool b) {return 0}",
        "}",
        " void main() {",
        "    F f = new F",
        "    int y = f.fun(4.5, true).x",
        "}"
      }));
    try {
    SimpleParser(Lexer(in)).parse();
    FAIL();
    }
    catch(MyPLException& e) {
      string msg = e.what();
      ASSERT_EQ("Parser Error: ", msg.substr(0, 14));
    }
}

//----------------------------------------------------------------------
// AST parser tests
//----------------------------------------------------------------------

TEST(BasicClassTests, ASTEmptyInput) {
  stringstream in("");
  Program p = ASTParser(Lexer(in)).parse();
  ASSERT_EQ(0, p.fun_defs.size());
  ASSERT_EQ(0, p.struct_defs.size());
  ASSERT_EQ(0, p.class_defs.size());
}

TEST(BasicClassTests, ASTEmptyClass) {
  stringstream in("class C {}");
  Program p = ASTParser(Lexer(in)).parse();
  ASSERT_EQ(1, p.class_defs.size());
}

TEST(BasicClassTests, ASTSimpleClass) {
  stringstream in("class C {private: }");
  Program p = ASTParser(Lexer(in)).parse();
  ASSERT_EQ(1, p.class_defs.size());
}

TEST(BasicClassTests, ASTSimpleASTClass2) {
  stringstream in("class C {public: }");
  Program p = ASTParser(Lexer(in)).parse();
  ASSERT_EQ(1, p.class_defs.size());
}

TEST(BasicClassTests, ASTSimpleClass3) {
  stringstream in("class C {public: private:}");
  Program p = ASTParser(Lexer(in)).parse();
  ASSERT_EQ(1, p.class_defs.size());
}

TEST(BasicClassTests, ASTTwoSimpleClasses) {
  stringstream in("class C {} class K {}");
  Program p = ASTParser(Lexer(in)).parse();
  ASSERT_EQ(2, p.class_defs.size());
}

 
TEST(BasicClassTests, ASTClassPublicMember) {
  stringstream in("class C {public: int x}");
  Program p = ASTParser(Lexer(in)).parse();
  ClassDef &c = p.class_defs[0];
  ASSERT_EQ(1, p.class_defs.size());
  ASSERT_EQ(0, c.private_members.size());
  ASSERT_EQ(1, c.public_members.size());
  ASSERT_EQ(0, c.private_methods.size());
  ASSERT_EQ(0, c.public_methods.size());
}

TEST(BasicClassTests, ASTClassPrivateMember) {
  stringstream in("class C {private: int x}");
  Program p = ASTParser(Lexer(in)).parse();
  ClassDef &c = p.class_defs[0];
  ASSERT_EQ(1, p.class_defs.size());
  ASSERT_EQ(1, c.private_members.size());
  ASSERT_EQ(0, c.public_members.size());
  ASSERT_EQ(0, c.private_methods.size());
  ASSERT_EQ(0, c.public_methods.size());
}

TEST(BasicClassTests, ASTClassPrivateAndPublicMember) {
  stringstream in(build_string({
        "class F {",
        "  private:",
        "     int z",
        "  public: ",
        "     string str",
        "}"
      }));
  Program p = ASTParser(Lexer(in)).parse();
  ClassDef &c = p.class_defs[0];
  ASSERT_EQ(1, p.class_defs.size());
  ASSERT_EQ(1, c.private_members.size());
  ASSERT_EQ(1, c.public_members.size());
  ASSERT_EQ(0, c.private_methods.size());
  ASSERT_EQ(0, c.public_methods.size());
}

TEST(BasicClassTests, ASTClassPrivateMethod) {
  stringstream in(build_string({
        "class F {",
        "  private:",
        "     void f() {}",
        "}"
      }));
  Program p = ASTParser(Lexer(in)).parse();
  ClassDef &c = p.class_defs[0];
  ASSERT_EQ(1, p.class_defs.size());
  ASSERT_EQ(0, c.private_members.size());
  ASSERT_EQ(0, c.public_members.size());
  ASSERT_EQ(1, c.private_methods.size());
  ASSERT_EQ(0, c.public_methods.size());
}

TEST(BasicClassTests, ASTClassPublicMethod) {
  stringstream in(build_string({
        "class F {",
        "  public:",
        "     void f() {}",
        "}"
      }));
  Program p = ASTParser(Lexer(in)).parse();
  ClassDef &c = p.class_defs[0];
  ASSERT_EQ(1, p.class_defs.size());
  ASSERT_EQ(0, c.private_members.size());
  ASSERT_EQ(0, c.public_members.size());
  ASSERT_EQ(0, c.private_methods.size());
  ASSERT_EQ(1, c.public_methods.size());
}

TEST(BasicClassTests, ASTClassMemberRvalPublic) {
  stringstream in(build_string({
        "class F {",
        "  public:",
        "     double d",
        "}",
        "void main() {",
        "    F f = new F",
        "    double x = f.d",
        "}"
      }));
  Program p = ASTParser(Lexer(in)).parse();
  ClassDef &c = p.class_defs[0];
  ASSERT_EQ(1, p.class_defs.size());
  ASSERT_EQ(0, c.private_members.size());
  ASSERT_EQ(1, c.public_members.size());
  ASSERT_EQ(0, c.private_methods.size());
  ASSERT_EQ(0, c.public_methods.size());
  ASSERT_EQ(2, p.fun_defs[0].stmts.size());
}


TEST(BasicClassTests, ASTClassMemberLvalPublic) {
  stringstream in(build_string({
        "class F {",
        "  public:",
        "     double d",
        "}",
        "void main() {",
        "    F f = new F",
        "    f.d = 9.5",
        "}"
      }));
  Program p = ASTParser(Lexer(in)).parse();
  ClassDef &c = p.class_defs[0];
  ASSERT_EQ(1, p.class_defs.size());
  ASSERT_EQ(0, c.private_members.size());
  ASSERT_EQ(1, c.public_members.size());
  ASSERT_EQ(0, c.private_methods.size());
  ASSERT_EQ(0, c.public_methods.size());
  ASSERT_EQ(2, p.fun_defs[0].stmts.size());
}

TEST(BasicClassTests, ASTClassMethodCallPublic) {
  stringstream in(build_string({
        "class F {",
        "  public:",
        "     double d() {return 9.5}",
        "}",
        "void main() {",
        "    F f = new F",
        "    double x = f.d()",
        "}"
      }));
  Program p = ASTParser(Lexer(in)).parse();
  ClassDef &c = p.class_defs[0];
  ASSERT_EQ(1, p.class_defs.size());
  ASSERT_EQ(0, c.private_members.size());
  ASSERT_EQ(0, c.public_members.size());
  ASSERT_EQ(0, c.private_methods.size());
  ASSERT_EQ(1, c.public_methods.size());
  ASSERT_EQ(1, p.fun_defs[0].stmts.size());
  ASSERT_EQ(2, p.fun_defs[1].stmts.size());
}


TEST(BasicClassTests, ASTClassMethodCallPublicWithParams) {
  stringstream in(build_string({
        "class F {",
        "  public:",
        "     double d(int x, bool b) {return 9.5}",
        "}",
        "void main() {",
        "    F f = new F",
        "    double x = f.d(1, false)",
        "}"
      }));
  Program p = ASTParser(Lexer(in)).parse();
  ClassDef &c = p.class_defs[0];
  ASSERT_EQ(1, p.class_defs.size());
  ASSERT_EQ(0, c.private_members.size());
  ASSERT_EQ(0, c.public_members.size());
  ASSERT_EQ(0, c.private_methods.size());
  ASSERT_EQ(1, c.public_methods.size());
  ASSERT_EQ(1, p.fun_defs[0].stmts.size());
  ASSERT_EQ(2, p.fun_defs[1].stmts.size());
}



//----------------------------------------------------------------------
// Semantic Checker tests
//----------------------------------------------------------------------


TEST(BasicClassTests, NonExistantClassObject) {
  stringstream in(build_string({
        "void main() {",
        "  myClass m = new myClass",
        "}",
      }));
  SemanticChecker checker;
  try {
    ASTParser(Lexer(in)).parse().accept(checker);
    FAIL();
  } catch(MyPLException& ex) {
    string msg = ex.what();
    ASSERT_TRUE(msg.starts_with("Static Error:"));
  }
}


TEST(BasicClassTests, ExistantClassObject) {
  stringstream in(build_string({
        "class C {}",
        "void main() {",
        "  C c = new C",
        "}",
      }));
  SemanticChecker checker;
  ASTParser(Lexer(in)).parse().accept(checker);
}


TEST(BasicClassTests, DuplicateClassMembersPublic) {
  stringstream in(build_string({
        "class C {",
        " public:",
        "   int x",
        "   int x",
        "}",
        "void main() {}"
      }));
  SemanticChecker checker;
  try {
    ASTParser(Lexer(in)).parse().accept(checker);
    FAIL();
  } catch(MyPLException& ex) {
    string msg = ex.what();
    ASSERT_TRUE(msg.starts_with("Static Error:"));
  }
}

TEST(BasicClassTests, DuplicateClassMembersPrivate) {
  stringstream in(build_string({
        "class C {",
        " private:",
        "   int x",
        "   int x",
        "}",
        "void main() {}"
      }));
  SemanticChecker checker;
  try {
    ASTParser(Lexer(in)).parse().accept(checker);
    FAIL();
  } catch(MyPLException& ex) {
    string msg = ex.what();
    ASSERT_TRUE(msg.starts_with("Static Error:"));
  }
}

TEST(BasicClassTests, DuplicateClassMembersDiffVis) {
  stringstream in(build_string({
        "class C {",
        " public:",
        "   int x",
        " private:",
        "   int x",
        "}",
        "void main() {}"
      }));
  SemanticChecker checker;
  try {
    ASTParser(Lexer(in)).parse().accept(checker);
    FAIL();
  } catch(MyPLException& ex) {
    string msg = ex.what();
    ASSERT_TRUE(msg.starts_with("Static Error:"));
  }
}

TEST(BasicClassTests, DuplicateClassMethodsPublic) {
  stringstream in(build_string({
        "class C {",
        " public:",
        "   int x() {}",
        "   int x() {}",
        "}",
        "void main() {}"
      }));
  SemanticChecker checker;
  try {
    ASTParser(Lexer(in)).parse().accept(checker);
    FAIL();
  } catch(MyPLException& ex) {
    string msg = ex.what();
    ASSERT_TRUE(msg.starts_with("Static Error:"));
  }
}

TEST(BasicClassTests, DuplicateClassMethodsPrivate) {
  stringstream in(build_string({
        "class C {",
        " private:",
        "   int x() {}",
        "   int x() {}",
        "}",
        "void main() {}"
      }));
  SemanticChecker checker;
  try {
    ASTParser(Lexer(in)).parse().accept(checker);
    FAIL();
  } catch(MyPLException& ex) {
    string msg = ex.what();
    ASSERT_TRUE(msg.starts_with("Static Error:"));
  }
}

TEST(BasicClassTests, DuplicateClassMethodsDiffVis) {
  stringstream in(build_string({
        "class C {",
        " public:",
        "   int x() {}",
        " private:",
        "   int x() {}",
        "}",
        "void main() {}"
      }));
  SemanticChecker checker;
  try {
    ASTParser(Lexer(in)).parse().accept(checker);
    FAIL();
  } catch(MyPLException& ex) {
    string msg = ex.what();
    ASSERT_TRUE(msg.starts_with("Static Error:"));
  }
}

//visibility testing
TEST(BasicClassTests, MainBadAccessPrivateMember) {
  stringstream in(build_string({
        "class C {",
        " private:",
        "   int x",
        "}",
        "void main(){",
        "  C c = new C",
        "  int y = c.x",
        "}"
      }));
  SemanticChecker checker;
  try {
    ASTParser(Lexer(in)).parse().accept(checker);
    FAIL();
  } catch(MyPLException& ex) {
    string msg = ex.what();
    ASSERT_TRUE(msg.starts_with("Static Error:"));
  }
}

TEST(BasicClassTests, MainBadAccessPrivateMethod) {
  stringstream in(build_string({
        "class C {",
        " private:",
        "   int x() {return 1}",
        "}",
        "void main(){",
        "  C c = new C",
        "  int y = c.x()",
        "}"
      }));
  SemanticChecker checker;
  try {
    ASTParser(Lexer(in)).parse().accept(checker);
    FAIL();
  } catch(MyPLException& ex) {
    string msg = ex.what();
    ASSERT_TRUE(msg.starts_with("Static Error:"));
  }
}

//visibility testing
TEST(BasicClassTests, MainBadAccessNonexistantMember) {
  stringstream in(build_string({
        "class C {",
        " private:",
        "   int x",
        "}",
        "void main(){",
        "  C c = new C",
        "  int y = c.z",
        "}"
      }));
  SemanticChecker checker;
  try {
    ASTParser(Lexer(in)).parse().accept(checker);
    FAIL();
  } catch(MyPLException& ex) {
    string msg = ex.what();
    ASSERT_TRUE(msg.starts_with("Static Error:"));
  }
}

TEST(BasicClassTests, MainBadAccessNonExistantMethod) {
  stringstream in(build_string({
        "class C {",
        " private:",
        "   int x() {return 1}",
        "}",
        "void main(){",
        "  C c = new C",
        "  int y = c.z()",
        "}"
      }));
  SemanticChecker checker;
  try {
    ASTParser(Lexer(in)).parse().accept(checker);
    FAIL();
  } catch(MyPLException& ex) {
    string msg = ex.what();
    ASSERT_TRUE(msg.starts_with("Static Error:"));
  }
}


TEST(BasicClassTests, ASTClassMemberLvals) {
  stringstream in(build_string({
        "class C {",
        "public:",
        "  int x",
        "  bool b",
        "  char c",
        "}",
        "void main() {",
        "  C c = new C",
        "  c.x = 1",
        "  c.b = false",
        "  c.c = 'c'",
        "}",
      }));
  SemanticChecker checker;
  ASTParser(Lexer(in)).parse().accept(checker);
}

//----------------------------------------------------------------------
// vm tests
//----------------------------------------------------------------------

TEST(BasicClassTests, BasicEmptyClassAlloc) {
  VMFrameInfo main {"main", 0};                      
  main.instructions.push_back(VMInstr::ALLOCC());
  main.instructions.push_back(VMInstr::ALLOCC());
  main.instructions.push_back(VMInstr::WRITE());
  main.instructions.push_back(VMInstr::WRITE());  
  VM vm;
  vm.add(main);
  stringstream out;
  change_cout(out);
  vm.run();
  EXPECT_EQ("20242023", out.str());
  restore_cout();
}

TEST(BasicClassTests, OneFieldCLassAlloc) {
  VMFrameInfo main {"main", 0};                      
  main.instructions.push_back(VMInstr::ALLOCC());
  main.instructions.push_back(VMInstr::DUP());
  main.instructions.push_back(VMInstr::ADDMEM("mem_1"));
  main.instructions.push_back(VMInstr::DUP());
  main.instructions.push_back(VMInstr::PUSH("blue"));
  main.instructions.push_back(VMInstr::SETMEM("mem_1"));
  main.instructions.push_back(VMInstr::GETMEM("mem_1"));
  main.instructions.push_back(VMInstr::WRITE());
  VM vm;
  vm.add(main);
  stringstream out;
  change_cout(out);
  vm.run();
  EXPECT_EQ("blue", out.str());
  restore_cout();
}

TEST(BasicClassTests, OneMemberTwoClassAlloc) {
  VMFrameInfo main {"main", 0};                      
  main.instructions.push_back(VMInstr::ALLOCC());
  main.instructions.push_back(VMInstr::STORE(0));     // x = oid_1
  main.instructions.push_back(VMInstr::ALLOCC());
  main.instructions.push_back(VMInstr::STORE(1));     // y = oid_1
  main.instructions.push_back(VMInstr::LOAD(0));
  main.instructions.push_back(VMInstr::ADDMEM("m_1"));
  main.instructions.push_back(VMInstr::LOAD(1));
  main.instructions.push_back(VMInstr::ADDMEM("m_1"));
  main.instructions.push_back(VMInstr::LOAD(0));
  main.instructions.push_back(VMInstr::PUSH("blue"));
  main.instructions.push_back(VMInstr::SETMEM("m_1"));
  main.instructions.push_back(VMInstr::LOAD(1));
  main.instructions.push_back(VMInstr::PUSH("green"));
  main.instructions.push_back(VMInstr::SETMEM("m_1"));
  main.instructions.push_back(VMInstr::LOAD(0));  
  main.instructions.push_back(VMInstr::GETMEM("m_1"));
  main.instructions.push_back(VMInstr::WRITE());
  main.instructions.push_back(VMInstr::LOAD(1));  
  main.instructions.push_back(VMInstr::GETMEM("m_1"));
  main.instructions.push_back(VMInstr::WRITE());
  VM vm;
  vm.add(main);
  stringstream out;
  change_cout(out);
  vm.run();
  EXPECT_EQ("bluegreen", out.str());
  restore_cout();
}


//----------------------------------------------------------------------
// code gen tests
//----------------------------------------------------------------------

TEST(BasicClassTests, SimpleMemberLval) {
  stringstream in(build_string({
        "class C {",
        "public:  ",
        "  int x1",
        "   bool x2",
        "   char x3",
        "   double x4",
        "   string x5",
        "}",
        "void main() {",
        "  C c = new C",
        "  c.x1 = 3",
        "  c.x2 = true",
        "  c.x3 = 'a'",
        "  c.x4 = 2.7",
        "  c.x5 = \"abc\"", 
        "  print(c.x1)",
        "  print(c.x2)",
        "  print(c.x3)",
        "  print(c.x4)",
        "  print(c.x5)", 
        "}"
      }));
  VM vm;
  CodeGenerator generator(vm);
  ASTParser(Lexer(in)).parse().accept(generator);
  stringstream out;
  change_cout(out);
  vm.run();
  EXPECT_EQ("3truea2.700000abc", out.str());
  restore_cout();
}   

TEST(BasicClassTests, SimpleMemberRvalAssigns) {
  stringstream in(build_string({
        "class C{",
        "public:",
        "  int x",
        " }",
        "void main() {",
        "  C c = new C",
        "  c.x = 3",
        "  int y = c.x",
        "  print(y)",
        "  c.x = 4",
        "  y = c.x", 
        "  print(y)",
        "}"
      }));
  VM vm;
  CodeGenerator generator(vm);
  ASTParser(Lexer(in)).parse().accept(generator);
  stringstream out;
  change_cout(out);
  vm.run();
  EXPECT_EQ("34", out.str());
  restore_cout();
}   


TEST(BasicClassTests, SimpleMethodCalls) {
  stringstream in(build_string({
        "class C{",
        "public:",
        "  void y() {}",
        "  int x() {return 0}",
        "  bool b(int z) {return true}",
        " }",
        "void main() {",
        "  C c = new C",
        "  int x = c.y()",
        "  int i = c.x()",
        "  bool bl = c.b(5)",
        "  print(i)",
        "  print(\"\n\")",
        "  print(bl)",
        "  print(\"\n\")",
        "}"
      }));
  VM vm;
  CodeGenerator generator(vm);
  ASTParser(Lexer(in)).parse().accept(generator);
  stringstream out;
  change_cout(out);
  vm.run();
  EXPECT_EQ("0\ntrue\n", out.str());
  restore_cout();
}  

//----------------------------------------------------------------------
// main
//----------------------------------------------------------------------

int main(int argc, char** argv)
{
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
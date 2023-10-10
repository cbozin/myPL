//----------------------------------------------------------------------
// FILE: mypl.cpp
// DATE: Spring 2023
// AUTH: Carolyn Bozin
// DESC: Revised version of myPl.cpp where the --lex flag
// reads and outputs tokens, the --parse flag parses the code checking
// that the syntax is correct, the --print flag pretty prints the code
// after parsing using AST parser, and the --check flag type cheks the code
//-----------------------------------------------------------------------

#include <iostream>
#include <fstream>
#include "lexer.h"
#include "token.h"
#include "simple_parser.h"
#include "ast_parser.h"
#include "print_visitor.h"
#include "mypl_exception.h"
#include "semantic_checker.h"
#include "vm.h"
#include "code_generator.h"

using namespace std;

//function prototypes
void printOptions();
void printHeaders(string args[]);
void printInput(string flag, istream *input);


int main(int argc, char* argv[])
{

  //array to store command line arguments
  string args[argc];

  //convert args from c strings into strings
  //store in args[]
  for(int i = 0; i < argc; ++i){

    args[i] = string(argv[i]);
  
  }  

  //if help flag given, print options menu
  if(args[1] == "--help"){

    printOptions();
    return -1;
  
  }

  //if three arguments given, or 2 arguments where 2nd arg not a flag, check for valid file
  if(argc == 3 || (argc == 2 && args[1][0] != '-')){
    
    //new ptr to track input
    istream *input = new ifstream;

    if(argc == 3){//if 3 arguments, assume file is 3rd arg

      input = new ifstream(argv[2]);

    }else if(argc == 2){// if 3 arguments, assume file is 2nd arg

      input = new ifstream(argv[1]);
    }

    if(input->fail()){//if error print to user

      cout << "ERROR: This file does not exist." << endl;
      //delete ptr
      delete input;
      return -1;
    }
    else{// if valid file, continue

      //print headers for each respective flag
      printHeaders(args);

      // if 3 arguments, flag is arg 2
      // call printInput()
      if(argc == 3){

        printInput(args[1], input);

      }else{// else if 2 arguments, no flag. call printInput()

        printInput("", input);

      }
      delete input;
    }

  }else if(argc == 2 || argc == 1){// 2 args or 1 args at this point means std input

    //input stream pointer
    istream *input = &cin;  

    //print headers based on flag
    printHeaders(args);

    if(argc == 2){// if 2 arguments, assume flag is 2nd arg. call printInput()

      printInput(args[1], input);

    }else{// else 1 argument, assume no flag. call printInput()

      printInput("", input);
    }  

  }else{// otherwise, print help menu

    printOptions();

  }

  return 0;
  
}

/*
* Input: void
* Output: void
* This function prints out an options menu that
* explains what each flag does in this program
*/
void printOptions(){

  cout << "Usage: ./mypl [option] [script-file]" << endl;
  cout << "Options:" << endl;
  cout << "   --help   prints this message" << endl;
  cout << "   --lex    displays token information" << endl;
  cout << "   --parse  checks for syntax errors" << endl;
  cout << "   --print  pretty prints program" << endl;
  cout << "   --check  statically checks program" << endl;
  cout << "   --ir     print intermediate (code) representation" << endl; 
  

}

/*
* Input: string array of arguments
* Output: void
* This function prints out Headers for each mode
* depending on the flag it gets from the args[]
* array.
*/
void printHeaders(string args[]){

  // if(args[1] == "--lex"){

  //   cout << "[Lex Mode]" << endl;

  // }else if(args[1] == "--parse"){

  //   cout << "[Parse Mode]" << endl;

  // }else if(args[1] == "--print"){

  //  // cout << "[Print Mode]" << endl;
        
  // }else if(args[1] == "--check"){

  //   //cout << "[Check Mode]" << endl;
        
  // }else if(args[1] == "--ir"){

  //   //cout << "[IR Mode]" << endl;
        
  // }else{

  //   cout << "[Normal Mode]" << endl;
  // }
  
}

/*
* Input: string flag, istream ptr input
* Output: void
* This function takes in the given flag as well as
* an input stream ptr. Based on the flag, it grabs
* the right amount of characters from the stream and prints them.
* If using std input, the function allws user input before printing.
*/
void printInput(string flag, istream *input){

  Lexer lexer = Lexer(*input);

  if(flag == "--lex"){// if lex flag, create Lexer object

    try{

      //grab token and print
      Token t = lexer.next_token();
      cout << to_string(t) << endl;
      
      while(t.type() != TokenType::EOS){//while not EOS, grab tokens

        t = lexer.next_token();
        cout << to_string(t) << endl;

      }

    }catch(MyPLException& ex){//throw exception

      cerr << ex.what() << endl;

    }
  }
  // if parse, create parser 
  if(flag == "--parse"){

    try{

      SimpleParser parser(lexer);
      parser.parse();//parse the tokens output by lexer
      // ASTParser parser(lexer);

    }catch(MyPLException &ex){

      cerr << ex.what() << endl;
    }
  }
  // if print, create print visitor object 
  if(flag == "--print"){

    try {
      ASTParser parser(lexer);
      Program p = parser.parse();
      PrintVisitor v(cout);
      p.accept(v);

    } catch (MyPLException& ex) {
      cerr << ex.what() << endl;

    }
  }
  
  //if check, print first line
  if(flag == "--check"){

    try {
      ASTParser parser(lexer);
      Program p = parser.parse();
      SemanticChecker v;
      p.accept(v);
    } catch (MyPLException& ex) {
      cerr << ex.what() << endl;
    }


  }
  
  // if ir, print first two lines
  if(flag == "--ir"){

    try {
      ASTParser parser(lexer);
      Program p = parser.parse();
      SemanticChecker t;
      p.accept(t);
      VM vm;
      CodeGenerator g(vm);
      p.accept(g);
      cout << to_string(vm) << endl;
    } catch (MyPLException& ex) {
      cerr << ex.what() << endl;
    }

  }

  // if no flag, print all the text
  if(flag == ""){

    try {
      ASTParser parser(lexer);
      Program p = parser.parse();
      SemanticChecker t;
      p.accept(t);
      VM vm;
      CodeGenerator g(vm);
      p.accept(g);
      vm.run();
    } catch (MyPLException& ex) {
      cerr << ex.what() << endl;
    }
  }

}
//----------------------------------------------------------------------
// FILE: lexer.cpp
// DATE: CPSC 326, Spring 2023
// NAME: Carolyn Bozin
// DESC: Implementation file for Lexer class
//----------------------------------------------------------------------

#include "lexer.h"
#include "token.h"
#include <iostream>

using namespace std;

//Lexer constructor
Lexer::Lexer(istream& input_stream)
  : input {input_stream}, column {0}, line {1}
{}

//input: void
//output: char val
//read function (iterates column number)
char Lexer::read()
{
  ++column;
  return input.get();
}

//input: void
//output: char val
//function to look ahead into input stream
char Lexer::peek()
{
  return input.peek();
}

//input: string msg, int line, int col
//output: void 
//function to throw myPL error and display message
void Lexer::error(const string& msg, int line, int column) const
{
  throw MyPLException::LexerError(msg + " at line " + to_string(line) +
                                  ", column " + to_string(column));
}

//input: void
//output: Token object
//this function searches an input stream for tokens.
Token Lexer::next_token()
{

  //default token
  Token t = Token();
  //peek at first char
  char ch = peek();

  // if ch is a comment starter or space, continue
  if(ch == '#' || isspace(ch)){

    //loop while next ch is a space or #
    while(isspace(peek()) || peek() == '#'){

      ch = read();//read ch
      if(isspace(ch)){//if space (or newline), continue

        if(ch == '\n'){// if newline, increment line & reset column

          ++line;
          column = 0;
        }

        if(!isspace(peek()) && peek() != '#'){// if next ch is not # or space, read and break loop

          ch = read();
          break; 
        }

        if(ch != '\n'){// if regular space, check for more spaces
          while(isspace(ch) && ch != '\n'){// while space, read

            ch = read();
          }
          if(ch == '\n'){// if hit a newline, increment line, reset column

            ++line;
            column = 0;

            if(!isspace(peek()) && (peek()) != '#'){// if next ch is not space or #, read ch and break loop
              ch = read();
              break;
            }
            
          }else if(ch == '#'){// if comment after space, eat comment line
            while(ch != '\n' && ch != EOF){//while not newline or EOF, read ch

              ch = read();

            }
            if(ch == EOF){// if ch is EOF break

              break;
            }
            //if newline increment line and reset column
            ++line;
            column = 0;
            
          }else{//if next ch not # or newline, break

            break;

          }
        }
      
      }else{// if comment line, continue
   
        while(ch != '\n' && ch != EOF){//while not newline or EOF, read ch
          
          ch = read();
        }
        if(ch == EOF){// if EOF, break
      
          break;

        }
        //if newline,increment line and reset column
        ++line;
        column = 0;

        //check if not space or comment 
        if(peek() != '#' && !isspace(peek())){
          
          ch = read();
          break;

        }
      } 
    }
    
  }else{// if peek is not a # or space, read char

    read();

  }

  //if no more spaces or comments, can check for tokens:

  if(ch == EOF){//check for end of file

    return(Token(TokenType::EOS, "end-of-stream", line, column));

  }else if(ispunct(ch) && ch != '\'' && ch != '"'){//check for punctuation marks (not ' or " yet)
  
    if(ch == ','){//comma

      return(Token(TokenType::COMMA, ",", line, column));

    }
    else if(ch == '.'){//dot

      return(Token(TokenType::DOT, ".", line, column));
    }
    else if(ch == '['){// left bracket

      return(Token(TokenType::LBRACKET, "[", line, column));
    }
    else if(ch == ']'){//right bracket

      return(Token(TokenType::RBRACKET, "]", line, column));
      
    }
    else if(ch == '('){//left parentheses
      
      return(Token(TokenType::LPAREN, "(", line, column));
      
    }
    else if(ch == ')'){//right parentheses
      
      return(Token(TokenType::RPAREN, ")", line, column));
      
    }
    else if(ch == ';'){//semicolon
      
      return(Token(TokenType::SEMICOLON, ";", line, column));
      
    }
    else if(ch == '{'){//left brace
      
      return(Token(TokenType::LBRACE, "{", line, column));
      
    }
    else if(ch == '}'){//right brace
      
      return(Token(TokenType::RBRACE, "}", line, column));
      
    }else if(ch == '+'){//plus
      
      return(Token(TokenType::PLUS, "+", line, column));
      
    }else if(ch == '-'){//minus

      return(Token(TokenType::MINUS, "-", line, column));
      
    }else if(ch == '*'){//times

      return(Token(TokenType::TIMES, "*", line, column));
      
    }else if(ch == '/'){//divide

      return(Token(TokenType::DIVIDE, "/", line, column));
      
    }//add semicolon
    else if(ch == ':'){
      
      return(Token(TokenType::COLON, ":", line, column));
    }
    //two character tokens:
    else if(ch == '='){ //tokens starting with =

      if(peek() == '='){// return == (equality)
        
        ch = read();
        return(Token(TokenType::EQUAL, "==", line, column - 1));

      }else{//otherwise, ch is = (assign)

        return(Token(TokenType::ASSIGN, "=", line, column));

      }

    }else if(ch == '<'){//tokens starting with <

      if(peek() == '='){// return <= sign

        ch = read();
        return(Token(TokenType::LESS_EQ, "<=", line, column - 1));

      }else{//otherwise, < sign

        return(Token(TokenType::LESS, "<", line, column));

      }

    }else if(ch == '>'){//tokens starting with >

      if(peek() == '='){// return >=
        
        ch = read();
        return(Token(TokenType::GREATER_EQ, ">=", line, column - 1));

      }else{//otherwise, return >

        return(Token(TokenType::GREATER, ">", line, column));

      }

    }else if(ch == '!'){// tokens starting with !

      if(peek() == '='){// return !=

        ch = read();
        return(Token(TokenType::NOT_EQUAL, "!=", line, column - 1));

      }else{// otherwise throw error

        ch = read();

        string myStr;
        myStr += ch;

        error("expecting '!=' found '!" + myStr + "'", line, column - 1);

      }

    }else{// if none of the above punctuation chars, then throw error

      string myStr;
      myStr += ch;

      error("unexpected character '" + myStr + "'", line, column);

    }

  //check for chars:
  }else if(ch == '\''){//if ', then continue

    // if next ch is letter or punctuation and is not '\\', continue
    if(isalpha(peek()) || ispunct(peek()) && peek() != '\\'){

      if(peek() == '\''){//if next ch is a ', throw error (empty char)

        ch = read();
        error("empty character", line, column);

      }else{//otherwise, read next ch
        
        ch = read();

      }

      if(peek() == '\''){//if next character is a ', return char token
        string myCh;
        myCh += ch;
        ch = read();

        return(Token(TokenType::CHAR_VAL, myCh, line, column - 2));

      }else{//otherwise, throw error (unclosed char)

        ch = read();

        string myStr;
        myStr += ch;

        error("expecting ' found " + myStr, line, column);
      }

    }else if(isdigit(peek())){// if next ch is a number, throw error (invalid char)

      read();
      error("Invalid char", line, column);

    }else if(peek() == '\\'){//if backslashes, then continue

      read();//read next ch

      if(isalpha(peek())){// if letter follows backslash, continue
          
        ch = read();//read next ch

        if(peek() == '\''){// if next ch is a ', return a char value

          string myCh;
          myCh += ch;

          ch = read();
          return(Token(TokenType::CHAR_VAL, "\\" + myCh, line, column - 3));

        }      
      
      }else{// if letter does not follow, throw error

        ch = read();
        error("Invalid char" , line, column);
      }

    }else if(peek() == EOF){// if EOF after ', throw error

      ch = read();
      error("found end-of-file in character", line, column);

    }else if(peek() == '\n'){// if newline after ', throw error

      ch = read();

      error("found end-of-line in character", line, column);
    }else if(isspace(peek())){//if space, eat space and check for '
      ch = read();
      if(peek() != '\''){
        error("char not closed", line, column);
      }else{
        ch = read();
        return(Token(TokenType::CHAR_VAL, " ", line, column - 3));

      }
    }
    
  // check for ints and doubles:
  }else if(isdigit(ch)){

    //check if leading zero, if so throw error
    if(ch == '0' && isdigit(peek())){

      error("leading zero in number", line, column);
    }

    //string to hold num value
    string num;
    num += ch;
    //var to hold start pos of int
    int intStart = column;

    while(!isspace(peek()) && peek() != EOF){//while next ch not space or EOF, loop

      if(isdigit(peek())){ // if numeric val is next, read and add to string

        ch = read();
        num += ch;

      }else if(peek() == '.'){ // if . then is a double. read and add to string

        ch = read();
        num += ch;

        //check if number after decimal. if not throw error
        if(!isdigit(peek())){

          error("missing digit in '" + num + "'", line, column + 1);

        }

        if(isdigit(peek())){// if next val is number, continue

          while(isdigit(peek())){//while next ch is number, read and add to string

            ch = read();
            num += ch;
          }

          //return double 
          return(Token(TokenType::DOUBLE_VAL, num, line, intStart));

        }else{//if next val not number, throw error

          error("Invalid double value", line, column);
        }

      }else{// if not number or ., break loop and return int val

        break;
      }
    }
 
    return(Token(TokenType::INT_VAL, num, line, intStart));

  //strings:
  }else if(ch == '"'){

    if((peek()) == '"'){// check if empty string, if so read ch and return string val

      ch = read();
      return(Token(TokenType::STRING_VAL, "", line, column));

    }

    int startCol = column;// track beginning of string
    ch = read();//read next ch

    string myStr;//string to store char vals
    myStr += ch;

    if(isalpha(ch) || ispunct(ch) || isdigit(ch) || isspace(ch)){ // check if string contains punctuation or numbers

      ch = read();//read ch

      while(ch != '"'){//while ch is not ", loop

        if(ch == '\n'){// if newline found, throw error

          error("found end-of-line in string", line, column);

        }else if(ch == EOF){// if EOF, throw error

          error("found end-of-file in string", line, column);
        }

        //add to string val and read ch
        myStr += ch;
        ch = read();
      }
      //return string value
      return(Token(TokenType::STRING_VAL, myStr, line, startCol));

    }else if(ch == EOF){// if EOF found in string throw error

      error("found end-of-file in string", line, column);

    }else{//otherwise throw error

      error("Invalid string", line, column);

    }

  //reserved words, data types, ids:
  }else if(isalpha(ch)){

    string myWord = "";//string to store ch vals
    int wordStart = column ;//store beginning of string

    // if space after or punctuation after (not _), then is ID. return ID Token 
    if((isspace(peek()) || ispunct(peek())) && peek() != '_'){

      myWord += ch;
      return(Token(TokenType::ID, myWord, line, column));
    }

//if next char is letter, number, or _, continue
    if(peek() == '_' || isalpha(peek()) || isdigit(peek())){

      while(peek() == '_' || isalpha(peek()) || isdigit(peek())){//while next ch is letter, number, or _, loop

        //add to string, read next ch
        myWord += ch;
        ch = read();        

      }

      myWord += ch;//add ch to string

    //check string for reserved words
      if(myWord == "and"){// and
          
        return(Token(TokenType::AND, "and", line, wordStart));

      }else if(myWord == "or"){//or

        return(Token(TokenType::OR, "or", line, wordStart));

      }else if(myWord == "not"){//not

        return(Token(TokenType::NOT, "not", line, wordStart));
        
      }
      else if(myWord == "if"){//if

        return(Token(TokenType::IF, "if", line, wordStart));
        
      }else if(myWord == "else"){//else

        return(Token(TokenType::ELSE, "else", line, wordStart)); 

      }else if(myWord == "elseif"){//elseif

        return(Token(TokenType::ELSEIF, "elseif", line, wordStart)); 
      
      }else if(myWord == "new"){//new

        return(Token(TokenType::NEW, "new", line, wordStart));

      }else if(myWord == "return"){//return
        
        return(Token(TokenType::RETURN, "return", line, wordStart));
        
      }else if(myWord == "for"){//for

        return(Token(TokenType::FOR, "for", line, wordStart));

      }else if(myWord == "while"){//while

        return(Token(TokenType::WHILE, "while", line, wordStart));

      }else if(myWord == "struct"){//struct

        return(Token(TokenType::STRUCT, "struct", line, wordStart));

      }else if(myWord == "array"){//array

        return(Token(TokenType::ARRAY, "array", line, wordStart));
      
      }else if(myWord == "true" || myWord == "false"){// bool values (trur & false)

        return(Token(TokenType::BOOL_VAL, myWord, line, wordStart));
          
      }else if(myWord == "null"){//null

        return(Token(TokenType::NULL_VAL, "null", line, wordStart));

      }
      //check class reserved words
      else if(myWord == "class"){

        return(Token(TokenType::CLASS, "class", line, wordStart));

      }else if(myWord == "public"){

         return(Token(TokenType::PUBLIC, "public", line, wordStart));


      }else if(myWord == "private"){

        return(Token(TokenType::PRIVATE, "private", line, wordStart));

      }
      //check primitive data types:
      else if(myWord == "int"){//int
      
        return(Token(TokenType::INT_TYPE, "int", line, wordStart));

      }else if(myWord == "double"){//double

        return(Token(TokenType::DOUBLE_TYPE, "double", line, wordStart));

      }else if(myWord == "bool"){//bool

        return(Token(TokenType::BOOL_TYPE, "bool", line, wordStart));

      }else if(myWord == "char"){//char

        return(Token(TokenType::CHAR_TYPE, "char", line, wordStart));

      }else if(myWord == "string"){//string

        return(Token(TokenType::STRING_TYPE, "string", line, wordStart));

      }else if(myWord == "void"){//void

        return(Token(TokenType::VOID_TYPE, "void", line, wordStart));

      }else{// everything else is an ID

        return(Token(TokenType::ID, myWord, line, wordStart));
      }

    }

  }else if(ispunct(ch)){// if any other punctuation, then throw error

    string myStr;
    myStr += ch;
    error("unexpected character '" + myStr + "'", line, column);
    
  }
  
  return t;

}
  


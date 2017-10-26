#ifndef H_Lexer
#define H_Lexer

#include <iostream>
#include <sstream>
#include <string>
//---------------------------------------------------------------------------
using namespace std;
//---------------------------------------------------------------------------



struct Token
{
   enum Type {
      tok_empty = 0,
      tok_eof = -1,
      tok_def = -2,
      tok_err = -3,
      tok_pun = -4,
      tok_lit_int = 1,
      tok_lit_dbl = 2,
      tok_lit_bool = 3,
      tok_lit_str = 4
   };

   string value;
   Type type;
   
   union { int intValue; double doubleValue; bool boolValue; };

   Token() : type(Type::tok_empty) {}
   Token(Type type, string value) : value(move(value)), type(type) {}
   
   bool is_literal() { return type > 0; }
};

class Lexer 
{
private:
   istream& input;
   stringstream current;
   Token::Type last_type;
   Token current_token;
   char terminator;
   
   Token getToken();
   void readNext();
public:
   Lexer(istream& input, char terminator = '\0')
    : input(input), terminator(terminator) {
      operator++();
   }
   
   void operator++();
   Token operator*();
};

#endif
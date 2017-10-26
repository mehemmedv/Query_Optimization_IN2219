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
        tok_eof = 0,
        tok_def = -1,
        tok_err = -2,
        tok_lit_int = 1,
        tok_lit_dbl = 2,
        tok_lit_bool = 3,
        tok_lit_str = 4
    };

    string value;
    Type type;

    Token(Type type, string value) : value(move(value)), type(type) {}

    union { int intValue; double doubleValue; bool boolValue; };

    static Token eof();
    static Token def(string value);
    static Token err(string value);
    static Token lit_int(string value, int intValue);
    static Token lit_dbl(string value, double doubleValue);
    static Token lit_bool(string value, bool boolValue);
    static Token lit_str(string value);
};

class Lexer 
{
private:
   istream& input;
   stringstream current;
   Token::Type last_type;
   char terminator;
public:
   Lexer(istream& input, char terminator = '\0')
    : input(input), terminator(terminator) {}
   
   void operator++();
   Token operator*();
};

#endif
#include "Lexer.hpp"
#include <locale> 
//---------------------------------------------------------------------------
using namespace std;
//---------------------------------------------------------------------------

Token Token::eof()
{
    return Token(Type::tok_eof, string());
}

Token Token::def(string value)
{
    return Token(Type::tok_def, move(value));
}

Token Token::err(string value)
{
    return Token(Type::tok_err, move(value));
}

Token Token::lit_int(string value, int intValue)
{
    Token retval(Type::tok_lit_int, move(value));
    retval.intValue = intValue;
    return retval;
}

Token Token::lit_dbl(string value, double doubleValue)
{
    Token retval(Type::tok_lit_dbl, move(value));
    retval.doubleValue = doubleValue;
    return retval;
}

Token Token::lit_bool(string value, bool boolValue)
{
    Token retval(Type::tok_lit_bool, move(value));
    retval.boolValue = boolValue;
    return retval;
}

Token Token::lit_str(string value)
{
    return Token(Type::tok_lit_int, move(value));
}


void Lexer::operator++()
{
   if (last_type == Token::Type::tok_eof)
      return;
   
   current.clear();
   
   char c;
   input.get(c);
   if (input.eof() || c == terminator) {
      last_type = Token::Type::tok_eof;
      return;
   }
   
   while (isspace(c)) input.get(c);
   
   if (c == '-' || c == '+')
   {
      current << c;
      input.get(c); while (isspace(c)) input.get(c);
      if (!isdigit(c)) {
         input.putback(c);
         last_type = Token::Type::tok_def;
         return;
      }
   }
   
   if (isdigit(c)) {
      current << c;
      input.get(c);
      while(isdigit(c)) {
         current << c;
         input.get(c);
      }
      
      if (c != '.') {
         input.putback(c);
         last_type = Token::Type::tok_lit_int;
         return;
      }
      else { //c == '.'
         current << c;
         input.get(c);
         while(isdigit(c)) {
            current << c;
            input.get(c);
         }
         
         input.putback(c);
         last_type = Token::Type::tok_lit_dbl;
         return;
      }
   }
   
   if (c == '"') {
      input.get(c);
      while(!input.eof() && c != '"') {
         current << c;
         input.get(c);
      }
      
      //don't put back '"'
      last_type = Token::Type::tok_lit_str;
      return;
   }
   
   current << c;
   input.get(c);
   while(!isspace(c) && !input.eof() && c != terminator) {
      current << c;
      input.get(c);
   }
   
   locale loc;
   string checkstr = tolower(current.str(), loc);
   if (checkstr == "true" || checkstr == "false") {
      last_type = Token::Type::tok_lit_bool;
      return;
   }
   
   last_type = Token::Type::tok_def;
   return;
}

Token Lexer::operator*()
{
   string value = current.str();
   
   switch (last_type) {
      case Token::Type::tok_eof:
         return Token::eof();
      case Token::Type::tok_def:
         return Token::def(value);
      case Token::Type::tok_err:
         return Token::err(value);
      case Token::Type::tok_lit_int:
         int intValue; current >> intValue;
         return Token::lit_int(value, intValue);
      case Token::Type::tok_lit_dbl:
         double doubleValue; current >> doubleValue;
         return Token::lit_dbl(value, doubleValue);
      case Token::Type::tok_lit_bool:
         bool boolValue = value[0] == 't' || value[0] == 'T';
         return Token::lit_bool(value, boolValue);
      case Token::Type::tok_lit_str:
         return Token::lit_str(value);
      default:
         return Token::err(value);
   }
}
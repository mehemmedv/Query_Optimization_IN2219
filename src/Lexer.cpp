#include "Lexer.hpp"
#include <locale> 
//---------------------------------------------------------------------------
using namespace std;
//---------------------------------------------------------------------------


bool ismultipunct(char c)
{
   return c == '<' || c == '=' || c == '>';
}

bool issinglepunct(char c)
{
   return ispunct(c) && !ismultipunct(c);
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
   char oldc = c;
   input.get(c);
   while(!input.eof() && c != terminator && !isspace(c)
        && (!isalnum(oldc) || isalnum(c)) //isword ==> read alphanum
        && (!ismultipunct(oldc) || ismultipunct(c)) //ismultipunc ==> read multipunctuation (==)
        && (!issinglepunct(oldc)) //only read single comma
      ) {
      current << c;
      input.get(c);
   }
   
   if (!input.eof() && c != terminator)
      input.putback(c);
      
   if (ispunct(oldc)) {
      last_type = Token::Type::tok_pun;
      return;
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
         return Token(last_type, string());
      case Token::Type::tok_def:
      case Token::Type::tok_err:
      case Token::Type::tok_pun:
         return Token(last_type, move(value));
      case Token::Type::tok_lit_int: {
         int intValue; current >> intValue;
         Token retval(last_type, move(value));
         retval.intValue = intValue;
         return retval;
      } case Token::Type::tok_lit_dbl: {
         double doubleValue; current >> doubleValue;
         Token retval(last_type, move(value));
         retval.doubleValue = doubleValue;
         return retval;
      } case Token::Type::tok_lit_bool: {
         bool boolValue = value[0] == 't' || value[0] == 'T';
         Token retval(last_type, move(value));
         retval.boolValue = boolValue;
         return retval;
      } case Token::Type::tok_lit_str:
         return Token(last_type, move(value));
      default:
         return Token(Token::Type::tok_err, move(value));
   }
}
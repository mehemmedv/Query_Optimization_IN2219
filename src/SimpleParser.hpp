#ifndef H_SimpleParser
#define H_SimpleParser

#include <iostream>
#include <vector>
#include "Lexer.hpp"

struct SqlParseException : public exception {
    const char * what () const throw () {
       return "Sql Parse Exception";
    }
 };
 
struct SqlBinding
{
    Token relation;
    Token binding;
};

struct SqlAttribute
{
    Token binding;
    Token attribute;
};

struct SqlPredicate
{
    enum Type { prd_attribute, prd_constant };
    Type type;
    SqlAttribute lhs;
    
    SqlAttribute rhs;
    Token constant;
    
    SqlPredicate(SqlAttribute lhs, SqlAttribute rhs) : type(Type::prd_attribute), lhs(lhs), rhs(rhs) {}
    SqlPredicate(SqlAttribute lhs, Token constant) : type(Type::prd_constant), lhs(lhs), constant(constant) {}
};

struct SqlParse
{
    vector<Token> projection;
    vector<SqlBinding> bindings;
    vector<SqlPredicate> predicates;
};


class SimpleParser
{
public:
    SimpleParser() {}   //take options as parameter (case sensitivity?)
    SqlParse parse_stream(istream& input);
};


#endif
#include "SimpleParser.hpp"

#include <locale>

bool peek_check(Lexer& lexer, const string& check_value)
{
    if ((*lexer).type == Token::Type::tok_eof)
        return false;
    std::locale loc;
    if (tolower((*lexer).value, loc) != check_value) {
        return false;
    }
    return true;
}

Token read_attribute(Lexer& lexer)
{
    Token retval = *lexer;
    ++lexer;
    return retval;
}

void parse_select(Lexer& lexer, SqlParse& sql)
{
    if (!peek_check(lexer, "select")) throw SqlParseException();
    
    ++lexer;
    sql.projection.push_back(read_attribute(lexer));
    
    while (!peek_check(lexer, "from"))
    {
        if ((*lexer).type == Token::Type::tok_eof)
            throw SqlParseException();
        
        if (!peek_check(lexer, ",")) throw SqlParseException();
        ++lexer;
        sql.projection.push_back(read_attribute(lexer));
    }
}

SqlBinding read_binding(Lexer& lexer)
{
    SqlBinding mybind;
    mybind.relation = *lexer;
    ++lexer;
    mybind.binding = *lexer;
    ++lexer;
    return mybind;
}

void parse_from(Lexer& lexer, SqlParse& sql)
{
    if (!peek_check(lexer, "from")) throw SqlParseException();
    
    ++lexer;
    sql.bindings.push_back(read_binding(lexer));
    
    while (!peek_check(lexer, "where"))
    {
        if ((*lexer).type == Token::Type::tok_eof)
            throw SqlParseException();
        
        if (!peek_check(lexer, ",")) throw SqlParseException();
        ++lexer;
        sql.bindings.push_back(read_binding(lexer));
    }
}

SqlPredicate readSqlPredicate(Lexer& lexer)
{
    SqlAttribute lhs;
    
    lhs.binding = *lexer;
    ++lexer;
    if (!peek_check(lexer, ".")) throw SqlParseException();
    ++lexer;
    lhs.attribute = *lexer;
    ++lexer;
    
    Token rhsfirst = *lexer;
    ++lexer;
    
    if (rhsfirst.is_literal()) {
        return SqlPredicate(lhs, rhsfirst);
    } else {
        SqlAttribute rhs;
        rhs.binding = rhsfirst;
        lhs.attribute = *lexer;
        ++lexer;
        
        return SqlPredicate(lhs, rhs);
    }
}

void parse_where(Lexer& lexer, SqlParse& sql)
{
    if (!peek_check(lexer, "where")) throw SqlParseException();
    
    ++lexer;
    sql.predicates.push_back(readSqlPredicate(lexer));
    
    while (!peek_check(lexer, "where"))
    {
        if ((*lexer).type == Token::Type::tok_eof)
            throw SqlParseException();
        
        if (!peek_check(lexer, ",")) throw SqlParseException();
        ++lexer;
        sql.predicates.push_back(readSqlPredicate(lexer));
    }
}

SqlParse SimpleParser::parse_stream(istream& input)
{
    Lexer lexer(input);
    SqlParse retval;
    
    parse_select(lexer, retval);
    parse_from(lexer, retval);
    parse_where(lexer, retval);
}
#include "SimpleParser.hpp"

#include <locale>

bool peek_check(Lexer& lexer, const string& check_value)
{
    if ((*lexer).type == Token::Type::tok_eof)
        return false;
    //std::locale loc;
    // TODO: lower
    if ((*lexer).value != check_value) {
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
    if (!peek_check(lexer, "select")) throw SqlParseException("No select statement found");
    
    ++lexer;
    sql.projection.push_back(read_attribute(lexer));
    
    while (!peek_check(lexer, "from"))
    {
        if ((*lexer).type == Token::Type::tok_eof)
            throw SqlParseException("Select statement ended prematurely (no from statement)");
        
        if (!peek_check(lexer, ",")) throw SqlParseException("Select statement broken");
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
    if (!peek_check(lexer, "from")) throw SqlParseException("No from statement found");
    
    ++lexer;
    sql.bindings.push_back(read_binding(lexer));
    
    while (!peek_check(lexer, "where"))
    {
        if ((*lexer).type == Token::Type::tok_eof)
            throw SqlParseException("From statement ended prematurely (no where statement)");
        
        if (!peek_check(lexer, ",")) throw SqlParseException("From statement broken");
        ++lexer;
        sql.bindings.push_back(read_binding(lexer));
    }
}

SqlPredicate readSqlPredicate(Lexer& lexer)
{
    SqlAttribute lhs;
    
    lhs.binding = *lexer;
    ++lexer;
    if (!peek_check(lexer, ".")) throw SqlParseException("Predicate format broken");
    ++lexer;
    lhs.attribute = *lexer;
    ++lexer;
    
    if (!peek_check(lexer, "=")) throw SqlParseException("Predicate format broken");
    ++lexer;
    
    Token rhsfirst = *lexer;
    ++lexer;
    
    if (rhsfirst.is_literal()) {
        return SqlPredicate(lhs, rhsfirst);
    } else {
        if (!peek_check(lexer, ".")) throw SqlParseException("Predicate format broken");
        ++lexer;
        SqlAttribute rhs;
        rhs.binding = rhsfirst;
        rhs.attribute = *lexer;
        ++lexer;
        
        return SqlPredicate(lhs, rhs);
    }
}

void parse_where(Lexer& lexer, SqlParse& sql)
{
    if (!peek_check(lexer, "where")) throw SqlParseException("Where statement not found");
    
    ++lexer;
    sql.predicates.push_back(readSqlPredicate(lexer));
    
    while ((*lexer).type != Token::Type::tok_eof)
    {
        if (!peek_check(lexer, "and")) throw SqlParseException("Where statement broken");
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
    
    return retval;
}
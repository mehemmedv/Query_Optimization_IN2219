#include "SqlQuery.hpp"


SqlQuery::SqlQuery(Database& db, const SqlParse& parse)
    // Constructor
{
    // TODO: check correctness
    for (const auto& bind : parse.bindings)
    {
        bindings.emplace(bind.binding.value, new Tablescan(db.getTable(bind.relation.value)));
    }
    
    for (const auto& pred : parse.predicates)
    {
        registers[pred.lhs.binding.value][pred.lhs.attribute.value] = 
            bindings[pred.lhs.binding.value]->getOutput(pred.lhs.attribute.value);
    }
    
    for (const auto& pred : parse.predicates)
    {
    }
}
//---------------------------------------------------------------------------
SqlQuery::~SqlQuery()
    // Destructor
{
}
//---------------------------------------------------------------------------
void SqlQuery::open()
    // Open the operator
{
    input->open();
}
//---------------------------------------------------------------------------
bool SqlQuery::next()
    // Get the next tuple
{
    return input->next();
}
//---------------------------------------------------------------------------
void SqlQuery::close()
    // Close the operator
{
    input->close();
}
//---------------------------------------------------------------------------
vector<const Register*> SqlQuery::getOutput() const
    // Get all produced values
{
    return output;
}
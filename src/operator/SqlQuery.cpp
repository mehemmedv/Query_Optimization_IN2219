#include "SqlQuery.hpp"

#include "operator/CrossProduct.hpp"
#include "operator/Selection.hpp"
#include "operator/Projection.hpp"

SqlQuery::SqlQuery(Database& db, const SqlParse& parse)
    // Constructor
{
    // TODO: check correctness
    for (const auto& bind : parse.bindings) {
        bindings.emplace(bind.binding.value, new Tablescan(db.getTable(bind.relation.value)));
    }
    
    for (const auto& pred : parse.predicates) {
        registers[pred.lhs.binding.value][pred.lhs.attribute.value] = 
            bindings[pred.lhs.binding.value]->getOutput(pred.lhs.attribute.value);
        
        if (pred.type == SqlPredicate::Type::prd_attribute) {
            registers[pred.rhs.binding.value][pred.rhs.attribute.value] = 
                bindings[pred.rhs.binding.value]->getOutput(pred.rhs.attribute.value);
        }
    }
    
    //implement '*'
    for (const auto& tok : parse.projection) {
        for (const auto& kv : bindings) {
            const Register* mr = kv.second->getOutput(tok.value);
            if (mr) {
                projection.push_back(mr);
                break;
            }
        }
    }
    
    
    
    for (auto& kv : bindings) {
        if (!input) {
            input = move(kv.second);
            continue;
        }
        input = unique_ptr<Operator>(new CrossProduct(move(input), move(kv.second)));
    }
    
    
    for (const auto& pred : parse.predicates) {
        const Register* lhs = registers[pred.lhs.binding.value][pred.lhs.attribute.value];
        const Register* rhs;
        
        if (pred.type == SqlPredicate::Type::prd_attribute) {
            rhs = registers[pred.rhs.binding.value][pred.rhs.attribute.value];
        } else {
            constantRegs.emplace_back(new Register());
            Register* ncrhs = constantRegs.back().get();
            switch (pred.constant.type) {
                case Token::Type::tok_lit_int: {
                    ncrhs->setInt(pred.constant.intValue);
                    break;
                 } case Token::Type::tok_lit_dbl: {
                    ncrhs->setDouble(pred.constant.doubleValue);
                    break;
                 } case Token::Type::tok_lit_bool: {
                    ncrhs->setBool(pred.constant.boolValue);
                     break;
                 } case Token::Type::tok_lit_str: {
                    ncrhs->setString(pred.constant.value);
                    break;
                 } default:
                     break;
            }
            rhs = ncrhs;
        }
        input = unique_ptr<Operator>(new Selection(move(input), lhs, rhs));
    }
    
    //return;
    
    input = unique_ptr<Operator>(new Projection(move(input), projection));
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
    return input->getOutput();
}
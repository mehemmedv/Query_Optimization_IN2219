#include "SqlQuery.hpp"

#include "operator/CrossProduct.hpp"
#include "operator/Selection.hpp"
#include "operator/Projection.hpp"

SqlQuery::SqlQuery(Database& db, const SqlParse& parse)
    // Constructor
{
    {
        map<string, unique_ptr<Tablescan>> tablescans;
        for (const auto& bind : parse.bindings) {
            tablescans.emplace(bind.binding.value, unique_ptr<Tablescan>(new Tablescan(db.getTable(bind.relation.value))));
        }
        
        for (const auto& pred : parse.predicates) {
            registers[pred.lhs.binding.value][pred.lhs.attribute.value] = 
                tablescans[pred.lhs.binding.value]->getOutput(pred.lhs.attribute.value);
            
            if (pred.type == SqlPredicate::Type::prd_attribute) {
                registers[pred.rhs.binding.value][pred.rhs.attribute.value] = 
                    tablescans[pred.rhs.binding.value]->getOutput(pred.rhs.attribute.value);
            }
        }
        
        //implement '*'
        if (!parse.projection.empty() && parse.projection.begin()->value == "*") {
            for (const auto& bind : parse.bindings) {
                Table& mytable = db.getTable(bind.relation.value);
                for(unsigned int i = 0; i < mytable.getAttributeCount(); i++) {
                    const Attribute& attr = mytable.getAttribute(i);
                    
                    const Register* mr = tablescans[bind.binding.value]->getOutput(attr.getName());
                    if (mr) {
                        projection.push_back(mr);
                    }
                }
            }
        } else {
            for (const auto& tok : parse.projection) {
                for (const auto& kv : tablescans) {
                    const Register* mr = kv.second->getOutput(tok.value);
                    if (mr) {
                        projection.push_back(mr);
                        break;
                    }
                }
            }
        }
        
        for (auto& kv : tablescans) {
            bindings.emplace(kv.first, move(kv.second));
        }
    }
    
    for (const auto& pred : parse.predicates) {
        if (pred.type != SqlPredicate::Type::prd_constant)
            continue;
        const Register* lhs = registers[pred.lhs.binding.value][pred.lhs.attribute.value];
        
        constantRegs.emplace_back(new Register());
        Register* rhs = constantRegs.back().get();
        
        
        switch (pred.constant.type) {
            case Token::Type::tok_lit_int: {
                    rhs->setInt(pred.constant.intValue);
                    break;
                } case Token::Type::tok_lit_dbl: {
                    rhs->setDouble(pred.constant.doubleValue);
                    break;
                } case Token::Type::tok_lit_bool: {
                    rhs->setBool(pred.constant.boolValue);
                    break;
                } case Token::Type::tok_lit_str: {
                    rhs->setString(pred.constant.value);
                    break;
                } default:
                    break;
        }
        
        bindings[pred.lhs.binding.value] = unique_ptr<Operator>(new Selection(move(bindings[pred.lhs.binding.value]), lhs, rhs));
    }
    
    for (auto& kv : bindings) {
        if (!input) {
            input = move(kv.second);
            continue;
        }
        input = unique_ptr<Operator>(new CrossProduct(move(input), move(kv.second)));
    }
    
    
    for (const auto& pred : parse.predicates) {
        if (pred.type != SqlPredicate::Type::prd_attribute)
            continue;
        
        const Register* lhs = registers[pred.lhs.binding.value][pred.lhs.attribute.value];
        const Register* rhs = registers[pred.rhs.binding.value][pred.rhs.attribute.value];
        
        input = unique_ptr<Operator>(new Selection(move(input), lhs, rhs));
    }
    
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
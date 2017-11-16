#include "QueryGraphFactory.hpp"

#include <algorithm>


QueryGraph buildGraphFromParse(Database& db, const SqlParse& parse)
{
    QueryGraph retval;
    
    unordered_map<string, Table&> bindtables;
    for (const auto& bind : parse.bindings) {
        Table& mytable = db.getTable(bind.relation.value);
        bindtables.emplace(bind.binding.value, mytable);
        int cardinality = mytable.getCardinality();
        retval.emplaceNode(bind, vector<SqlPredicate>(), cardinality);
    }
    
    for (const auto& pred : parse.predicates) {
        if (pred.type != SqlPredicate::Type::prd_constant)
            continue;
        auto& node = retval.getNode(pred.lhs.binding.value);
        
        Table& mytable = bindtables.find(pred.lhs.binding.value)->second;
        const Attribute& myattr = mytable.getAttribute(mytable.findAttribute(pred.lhs.attribute.value));
        float selectivity = 1.0f;
        if (myattr.getKey()) {
            selectivity = 1.0 / mytable.getCardinality();
        } else {
            selectivity = 1.0f / myattr.getUniqueValues();
        }
        
        node.predicates.push_back(pred);
        node.cardinality *= selectivity;
    }
    
    for (const auto& pred : parse.predicates) {
        if (pred.type != SqlPredicate::Type::prd_attribute)
            continue;
        
        Table& tableL = bindtables.find(pred.lhs.binding.value)->second;
        const Attribute& attrL = tableL.getAttribute(tableL.findAttribute(pred.lhs.attribute.value));
        Table& tableR = bindtables.find(pred.rhs.binding.value)->second;
        const Attribute& attrR = tableR.getAttribute(tableR.findAttribute(pred.rhs.attribute.value));
        float selectivity = 1.0f;
        if (attrL.getKey() && attrR.getKey()) {
            selectivity = 1.0f / max(tableL.getCardinality(), tableR.getCardinality());
        } else if (attrL.getKey()) {
            selectivity = 1.0f / tableL.getCardinality();
        } else if (attrR.getKey()) {
            selectivity = 1.0f / tableR.getCardinality();
        } else {
            selectivity = 1.0f / max(attrL.getUniqueValues(), attrR.getUniqueValues());
        }
        
        retval.emplaceEdge(retval.getNode(pred.lhs.binding.value),
                            retval.getNode(pred.rhs.binding.value),
                            {pred}, selectivity);
    }
    
    return retval;
}
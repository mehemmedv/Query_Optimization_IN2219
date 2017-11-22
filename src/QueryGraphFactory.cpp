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
        node.cardinality = max(node.cardinality, 1);    //no 0 cardinality
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

Tree GOO(QueryGraph &querygraph){
    std::vector<Tree*> trees;
    
    for(auto &node : querygraph.getAllNodes()){
        trees.push_back(new Tree(node));
    }

    while(trees.size() > 1){
        int minCost = trees[0]->cost(querygraph, trees[0], trees[1]);
        int leftIdx = 0, rightIdx = 1, currentLeftIdx = 0, currentRightIdx = 0;
        for(auto leftIterator = trees.begin(); leftIterator != trees.end(); ++leftIterator, ++currentLeftIdx){
            currentRightIdx = 0;
            for(auto rightIterator = leftIterator + 1; rightIterator != trees.end(); ++rightIterator, ++currentRightIdx){
                int currentCost = (*leftIterator)->cost(querygraph, *leftIterator, *rightIterator);
                if(currentCost < minCost){
                    minCost = currentCost;
                    leftIdx = currentLeftIdx;
                    rightIdx = currentRightIdx;
                }
            }
        }
	trees.push_back(new Tree(trees[leftIdx], trees[rightIdx]));
        delete *(trees.begin() + leftIdx);
        trees.erase(trees.begin() + leftIdx);
        delete *(trees.begin() + rightIdx - 1);
        trees.erase(trees.begin() + rightIdx - 1);
    }
    return *(trees[0]);
}


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

int Find(int x, int* parent){
    if(parent[x] == x)
        return x;
    return parent[x] = Find(parent[x], parent);
}

void Union(int l, int r, int* parent, int* rank, Tree** trees){
    int par_l = Find(l, parent);
    int par_r = Find(r, parent);

    if(rank[par_l] > rank[par_r]){
        parent[par_r] = par_l;
        trees[par_l] = new Tree(trees[par_l], trees[par_r]);
    } else{
        parent[par_l] = par_r;
        trees[par_r] = new Tree(trees[par_r], trees[par_l]);
        if(rank[par_l] == rank[par_r])
            ++rank[par_r];
    }
}

Tree QuickPick(QueryGraph &querygraph, int cnt_trees){
    Tree min_cost_tree;
    double min_cost = std::numeric_limits<double>::max();
    auto edges = querygraph.getAllEdges();
    int cnt_nodes = querygraph.getNodeCount();
    Tree** trees = new Tree*[cnt_nodes];
    int *parent = new int[cnt_nodes];
    int *rank = new int[cnt_nodes];
    for(int i = 0; i < cnt_nodes; ++i){
        trees[i] = new Tree(querygraph.getNode(i));
        parent[i] = i;
        rank[i] = 0;
    }
    

    for(int cnt_i = 0; cnt_i < cnt_trees; ++cnt_i){
        random_shuffle(edges.begin(), edges.end());
        for(auto edge: edges){
            if(Find(edge.nodeA, parent) != Find(edge.nodeB, parent)){ // different sets, can join
                Union(edge.nodeA, edge.nodeB, parent, rank, trees);
            }
        }

        int par_tree = Find(0, parent);
        double current_cost = trees[par_tree]->cost(querygraph);
        if(current_cost < min_cost){
            min_cost = current_cost;
            min_cost_tree = *trees[par_tree];
        }

        for(int i = 0; i < cnt_nodes; ++i){
            trees[i] = new Tree(querygraph.getNode(i));
            parent[i] = i;
            rank[i] = 0;
        }
    }
    return min_cost_tree;
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
	//delete *(trees.begin() + rightIdx);
        trees.erase(trees.begin() + rightIdx);
        //delete *(trees.begin() + leftIdx);
        trees.erase(trees.begin() + leftIdx);
    } 
    
    return *(trees[0]);
}


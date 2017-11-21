#include "QueryPlan.hpp"

#include "operator/CrossProduct.hpp"
#include "operator/HashJoin.hpp"
#include "operator/Selection.hpp"
#include "operator/Operator.hpp"

std::vector<QueryNode> Tree::getRelations(Tree* tree){
    std::vector<QueryNode> nodes;
    std::queue<Tree*> trees;
    trees.push(tree);
    while(!trees.empty()){
        Tree* newNode = trees.front();
        trees.pop();
        if(newNode->isLeaf()){
            nodes.push_back(newNode->node);
        } else{
            trees.push(newNode->leftNode);
            trees.push(newNode->rightNode);            
        }
    }
    return nodes;
}

double Tree::cardinality(QueryGraph &querygraph, Tree* left, Tree* right){
    if(isLeaf)
        return node->cardinality;
    double cardinality = left->cardinality(querygraph) * right->cardinality(querygraph);
    std::vector<QueryNode> leftRelations = getRelations(left);
    std::vector<QueryNode> rightRelations = getRelations(right);
    
    for(auto &node : leftRelations){
        for (auto& edge : querygraph.getEdges(getNode(node))) {
            int next = edge.other(cur);
            if(std::find(rightRelations.begin(), rightRelations.end(), querygraphgetNode(next)) != rightRelations.end()){
                cardinality *= edge.selectivity;
            }
        }
    }
    return cardinality;
}

int Tree::cost(QueryGraph& querygraph, Tree* left, Tree* right){
    if(isLeaf)
        return 0;
    return cardinality(querygraph, left, right) + left->cost(querygraph) + right->cost(querygraph);    
}

Tree Tree::GOO(QueryGraph& querygraph){
    std::vector<Tree> trees;
    
    for(auto &node : querygraph.getAllNodes()){
        trees.push_back(Tree(node));
    }

    while(trees.size() > 1){
        int minCost = cost(querygraph, &(trees[0]), &(trees[1]));
        int leftIdx = 0, rightIdx = 1, currentLeftIdx = 0, currentRightIdx = 0;
        for(auto leftIterator = trees.begin(); leftIterator != trees.end(); ++leftIterator, ++currentLeftIdx){
            currentRightIdx = 0;
            for(auto rightIterator = leftIterator + 1; rightIterator != tree.end(); ++rightIterator, ++currentRightIdx){
                int currentCost = cost(querygraph, leftIterator, rightIterator);
                if(currentCost < minCost){
                    minCost = currentCost;
                    leftIdx = currentLeftIdx;
                    rightIdx = currentRightIdx;
                }
            }
        }
	trees.push_back(Tree(querygraph, &(trees[leftidx]), &(trees[rightIdx])));
        trees.erase(trees.begin() + leftIdx);
        trees.erase(trees.begin() + rightIdx - 1);
    }
    return trees[0];
}

unique_ptr<Operator> TableScanNode::execute()
{
    return move(scan);
}

unique_ptr<Operator> CrossProductNode::execute()
{
    return unique_ptr<CrossProduct>(
        new CrossProduct(left->execute(), right->execute()));
}

unique_ptr<Operator> SelectNode::execute()
{
    return unique_ptr<Selection>(
        new Selection(child->execute(), regl, regr));
}

unique_ptr<Operator> HashJoinNode::execute()
{
    return unique_ptr<HashJoin>(
        new HashJoin(left->execute(), right->execute(),
                     regl, regr));
}


void QueryPlan::setRoot(unique_ptr<OperatorNode> root)
{
    this->root = root;
}

const Register* QueryPlan::getRegister(const string& bind, const string& attr)
{
    return registers[bind][attr];
}

unique_ptr<Operator> QueryPlan::execute()
{
    if (!root) {
        return unique_ptr<Operator>();
    } else {
        return root->execute();
    }
}

unordered_map<string, unique_ptr<Tablescan>> QueryPlan::init(Database& db, const vector<SqlBinding>& bindings)
{
    unordered_map<string, unique_ptr<Tablescan>> retval;    //binding, scan
    
    for (auto& bind : bindings) {
        Table& mytable = db.getTable(bind.relation.value);
        unique_ptr<Tablescan> myscan(new Tablescan(mytable));
        
        for (int i = 0; i < mytable.getAttributeCount(); i++) {
            const Attribute& attr = mytable.getAttribute(i);
            const Register* reg = myscan->getOutput(attr.getName());
            registers[bind.binding.value][attr.getName()] = reg;
        }
        
        retval.emplace(bind.binding.value, move(myscan));
    }
    
    return retval;
}

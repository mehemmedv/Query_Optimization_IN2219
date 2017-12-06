#include <string>
#include "QueryPlan.hpp"
#include "operator/CrossProduct.hpp"
#include "operator/HashJoin.hpp"
#include "operator/Selection.hpp"
#include "operator/Operator.hpp"
#include "operator/Projection.hpp"

std::vector<QueryNode> Tree::getRelations(Tree* tree){
    std::vector<QueryNode> nodes;
    std::queue<Tree*> trees;
    trees.push(tree);
    while(!trees.empty()) {
        Tree* newNode = trees.front();
        trees.pop();
        if (newNode->isLeaf) {
            nodes.push_back(*(newNode->node));
        } else {
            trees.push(newNode->leftTree);
            trees.push(newNode->rightTree);            
        }
    }
    return nodes;
}

double Tree::cardinality(QueryGraph &querygraph, Tree* left, Tree* right){
    if(isLeaf)
        return node->cardinality;
    if(left == NULL){
    	left = this->leftTree;
    	right= this->rightTree;
    }
    double cardinality = left->cardinality(querygraph, left->leftTree, left->rightTree) * right->cardinality(querygraph, right->leftTree, right->rightTree);
    std::vector<QueryNode> leftRelations = getRelations(left);
    std::vector<QueryNode> rightRelations = getRelations(right);
    for(auto &node : leftRelations){
        for (auto& edge : querygraph.getEdges(querygraph.getNode(node.index))) {
            int next = edge.other(node.index);
            for(auto &rightNode : rightRelations)
                if(rightNode.index == next)
                    cardinality *= edge.selectivity;
        }
    }
    return cardinality;
}

void Tree::print_rec(QueryGraph& querygraph){
    if(isLeaf){
    	cout<<node->binding.binding.value;
    	return;
    }
    
    cout<<"(";
    leftTree->print_rec(querygraph);
    cout<<") |><| (";
    rightTree->print_rec(querygraph);
    cout<<")";
}

string Tree::print_cost(QueryGraph& querygraph, int &index){
    if(isLeaf){
    	return node->binding.binding.value;
    }
    
    int cur_idx = index;
    string l_idx = leftTree->print_cost(querygraph, ++index);
    string r_idx = rightTree->print_cost(querygraph, ++index);
    int cost_join = cost(querygraph, leftTree, rightTree);
    cout<<"P" + std::to_string(cur_idx)<<" = (";
    cout<<l_idx;
    cout<<"|><|";
    cout<<r_idx;
    cout<<") "<<cost_join<<endl;
    return "P" + std::to_string(cur_idx);
}

void Tree::print(QueryGraph& querygraph){
    cout<<"Join order: ";
    print_rec(querygraph);
    cout<<endl;
    int index = 1;
    print_cost(querygraph, index);
}

int Tree::cost(QueryGraph& querygraph, Tree *left, Tree *right){
    if(isLeaf)
        return 0;
    double car = cardinality(querygraph, left, right);
    if(left == NULL){
    	left = this->leftTree;
    	right = this->rightTree;
    }
    return car + left->cost(querygraph) + right->cost(querygraph);    
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
    this->root.reset(root.release());
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

unique_ptr<Operator> QueryPlan::execute(const SqlParse& parse)
{
    if (!root) {
        return unique_ptr<Operator>();
    }
    
    vector<const Register*> projection;
    
    if (!parse.projection.empty() && parse.projection.begin()->value == "*") {
        for (const auto& regit : registers) {
            for (const auto& regit2 : regit.second) {
                projection.push_back(regit2.second);
            }
        }
    } else {
        for (const auto& tok : parse.projection) {
            for (const auto& regit : registers) {
                auto it = regit.second.find(tok.value);
                if (it != regit.second.end()) {
                    projection.push_back(it->second);
                    break;
                }
            }
        }
    }
    
    return unique_ptr<Operator>(new Projection(root->execute(), projection));
    
}

shared_ptr<Register> QueryPlan::createConstRegister()
{
    constRegisters.emplace_back(new Register());
    return constRegisters.back();
}

unordered_map<string, unique_ptr<Tablescan>> QueryPlan::init(Database& db, const vector<shared_ptr<SqlBinding>>& bindings)
{
    unordered_map<string, unique_ptr<Tablescan>> retval;    //binding, scan
    
    for (auto& bind : bindings) {
        Table& mytable = db.getTable(bind->relation.value);
        unique_ptr<Tablescan> myscan(new Tablescan(mytable));
        
        for (int i = 0; i < mytable.getAttributeCount(); i++) {
            const Attribute& attr = mytable.getAttribute(i);
            const Register* reg = myscan->getOutput(attr.getName());
            registers[bind->binding.value][attr.getName()] = reg;
        }
        
        retval.emplace(bind->binding.value, move(myscan));
    }
    
    return retval;
}

ostream& operator<<(ostream& out, const OperatorNode& node)
{
    node.output(out);
    return out;
}

ostream& operator<<(ostream& out, const QueryPlan& plan)
{
    plan.output(out);
    out << endl;
    return out;
}

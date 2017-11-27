#ifndef H_QueryPlan
#define H_QueryPlan

#include "QueryGraph.hpp"
#include "Database.hpp"
#include "SimpleParser.hpp"
#include "Register.hpp"
#include "operator/Tablescan.hpp"
#include <memory>
#include <queue>
#include <unordered_map>

using namespace std;

class Tree {
private:
    bool isLeaf;
    QueryNode* node;
    Tree *leftTree, *rightTree;
    std::vector<QueryNode> getRelations(Tree* tree);

public:

    Tree(QueryNode& querynode) : isLeaf(true), node(&querynode), leftTree(NULL), rightTree(NULL){}
    
    Tree(Tree* leftTree, Tree* rightTree) : isLeaf(false), leftTree(leftTree), rightTree(rightTree){}
    
    double cardinality(QueryGraph &querygarph, Tree* left = NULL, Tree* right = NULL);
    
    int cost(QueryGraph& querygraph, Tree *left = NULL, Tree *right = NULL);
    
    void print(QueryGraph& querygraph);
    
    void print_rec(QueryGraph& querygraph);
    
    string print_cost(QueryGraph& querygraph, int &index);
};

struct OperatorNode
{
    virtual unique_ptr<Operator> execute() = 0;
};

struct TableScanNode : OperatorNode
{
    unique_ptr<Tablescan> scan;
    
    unique_ptr<Operator> execute();
};

struct CrossProductNode : OperatorNode
{
    unique_ptr<OperatorNode> left;
    unique_ptr<OperatorNode> right;
    
    unique_ptr<Operator> execute();
};

struct SelectNode : OperatorNode
{
    unique_ptr<OperatorNode> child;
    
    const Register* regl;
    const Register* regr;
    
    unique_ptr<Operator> execute();
};

struct HashJoinNode : OperatorNode
{
    unique_ptr<OperatorNode> left;
    unique_ptr<OperatorNode> right;
    
    const Register* regl;
    const Register* regr;
    
    unique_ptr<Operator> execute();
};

class QueryPlan
{
private:
    unordered_map<string, unordered_map<string, const Register*>> registers; //bind, attr, reg
    unique_ptr<OperatorNode> root;
public:
    QueryPlan() {}
    
    unordered_map<string, unique_ptr<Tablescan>> init(Database& db, const vector<SqlBinding>& bindings);
    
    void setRoot(unique_ptr<OperatorNode> root);
    const Register* getRegister(const string& bind, const string& attr);
    
    unique_ptr<Operator> execute();
};

#endif

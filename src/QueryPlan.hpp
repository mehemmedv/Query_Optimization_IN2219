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
    Tree() : isLeaf(true){}

    ~Tree(){
        if(leftTree != NULL)
            delete leftTree;
        if(rightTree != NULL)
            delete rightTree;
    }

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
    virtual void output(ostream& out) const { out << "()"; }
};

struct TableScanNode : OperatorNode
{
    unique_ptr<Tablescan> scan;
    
    TableScanNode(unique_ptr<Tablescan> scan) : scan(move(scan)) {}
    
    void output(ostream& out) const { 
        out << "(Tablescan ";
        out << scan->getTable().getName();
        out << ")";
    }
    
    unique_ptr<Operator> execute();
};

struct CrossProductNode : OperatorNode
{
    unique_ptr<OperatorNode> left;
    unique_ptr<OperatorNode> right;
    
    CrossProductNode(unique_ptr<OperatorNode> left, unique_ptr<OperatorNode> right) 
        : left(move(left)), right(move(right)) {}
        
    void output(ostream& out) const { 
        out << "(Cross ";
        left->output(out);;
        out << " ";
        right->output(out);
        out << ")";
    }
    
    unique_ptr<Operator> execute();
};

struct SelectNode : OperatorNode
{
    unique_ptr<OperatorNode> child;
    
    vector<const Register*> regl;
    vector<const Register*> regr;
    
    SelectNode(unique_ptr<OperatorNode> child, vector<const Register*> regl, vector<const Register*> regr) 
        : child(move(child)), regl(move(regl)), regr(move(regr)) {}
    
    void output(ostream& out) const { 
        out << "(Select ";
        child->output(out);
        out << ")";
    }
    
    unique_ptr<Operator> execute();
};

struct HashJoinNode : OperatorNode
{
    unique_ptr<OperatorNode> left;
    unique_ptr<OperatorNode> right;
    
    vector<const Register*> regl;
    vector<const Register*> regr;
    
    HashJoinNode(unique_ptr<OperatorNode> left, unique_ptr<OperatorNode> right, vector<const Register*> regl, vector<const Register*> regr) 
        : left(move(left)), right(move(right)), regl(move(regl)), regr(move(regr)) {}
    
    void output(ostream& out) const { 
        out << "(HashJoin ";
        left->output(out);;
        out << " ";
        right->output(out);
        out << ")";
    }
    
    unique_ptr<Operator> execute();
};

ostream& operator<<(ostream& out, const OperatorNode& node);

class QueryPlan
{
private:
    unordered_map<string, unordered_map<string, const Register*>> registers; //bind, attr, reg
    vector<shared_ptr<Register>> constRegisters;
    unique_ptr<OperatorNode> root;
public:
    QueryPlan() {}
    
    unordered_map<string, unique_ptr<Tablescan>> init(Database& db, const vector<shared_ptr<SqlBinding>>& bindings);
    
    void setRoot(unique_ptr<OperatorNode> root);
    const Register* getRegister(const string& bind, const string& attr);
    shared_ptr<Register> createConstRegister();
    
    void output(ostream& out) const { if (root) root->output(out); }
    
    unique_ptr<Operator> execute();
    unique_ptr<Operator> execute(const SqlParse& parse);
};

ostream& operator<<(ostream& out, const QueryPlan& plan);

#endif

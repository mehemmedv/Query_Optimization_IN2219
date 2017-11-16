#ifndef H_QueryPlan
#define H_QueryPlan

#include "Database.hpp"
#include "SimpleParser.hpp"
#include "Register.hpp"
#include "operator/Tablescan.hpp"
#include <memory>
#include <unordered_map>

using namespace std;

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
    
    Register* regl;
    Register* regr;
    
    unique_ptr<Operator> execute();
};

struct HashJoinNode : OperatorNode
{
    unique_ptr<OperatorNode> left;
    unique_ptr<OperatorNode> right;
    
    Register* regl;
    Register* regr;
    
    unique_ptr<Operator> execute();
};

class QueryPlan
{
private:
    unordered_map<pair<string,string>, Register*> registers; //bind, attr, reg
    unique_ptr<OperatorNode> root;
public:
    QueryPlan() {}
    
    unordered_map<string, unique_ptr<Tablescan>> init(Database& db, const vector<SqlBinding>& bindings);
    
    void setRoot(unique_ptr<OperatorNode> root);
    Register* getRegister(const string& bind, const string& attr);
    
    unique_ptr<Operator> execute();
};

#endif
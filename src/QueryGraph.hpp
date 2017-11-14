#ifndef H_QueryGraph
#define H_QueryGraph

#include "SimpleParser.hpp"
#include <string>
#include <unordered_map>

using namespace std;

class QueryEdge
{
public:
    int index;
    float selectivity;
    int nodeA;
    int nodeB;
    SqlPredicate predicate; //not a vector because operators dont
                            //support multi predicate/register joins
};

class QueryNode
{
public:
    int index;
    int cardinality;
    SqlBinding binding;
    vector<SqlPredicate> predicates;
};

class QueryGraph 
{
private:
    unordered_map<string, int> bindingToIndex;
    vector<QueryNode> nodes;
    vector<QueryEdge> edges;
    vector<vector<int>> adjacencyList;  //int is index of edge in edges
public:
    QueryGraph();
    
    QueryNode& getNode(string binding);
    QueryNode& getNode(int index);
    QueryEdge& getEdge(int index);
    vector<int>& getEdgeIndices(const QueryNode&);
    
    void emplaceNode(SqlBinding, vector<SqlPredicate>, int cardinality);
    void emplaceEdge(const QueryNode& nodeA, const QueryNode& nodeB, SqlPredicate, float selectivity);
    
    bool checkCycle();
    QueryGraph buildMST();
};

#endif
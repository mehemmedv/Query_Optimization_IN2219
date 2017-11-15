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
    
    int other(int current) { return (current == nodeA) ? nodeB : nodeA; }
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
    QueryGraph() {}
    
    QueryNode& getNode(const string& binding);
    QueryNode& getNode(int index);
    QueryEdge& getEdge(int index);
    
    vector<QueryNode>& getAllNodes() { return nodes; }
    vector<QueryEdge>& getAllEdges() { return edges; }
    
    vector<int>& getEdgeIndices(const QueryNode&);
    
    void emplaceNode(SqlBinding, vector<SqlPredicate>, int cardinality);
    void emplaceEdge(const QueryNode& nodeA, const QueryNode& nodeB, SqlPredicate, float selectivity);
    
    bool checkCycle();
    QueryGraph buildMST();
    
    class EdgeIterable
    {
    private:
        const vector<int>& indices;
        vector<QueryEdge>& edges;
    public:
        EdgeIterable(vector<int>& indices, vector<QueryEdge>& edges)
            : indices(indices), edges(edges) {}
        
        class EdgeIterator
        {
        private:
            vector<int>::const_iterator cur;
            vector<QueryEdge>& edges;
        public:
            EdgeIterator(vector<QueryEdge>& edges, vector<int>::const_iterator beg) 
                : edges(edges), cur(beg) {}
            
            EdgeIterator& operator++() { cur++; return *this; }
            
            bool operator==(const EdgeIterator& other) { return cur == other.cur; }
            bool operator!=(const EdgeIterator& other) { return cur != other.cur; }
            QueryEdge& operator*() { return edges[*cur]; }
        };
        
        EdgeIterator begin() { return EdgeIterator(edges, indices.begin()); }
        EdgeIterator end() { return EdgeIterator(edges, indices.end()); }
    };
    
    EdgeIterable getEdges(const QueryNode& node) { return EdgeIterable(adjacencyList[node.index], edges); };
};

#endif
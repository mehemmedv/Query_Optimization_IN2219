#ifndef H_QueryGraph
#define H_QueryGraph

#include "SimpleParser.hpp"
#include <memory>
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
    vector<SqlPredicate> predicates;
    
    int other(int current) { return (current == nodeA) ? nodeB : nodeA; }
};

class QueryNode
{
public:
    int index;
    int cardinality;
    SqlBinding& binding;
    vector<SqlPredicate> predicates;
    bool operator==(const QueryNode& a) const
    {
        return index == a.index;
    }
};

class QueryGraph 
{
private:
    unordered_map<string, int> bindingToIndex;
    vector<QueryNode> nodes;
    vector<QueryEdge> edges;
    vector<unordered_map<int,int>> adjacencyList;  //to, edge_index
    vector<shared_ptr<SqlBinding>> bindings;
    
    int* par; // these 2 are used for MST(union find)
    int* weight;
    
public:
    QueryGraph() {}
    
    QueryNode& getNode(const string& binding);
    QueryNode& getNode(int index);
    QueryEdge& getEdge(int index);
    
    int getNodeCount() { return nodes.size(); }
    int getEdgeCount() { return edges.size(); }
    
    vector<QueryNode>& getAllNodes() { return nodes; }
    vector<QueryEdge>& getAllEdges() { return edges; }
    vector<shared_ptr<SqlBinding>>& getBindings() { return bindings; }
    
    void emplaceNode(SqlBinding, vector<SqlPredicate>, int cardinality);
    void emplaceEdge(const QueryNode& nodeA, const QueryNode& nodeB, const vector<SqlPredicate>&, float selectivity);
    
    bool checkCycle();
    
    bool operator()(const QueryEdge& l, const QueryEdge& r);
    
    QueryGraph buildMST();
    int find(int node);
    void unionNodes(int nodeA, int nodeB);
    
    vector<QueryGraph> getConnectedComponents();
    
    string graphviz();
    
    class EdgeIterable
    {
    private:
        const unordered_map<int,int>& indices;
        vector<QueryEdge>& edges;
    public:
        EdgeIterable(unordered_map<int,int>& indices, vector<QueryEdge>& edges)
            : indices(indices), edges(edges) {}
        
        class EdgeIterator
        {
        private:
            unordered_map<int,int>::const_iterator cur;
            vector<QueryEdge>& edges;
        public:
            EdgeIterator(vector<QueryEdge>& edges, unordered_map<int,int>::const_iterator beg) 
                : cur(beg), edges(edges) {}
            
            EdgeIterator& operator++() { cur++; return *this; }
            
            bool operator==(const EdgeIterator& other) { return cur == other.cur; }
            bool operator!=(const EdgeIterator& other) { return cur != other.cur; }
            QueryEdge& operator*() { return edges[cur->second]; }
        };
        
        EdgeIterator begin() { return EdgeIterator(edges, indices.begin()); }
        EdgeIterator end() { return EdgeIterator(edges, indices.end()); }
    };
    
    EdgeIterable getEdges(const QueryNode& node) { return EdgeIterable(adjacencyList[node.index], edges); };
};

#endif

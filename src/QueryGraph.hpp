#ifndef H_QueryGraph
#define H_QueryGraph

#include "SimpleParser.hpp"
#include "IteratorTools.hpp"
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
    void topoSortRec(vector<int>& retval, vector<bool>& vis, int cur);
    
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
    
    vector<int> topoSort();
    
    typedef uint64_t num_t;
    double getSelectivity(num_t ba, num_t bb);
    
    QueryGraph buildMST();
    int find(int node);
    void unionNodes(int nodeA, int nodeB);
    
    vector<QueryGraph> getConnectedComponents();
    
    string graphviz();
    
    class EdgeIterable
    {
    private:
        int node_index;
        QueryGraph* g;
    public:
        typedef QueryEdge& value_type;
        EdgeIterable(int node_index, QueryGraph* g)
            : node_index(node_index), g(g) {}
        
        class iterator
        {
        private:
            unordered_map<int,int>::const_iterator cur;
            QueryGraph* g;
        public:
            iterator(QueryGraph* g, unordered_map<int,int>::const_iterator beg) 
                : cur(beg), g(g) {}
            
            iterator& operator++() { cur++; return *this; }
            
            bool operator==(const iterator& other) const { return cur == other.cur; }
            bool operator!=(const iterator& other) const { return cur != other.cur; }
            QueryEdge& operator*() { return g->edges[cur->second]; }
        };
        
        iterator begin() { return iterator(g, g->adjacencyList[node_index].begin()); }
        iterator end() { return iterator(g, g->adjacencyList[node_index].end()); }
    };
    
    EdgeIterable getEdges(const QueryNode& node) { return EdgeIterable(node.index, this); };
    
    
    
    auto iterateCrossEdges(num_t ba, num_t bb) {
        auto mr = makeRange(0, this->getNodeCount());
        auto ls = makeFilter(mr, [ba](int i) -> bool {
            return ba & (1ull << i);
        });
        
        auto rs = [this, bb](int i) -> auto {
            auto filt = makeFilter(makeReferenceIterable(this->getEdges(this->getNode(i))), [bb, i](QueryEdge& edg) -> bool { 
                int to = edg.other(i); 
                return bb & (1ull << to); 
            });
            
            return make_pair(filt.begin(), filt.end());
        };
        
        return makeCross(ls, rs);
    }
};

#endif

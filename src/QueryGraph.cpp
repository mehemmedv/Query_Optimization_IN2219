#include "QueryGraph.hpp"

#include <stack>
#include <stdexcept>
    
QueryNode& QueryGraph::getNode(const string& binding)
{
    auto it = bindingToIndex.find(binding);
    if (it == bindingToIndex.end())
        throw new invalid_argument("binding is not valid");
    
    return getNode(it->second);
}

QueryNode& QueryGraph::getNode(int index)
{
    return nodes[index];
}

QueryEdge& QueryGraph::getEdge(int index)
{
    return edges[index];
}

vector<int>& QueryGraph::getEdgeIndices(const QueryNode& node)
{
    return adjacencyList[node.index];
}

void QueryGraph::emplaceNode(SqlBinding binding, vector<SqlPredicate> preds, int cardinality)
{
    nodes.emplace_back(nodes.size(), cardinality, move(binding), move(preds));
}

void QueryGraph::emplaceEdge(const QueryNode& nodeA, const QueryNode& nodeB, SqlPredicate pred, float selectivity)
{
    int index = edges.size();
    edges.emplace_back(index, selectivity, nodeA.index, nodeB.index, move(pred));
    
    adjacencyList[nodeA.index].push_back(index);
    adjacencyList[nodeB.index].push_back(index);
}

bool QueryGraph::checkCycle()
{
    vector<bool> vis(nodes.size());
    stack<pair<int,int>> ms;
    ms.emplace(0, -1);
    
    while(!ms.empty())
    {
        auto pa = ms.top();
        int cur = pa.first;
        int pre = pa.second;
        
        if (vis[cur]) {
            return true;
        } else {
            vis[cur] = true;
        }
        
        for (auto edge : getEdges(getNode(cur))) {
            int next = edge.other(cur);
            if (next == pre) {
                continue;
            }
            ms.emplace(next, cur);
        }
    }
    
}

//QueryGraph QueryGraph::buildMST();
#include "QueryGraph.hpp"

#include <stack>
#include <stdexcept>
    
QueryNode& QueryGraph::getNode(const string& binding)
{
    auto it = bindingToIndex.find(binding);
    if (it == bindingToIndex.end()) {
        throw new invalid_argument("binding is not valid");
    }
    
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

void QueryGraph::emplaceNode(SqlBinding binding, vector<SqlPredicate> preds, int cardinality)
{
    string bindingstr = binding.binding.value;
    int index = nodes.size();
    nodes.push_back(QueryNode{index, cardinality, move(binding), move(preds)});
    bindingToIndex.emplace(move(bindingstr), index);
    adjacencyList.emplace_back();
}

template <typename T1, typename T2>
inline bool contains(const T1& container, const T2& value) {
    return container.find(value) != container.end();
}

void QueryGraph::emplaceEdge(const QueryNode& nodeA, const QueryNode& nodeB, const vector<SqlPredicate>& preds, float selectivity)
{
    auto edgeiter = adjacencyList[nodeA.index].find(nodeB.index);
    if (edgeiter != adjacencyList[nodeA.index].end()) {
        edges[edgeiter->second].selectivity *= selectivity;
        for (const auto& pred : preds) {
            edges[edgeiter->second].predicates.push_back(pred);
        }
        return;
    }
    //not found
    int index = edges.size();
    edges.push_back(QueryEdge{index, selectivity, nodeA.index, nodeB.index, preds});
    
    adjacencyList[nodeA.index].emplace(nodeB.index, index);
    adjacencyList[nodeB.index].emplace(nodeA.index, index);
}

bool QueryGraph::checkCycle()
{
    vector<bool> vis(nodes.size());
    stack<pair<int,int>> ms;
    ms.push(make_pair(0, -1));
    
    while(!ms.empty()) {
        auto pa = ms.top();
        int cur = pa.first;
        int pre = pa.second;
        
        if (vis[cur]) {
            return true;
        } else {
            vis[cur] = true;
        }
        
        for (auto& edge : getEdges(getNode(cur))) {
            int next = edge.other(cur);
            if (next == pre) {
                continue;
            }
            ms.push(make_pair(next, cur));
        }
    }
    return false;
}

QueryGraph traverse(int start, QueryGraph& original, vector<bool>& vis)
{
    QueryGraph retval;
    stack<int> ms;
    ms.push(start);
    
    vector<bool> pvis(original.getNodeCount());
    
    {
        auto& mn = original.getNode(start);
        pvis[start] = true;
        retval.emplaceNode(mn.binding, mn.predicates, mn.cardinality);
    }
    
    while(!ms.empty()) {
        int cur = ms.top();
        ms.pop();
        auto& mn = original.getNode(cur);
        
        if (vis[cur]) {
            continue;
        }
        vis[cur] = true;
        
        for (auto& edge : original.getEdges(original.getNode(cur))) {
            int next = edge.other(cur);
            ms.push(next);
            
            if (!pvis[cur]) {
                pvis[cur] = true;
                retval.emplaceNode(mn.binding, mn.predicates, mn.cardinality);
            }
            retval.emplaceEdge(retval.getNode(mn.binding.binding.value),
                                retval.getNode(original.getNode(next).binding.binding.value),
                                edge.predicates, edge.selectivity);
        }
    }
    
    return retval;
}

vector<QueryGraph> QueryGraph::getConnectedComponents()
{
    vector<QueryGraph> retval;
    vector<bool> vis(getNodeCount());
    
    for (int i = 0; i < getNodeCount(); i++) {
        if (vis[i]) {
            continue;
        }
        retval.push_back(traverse(i, *this, vis));
    }
    
    return retval;
}

string QueryGraph::graphviz()
{
    stringstream output;
    output << "graph query {";
    
    for (auto& node : nodes) {
        output << "n_" << node.index << " [label=\"";
        output << node.binding.relation << " " << node.binding.binding << " [" << node.cardinality << "]";
        output << "\"];";
    }
    
    for (auto& node : nodes) {
        if (node.predicates.empty()) {
            continue;
        }
        output << "n_" << node.index << " -- " << "n_" << node.index << " [label=\"";
        bool first = true;
        for (auto& pred : node.predicates) {
            if (first) {
                first = false;
            } else {
                output << " and ";
            }
            output << pred.lhs.binding << "." << pred.lhs.attribute;
            output << "=";
            output << pred.constant;
        }
        output << "\"];";
    }
    
    for (auto& edge : edges) {
        output << "n_" << edge.nodeA << " -- " << "n_" << edge.nodeB << " [label=\"";
        bool first = true;
        for (auto& pred : edge.predicates) {
            if (first) {
                first = false;
            } else {
                output << " and ";
            }
            output << pred.lhs.binding << "." << pred.lhs.attribute;
            output << "=";
            output << pred.rhs.binding << "." << pred.rhs.attribute;
        }
        output << " [" << edge.selectivity << "]";
        output << "\"];";
    }
    
    output << "}";
    return output.str();
}

//QueryGraph QueryGraph::buildMST();
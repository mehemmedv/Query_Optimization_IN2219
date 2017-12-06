#include "DPccp.hpp"

#include <functional>
#include "BitSubsets.hpp"

typedef uint64_t num_t;

num_t getN(QueryGraph& graph, vector<int>& bfsid, num_t bmap) 
{
    num_t retval = 0;
	for(int i = 0; i < graph.getNodeCount(); i++) {
		if (bmap & (1ull<<i)) {
			for(auto edg : graph.getEdges(graph.getNode(bfsid[i]))) {
                int nx = edg.other(bfsid[i]);
				retval = retval | (1ull<<nx);
			}
		}
	}
	return retval & ~bmap;
}

void enumerateCsgRec(QueryGraph& graph, vector<int>& bfsid, vector<num_t>& out, num_t smap, num_t xmap)
{
	num_t nmap = getN(graph, bfsid, smap) & ~xmap;
    
	for (auto s1 : bitsubsets(nmap)) {
        out.push_back(s1 | smap);
    }
    
    for (auto s1 : bitsubsets(nmap)) {
        enumerateCsgRec(graph, bfsid, out, s1 | smap, xmap | nmap);
    }
}

vector<num_t> enumerateCsg(QueryGraph& graph, vector<int>& bfsid)
{
    vector<num_t> out;
    for(int i = graph.getNodeCount() - 1; i>=0 ; i--)
	{
		out.push_back(1ull<<i);
		enumerateCsgRec(graph, bfsid, out, 1ull<<i, (1ull<<i) - 1);
	}
    return out;
}

vector<num_t> enumerateCmp(QueryGraph& graph, vector<int>& bfsid, num_t smap)
{
    vector<num_t> out;
	num_t xmap = ((smap & -smap) - 1) | smap;
	num_t nmap = getN(graph, bfsid, smap) & ~xmap;
	for(int i = graph.getNodeCount() - 1; i>=0; i--) {
		if (nmap & (1<<i)) {
			out.push_back(1<<i);
			enumerateCsgRec(graph, bfsid, out, 1ull<<i, xmap | ( ((1ull<<i) - 1) & nmap )  );
		}
	}
    return out;
}

struct dpinfo 
{
    int64_t cout;
    int64_t card;
    num_t prea;
    num_t preb;
    
    dpinfo() : cout(LLONG_MAX), card(LLONG_MAX), prea(-1), preb(-1) {}
    dpinfo(int64_t cout, int64_t card, num_t prea, num_t preb) : cout(cout), card(card), prea(prea), preb(preb) {}
    
    bool operator<(const dpinfo& other) {
        return card < other.card;
    }
};

double getS(QueryGraph& graph, vector<int>& bfsid, num_t abfs, num_t bbfs)
{
    num_t b = 0;
    
    for (int i = 0; i < graph.getNodeCount(); i++) {
        if (bbfs & (1ull << i)) {
            b |= 1ull << bfsid[i];
        }
    }
    
    double result = 1.0;
    
    for (int i = 0; i < graph.getNodeCount(); i++) {
        if (abfs & (1ull << i)) {
            int ind = bfsid[i];
            
            for (auto& edg : graph.getEdges(graph.getNode(ind))) {
                int to = edg.other(ind);
                if (b & (1ull << to)) {
                    result *= edg.selectivity;
                }
            }
        }
    }
    
    return result;
}

shared_ptr<OperatorNode> dpBuildOperatorRec(QueryGraph& graph, QueryPlan& plan,
        unordered_map<string, unique_ptr<Tablescan>> scans, vector<int>& bfsid,
        vector<dpinfo>& dp, num_t cur)
{
    if (dp[cur].prea == -1) {
        auto& node = graph.getNode(bfsid[dp[cur].preb]);
        auto it = scans.find(node.binding.binding.value);
        
        if (it == scans.end()) {
            throw exception("Binding not found.");
        }
        
        unique_ptr<Operator> mo(move(it->second));
        scans.erase(it);
    }
}

shared_ptr<OperatorNode> dpccpPlanConn(QueryGraph& graph, QueryPlan& plan,
        unordered_map<string, unique_ptr<Tablescan>> scans)
{
    vector<int> bfsid(graph.getNodeCount());    //bfsid to nodeid
    //vector<int> bfsrid(graph.getNodeCount());   //nodeid to bfsid
    
    //fill bfsid
    {
        int lbfsid = 0;
        queue<int> mq;
        mq.push(0);
        
        vector<bool> vis(graph.getNodeCount());
        
        while (!mq.empty()) {
            int cur = mq.front();
            mq.pop();
            
            if (vis[cur]) {
                continue;
            }
            vis[cur] = true;
            
            bfsid[lbfsid] = cur;
            lbfsid++;
            
            for (auto& edg : graph.getEdges(graph.getNode(cur))) {
                int to = edg.other(cur);
                mq.push(to);
            }
        }
        
        //for (int i = 0; i < bfsid.size(); i++) {
        //    bfsrid[bfsid[i]] = i;
        //}
    }
    
    vector<dpinfo> dp(1ull << graph.getNodeCount());
    
    for (int i = 0; i < graph.getNodeCount(); i++) {
        int cout = 0;
        int card = graph.getNode(bfsid[i]).cardinality;
        int prea = -1;
        int preb = -1;
        dp[1ull << i] = dpinfo(cout, card, prea, preb);
    }
    
    for (auto a : enumerateCsg(graph, bfsid)) {
        for (auto b : enumerateCmp(graph, bfsid, a)) {
            dpinfo newinfo;
            newinfo.card = dp[a].card * dp[b].card * getS(graph, bfsid, a, b);
            newinfo.cout = newinfo.card + dp[a].cout + dp[b].cout;
            newinfo.prea = a;
            newinfo.preb = b;
            
            dp[a | b] = min(dp[a | b], newinfo);
        }
    }
    
    
    return dpBuildOperatorRec(graph, plan, scans, bfsid, dp, (1ull << graph.getNodeCount()) - 1);
}

QueryPlan dpccpPlan(Database& db, QueryGraph& graph)
{
    auto comp = graph.getConnectedComponents();
    
    QueryPlan myplan;
    auto scans = myplan.init(db, graph.getBindings());
}
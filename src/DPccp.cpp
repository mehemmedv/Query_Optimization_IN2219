#include "DPccp.hpp"

#include <bitset>
#include <functional>
#include <limits>
#include "IteratorTools.hpp"
#include "operator/Operator.hpp"
#include "operator/Selection.hpp"

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
    bool empty;
    bool used;
    
    const static int64_t maxval = numeric_limits<int64_t>::max();
    
    dpinfo() : cout(maxval), card(maxval), prea(0), preb(0), empty(true), used(false) {}
    dpinfo(int64_t cout, int64_t card, num_t prea, num_t preb, bool empty = false) 
        : cout(cout), card(card), prea(prea), preb(preb), empty(empty), used(true) {}
    
    bool operator<(const dpinfo& other) const {
        return card < other.card;
    }
};

void iterateCrossEdges(QueryGraph& graph, vector<int>& bfsid, num_t abfs, num_t bbfs, function<void(QueryEdge&, int)> foo)
{
    num_t b = 0;
    
    for (int i = 0; i < graph.getNodeCount(); i++) {
        if (bbfs & (1ull << i)) {
            b |= 1ull << bfsid[i];
        }
    }
    
    for (int i = 0; i < graph.getNodeCount(); i++) {
        if (abfs & (1ull << i)) {
            int ind = bfsid[i];
            
            for (auto& edg : graph.getEdges(graph.getNode(ind))) {
                int to = edg.other(ind);
                if (b & (1ull << to)) {
                    foo(edg, ind);
                }
            }
        }
    }
}

double getS(QueryGraph& graph, vector<int>& bfsid, num_t abfs, num_t bbfs)
{
    double result = 1.0;
    
    iterateCrossEdges(graph, bfsid, abfs, bbfs, [&](QueryEdge& edg, int from){
        result *= edg.selectivity;;
    });
    
    return result;
}

unique_ptr<OperatorNode> dpBuildOperatorRec(QueryGraph& graph, QueryPlan& plan,
        unordered_map<string, unique_ptr<Tablescan>>& scans, vector<int>& bfsid,
        vector<dpinfo>& dp, num_t cur)
{
    if (dp[cur].empty) {
        auto& node = graph.getNode(bfsid[dp[cur].preb]);
        auto it = scans.find(node.binding.binding.value);
        
        if (it == scans.end()) {
            throw runtime_error("Binding not found.");
        }
        
        unique_ptr<OperatorNode> mo(new TableScanNode(move(it->second)));
        scans.erase(it);
        
        vector<const Register*> lregs;
        vector<const Register*> rregs;
        
        //order?
        for (auto& pred : node.predicates) {
            auto regptr = plan.createConstRegister();
            switch (pred.constant.type) {
                case Token::Type::tok_lit_bool:
                    regptr->setBool(pred.constant.boolValue);
                    break;
                case Token::Type::tok_lit_dbl:
                    regptr->setDouble(pred.constant.doubleValue);
                    break;
                case Token::Type::tok_lit_int:
                    regptr->setInt(pred.constant.intValue);
                    break;
                case Token::Type::tok_lit_str:
                    regptr->setString(pred.constant.value);
                    break;
                default:
                    break;
            }
            lregs.push_back(plan.getRegister(pred.lhs.binding.value, pred.lhs.attribute.value));
            rregs.push_back(regptr.get());
        }
        
        if (!lregs.empty() && !rregs.empty()) {
            mo.reset(new SelectNode(move(mo), move(lregs), move(rregs)));
        }
        
        return mo;
    }
    
    num_t abfs = dp[cur].prea;
    num_t bbfs = dp[cur].preb;
    
    vector<const Register*> lregs;
    vector<const Register*> rregs;
    
    unique_ptr<OperatorNode> left = dpBuildOperatorRec(graph, plan, scans, bfsid, dp, dp[cur].prea);
    unique_ptr<OperatorNode> right = dpBuildOperatorRec(graph, plan, scans, bfsid, dp, dp[cur].preb);
    
    iterateCrossEdges(graph, bfsid, abfs, bbfs, [&](QueryEdge& edg, int from){
        for (auto& pred : edg.predicates) {
            const Register* lr = plan.getRegister(pred.lhs.binding.value, pred.lhs.attribute.value);
            const Register* rr = plan.getRegister(pred.rhs.binding.value, pred.rhs.attribute.value);
            if (pred.lhs.binding.value != graph.getNode(from).binding.binding.value) {
                swap(lr, rr);
            }
            
            lregs.push_back(lr);
            rregs.push_back(rr);
        }
    });
    
    return unique_ptr<OperatorNode>(new HashJoinNode(move(left), move(right), move(lregs), move(rregs)));
}

unique_ptr<OperatorNode> dpccpPlanConn(QueryGraph& graph, QueryPlan& plan,
        unordered_map<string, unique_ptr<Tablescan>>& scans)
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
        dp[1ull << i] = dpinfo(cout, card, 0, i, true);
    }
    
    for (auto a : enumerateCsg(graph, bfsid)) {
        for (auto b : enumerateCmp(graph, bfsid, a)) {
            auto selec = getS(graph, bfsid, a, b);
            int64_t card = max<int64_t>(1, (dp[a].card * dp[b].card) * selec);
            int64_t cout = card + dp[a].cout + dp[b].cout;
            num_t prea = a;
            num_t preb = b;
            bool empty = false;
            
            dpinfo newinfo(card, cout, prea, preb, empty);
            
            dp[a | b] = min(dp[a | b], newinfo);
        }
    }
    
    cout << "## DPTABLEBEG ##" << endl;
    
    cout << "ind\tbits\tcout\tcard\tleft\tright" << endl;
    for (int i = 1; i < dp.size(); i++) {
        if (!dp[i].used) {
            cout <<"-\t-\t-\t-\t-\t-\t" << endl;
            continue;
        }
        
        cout << i << "\t";
        cout << bitset<4>(i) << "\t";
        cout << dp[i].cout << "\t";
        cout << dp[i].card << "\t";
        if (dp[i].empty) {
            cout << "-\t-\t";
        } else {
            cout << dp[i].prea << "\t";
            cout << dp[i].preb << "\t";
        }
        cout << endl;
    }
    
    cout << "## DPTABLEEND ##" << endl;
    
    
    return dpBuildOperatorRec(graph, plan, scans, bfsid, dp, (1ull << graph.getNodeCount()) - 1);
}

QueryPlan dpccpPlan(Database& db, QueryGraph& graph)
{
    auto comp = graph.getConnectedComponents();
    
    QueryPlan plan;
    auto scans = plan.init(db, graph.getBindings());
    
    unique_ptr<OperatorNode> root;
    
    for (auto& g : comp) {
        unique_ptr<OperatorNode> cur = dpccpPlanConn(g, plan, scans);
        if (!root) {
            root.swap(cur);
        } else {
            root.reset(new CrossProductNode(move(root), move(cur)));
        }
    }
    
    plan.setRoot(move(root));
    
    return plan;
}
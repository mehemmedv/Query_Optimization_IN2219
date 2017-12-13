#include "ExhaustiveTransformation2.hpp"

#include <functional>
#include <limits>
#include <unordered_set>
#include <utility>
#include <vector>
#include <cstdint>

#include "IteratorTools.hpp"

typedef uint64_t num_t;

using namespace std;

inline void hash_combine(uint64_t& seed, uint64_t val)
{
    const uint64_t kMul = 0x9ddfea08eb382d69ULL;
    uint64_t a = (val ^ seed) * kMul;
    a ^= (a >> 47);
    uint64_t b = (seed ^ a) * kMul;
    b ^= (b >> 47);
    seed = b * kMul;
}

class ExhaustiveTransformation {
    public:
    struct SubTree {
        num_t left;
        num_t right;
        int type;
        
        SubTree() : left(0), right(0), type(0) {}
        SubTree(num_t left, num_t right) : left(left), right(right), type(0) {}
        SubTree(num_t left, num_t right, int type) : left(left), right(right), type(type) {}
        
        static SubTree makeLeaf(int ind) { return SubTree(1ull << ind, 0, -ind - 1); }
        
        bool isLeaf() const { return type < 0; }
        int getLeafInd() const { return -type - 1; }
        
        bool isComm() const { return type & 1; }
        bool isRightAssoc() const { return type & 2; }
        bool isLeftAssoc() const { return type & 4; }
        
        void disableComm() { type = type | 1; }
        void disableRightAssoc() { type = type | 2; }
        void disableLeftAssoc() { type = type | 4; }
        
        void clearDisabled() { type = 0; }
        
        void disableAll() { type = 1 | 2 | 4; }
        void enableAll() { type = 0; }
        
        inline num_t getClass() const { return left | right; }
        inline size_t hash() const { 
            uint64_t key = 0;
            hash_combine(key, left); 
            hash_combine(key, right); 
            key ^= key >> 33;
            key *= 0xff51afd7ed558ccd;
            key ^= key >> 33;
            key *= 0xc4ceb9fe1a85ec53;
            key ^= key >> 33;
            return key;
        }
        inline bool operator==(const SubTree& other) const { return hash() == other.hash(); }
        inline bool operator<(const SubTree& other) const {return hash() < other.hash(); }
    };
    
	struct SubtreeHash {
		size_t operator()(const ExhaustiveTransformation::SubTree& t) const
		{
			return t.hash();
		}
	};
    
    private:
    QueryGraph& g;
    num_t full;
    vector<unordered_set<SubTree,SubtreeHash>> unexplored;
    vector<unordered_set<SubTree,SubtreeHash>> explored;
    
    
    bool isFound(const SubTree& tree) {
        auto cls = tree.getClass();
        return ((unexplored[cls].find(tree) != unexplored[cls].end()) ||
            (explored[cls].find(tree) != explored[cls].end()));
    }
    
    void insertIfNotFound(const SubTree& tree) {
        if (!isFound(tree)) {
            unexplored[tree.getClass()].insert(tree);
        }
    }
    
    void applyTransformations(const SubTree& tree) {
        exploreClass(tree.left);
        exploreClass(tree.right);
        
        auto cls = tree.getClass();
        
        //RS1
        //comm
        if (tree.isComm()) {
            SubTree other(tree.right, tree.left);
            other.disableAll();
            insertIfNotFound(other);
        }
        
        //right assoc
        if (tree.isRightAssoc()) {
            for (auto st : makeChain(unexplored[tree.left], explored[tree.left])) {
                auto c1 = st.left;
                auto c2 = st.right;
                auto c3 = tree.right;
                
                SubTree j3(c2, c3);
                SubTree j2(c1, j3.getClass());
                
                j3.enableAll();
                
                j2.disableRightAssoc();
                j2.disableLeftAssoc();
                
                insertIfNotFound(j3);
                insertIfNotFound(j2);
            }
        }
        
        //left assoc
        if (tree.isLeftAssoc()) {
            for (auto st : makeChain(unexplored[tree.right], explored[tree.right])) {
                auto c1 = tree.left;
                auto c2 = st.left;
                auto c3 = st.right;
                
                SubTree j2(c1, c2);
                SubTree j3(j2.getClass(), c3);
                
                j2.enableAll();
                
                j3.disableRightAssoc();
                j3.disableLeftAssoc();
                
                insertIfNotFound(j2);
                insertIfNotFound(j3);
            }
        }
    }
    
    void exploreClass(uint64_t cls) {
        while (!unexplored[cls].empty()) {
            auto backit = unexplored[cls].begin();
            auto back = *backit;
            unexplored[cls].erase(backit);
            
            applyTransformations(back);
            explored[cls].insert(back);
        }
    }
    
    void initFromTopo(const vector<int>& topo) {
        if (topo.empty()) {
            return;
        }
        num_t cur = 1ull << topo.front();
        for (int i = 1; i < topo.size(); i++) {
            int ind = topo[i];
            num_t nex = cur | 1ull << ind;
            
            unexplored[nex].insert(SubTree(cur, 1ull << ind));
            
            cur = nex;
        }
    }
    
    void findBests() {
        vector<SubTree> bestres(full + 1);
        vector<int64_t> bestcosts(full + 1, numeric_limits<int64_t>::max());
        vector<int64_t> bestcards(full + 1, 0);
        
        for (num_t i = 1; i <= full; i++) {
            for (auto st : makeChain(unexplored[i], explored[i])) {
                int64_t card, cost;
                if (st.isLeaf()) {
                    int ind = st.getLeafInd();
                    card = g.getNode(ind).cardinality;
                    cost = 0;
                } else {
                    double sel = g.getSelectivity(st.left, st.right);
                    card = bestcards[st.left] * bestcards[st.right] * sel;
                    card = max<int64_t>(card, 1);
                    cost = bestcosts[st.left] + bestcosts[st.right] + card;
                } 
                
                if (cost < bestcosts[i]) {
                    bestcosts[i] = cost;
                    bestcards[i] = card;
                    bestres[i] = st;
                }
            }
        }
    }
    
    public:
    //g must be connected
    ExhaustiveTransformation(QueryGraph& g) 
        : g(g), full((1ull << g.getNodeCount()) - 1), unexplored(1ull << g.getNodeCount()), explored(1ull << g.getNodeCount()) 
    {
        vector<int> topo = g.topoSort();
        for (int i = 0; i < g.getNodeCount(); i++) {
            auto ml = SubTree::makeLeaf(i);
            explored[ml.getClass()].insert(ml);
        }
        initFromTopo(topo);
    }
    
    void getTree() {
        exploreClass(full);
    }
    
};


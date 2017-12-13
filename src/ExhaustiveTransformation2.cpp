#include "ExhaustiveTransformation2.hpp"

#include <functional>
#include <unordered_set>
#include <utility>
#include <vector>
#include <cstdint>

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
    
    private:
    vector<unordered_set<SubTree>> unexplored;
    vector<unordered_set<SubTree>> explored;
    
    bool isFound(const SubTree& tree) {
        auto cls = tree.getClass();
        return ((unexplored[cls].find(tree) != unexplored[cls].end()) ||
            (explored[cls].find(tree) != explored[cls].end()));
    }
    
    void applyTransformations(const SubTree& tree) {
        
    }
    
    void exploreClass(uint64_t bmap) {
        
    }
    
    public:
    //g must be connected
    ExhaustiveTransformation(QueryGraph g) {
        
    }
    
    
    
};

namespace std {
	template <> struct hash<ExhaustiveTransformation::SubTree> {
		size_t operator()(const ExhaustiveTransformation::SubTree& t) const
		{
			return t.hash();
		}
	};
}
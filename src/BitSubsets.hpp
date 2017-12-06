#ifndef H_BitSubsets
#define H_BitSubsets

#include <cstdint>

using namespace std;

class bitsubsets 
{
    typedef uint64_t num_t;
    
    num_t bits;
    
public:
    bitsubsets(num_t bits) : bits(bits) {}
    
    class bitsubsetIterator
    {
        num_t nmap;
        num_t s1;
        num_t s2;
        
    public:
        bitsubsetIterator(num_t nmap, num_t s1, num_t s2) : nmap(nmap), s1(s1), s2(s2) {}
        
        bool operator==(bitsubsetIterator other) {
            return nmap == other.nmap && s1 == other.s1;
        }
        
        bool operator!=(bitsubsetIterator other) {
            return nmap != other.nmap || s1 != other.s1;
        }
        
        void operator++() {
            s1 = nmap & (s1 - nmap);
            s2 = nmap - s1;
        }
        
        num_t operator*() {
            return s1;
        }
    };
    
    bitsubsetIterator begin() {
        if (bits == 0) {
            return bitsubsetIterator(0, 0, 0);
        }
        
        num_t nmap = bits;
        
        num_t s1 = nmap & -nmap;
        num_t s2 = nmap - s1;
        
        return bitsubsetIterator(nmap, s1, s2);
    }
    
    bitsubsetIterator end() {
        if (bits == 0) {
            return bitsubsetIterator(0, 0, 0);
        }
        
        return bitsubsetIterator(bits, 0, bits);
    }
};

#endif
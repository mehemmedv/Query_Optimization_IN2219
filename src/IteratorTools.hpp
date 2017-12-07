#ifndef H_IteratorTools
#define H_IteratorTools

#include <cstdint>
#include <vector>
#include <memory>
#include <functional>
#include <type_traits>

using namespace std;

class bitsubsets 
{
    typedef uint64_t num_t;
    
    
    num_t bits;
    
public:
    typedef num_t value_type;
    bitsubsets(num_t bits) : bits(bits) {}
    
    class iterator
    {
        num_t nmap;
        num_t s1;
        num_t s2;
        
    public:
        iterator(num_t nmap, num_t s1, num_t s2) : nmap(nmap), s1(s1), s2(s2) {}
        
        bool operator==(iterator other) {
            return nmap == other.nmap && s1 == other.s1;
        }
        
        bool operator!=(iterator other) {
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
    
    iterator begin() {
        if (bits == 0) {
            return iterator(0, 0, 0);
        }
        
        num_t nmap = bits;
        
        num_t s1 = nmap & -nmap;
        num_t s2 = nmap - s1;
        
        return iterator(nmap, s1, s2);
    }
    
    iterator end() {
        if (bits == 0) {
            return iterator(0, 0, 0);
        }
        
        return iterator(bits, 0, bits);
    }
};

template<typename T1v, typename T2v=T1v, typename T1=vector<T1v>, typename T2=T1>
class Zip
{
    typedef typename T1::iterator T1iter;
    typedef typename T2::iterator T2iter;
    
    T1& t1;
    T2& t2;
    
public:
    typedef pair<T1,T2> value_type;
    Zip(T1& t1, T2& t2) : t1(t1), t2(t2) {}
    
    class iterator {
        T1iter it1;
        T2iter it2;
        
    public:
        iterator(T1iter it1, T2iter it2) : it1(move(it1)), it2(move(it2)) {}
        
        void operator++() { it1++; it2++; }
        
        inline bool operator==(const iterator& other) const {
            return it1 == other.it1 || it2 == other.it2;
        }
        
        inline bool operator!=(const iterator& other) const {
            return !operator==(other);
        }
        
        auto operator*() const {
            return make_pair(*it1, *it2);
        }
    };
    
    iterator begin() { return iterator(t1.begin(), t2.begin()); }
    iterator end() { return iterator(t1.end(), t2.end()); }
};

template<typename T, typename R, typename C=vector<T>>
class Map {
    typedef typename C::iterator Titer;
    typedef function<R(T)> F;
    
    
    C& cont;
    F foo;
public:
    typedef R value_type;
    Map(C& cont, F foo) : cont(cont), foo(foo) {}
    
    class iterator {
        Titer it;
        const F& foo;
        shared_ptr<R> cache;
        
    public:
        iterator(Titer it, F foo) : it(move(it)), foo(foo), cache(NULL) {}
        
        void operator++() { it++; cache.reset(); }
        
        R operator*() {
            if (!cache) {
                cache.reset(new R(foo(*it)));
            }
            return *cache;
        }
        
        bool operator==(const iterator& other) const { return it == other.it; }
        bool operator!=(const iterator& other) const { return it != other.it; }
    };
    
    iterator begin() const { return iterator(cont.begin(), foo); }
    iterator end() const { return iterator(cont.end(), foo); }
};

template <typename T, typename R=typename T::value_type>
vector<R> toVector(const T& cont) { 
    vector<R> retval;
    for (auto val : cont) {
        retval.push_back(val);
    }
    return retval;
}

//template <typename T>
//vector<typename T::value_type> toVector(T&& cont) { return vector<typename T::value_type>(cont.begin(), cont.end()); }

#endif
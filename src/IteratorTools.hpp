#ifndef H_IteratorTools
#define H_IteratorTools

#include <cstdint>
#include <vector>
#include <memory>
#include <functional>
#include <type_traits>
#include <iostream>

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

template<typename T1, typename T2=T1>
class Zip
{
    typedef typename T1::iterator T1iter;
    typedef typename T2::iterator T2iter;
    typedef typename T1::value_type T1v;
    typedef typename T2::value_type T2v;
    
    T1 t1;
    T2 t2;
    
public:
    typedef pair<T1v,T2v> value_type;
    Zip(T1 t1, T2 t2) : t1(move(t1)), t2(move(t2)) {}
    
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

template<typename T1, typename T2>
inline Zip<T1,T2> makeZip(T1 t1, T2 t2) {return Zip<T1, T2>(move(t1), move(t2)); }

template<typename T1, typename F, typename ret_pair>
class Cross
{
    typedef typename T1::iterator T1iter;
    typedef typename T1::value_type T1v;
    typedef typename ret_pair::first_type T2iter;
    typedef typename T2iter::value_type T2v;
    
    T1 t1;
    F foo;
    
public:
    typedef pair<T1v,T2v> value_type;
    Cross(T1 t1, F foo) : t1(move(t1)), foo(foo) {}
    
    class iterator {
        T1iter it1;
        T1iter it1end;
        T2iter it2;
        T2iter it2end;
        F foo;
        
    public:
        typedef pair<T1v,T2v> value_type;
        iterator(T1iter it1p, T1iter it1endp, pair<T2iter, T2iter> mp, F foo) 
            : it1(move(it1p)), it1end(move(it1endp)), it2(move(mp.first)), it2end(move(mp.second)), foo(foo) 
        {
            while (it1 != it1end && it2 == it2end) {
                if (it1 != it1end) {
                    ++it1;
                }
                if (it1 != it1end) {
                    auto mp = foo(*it1);
                    new (&it2) T2iter(mp.first);
                    new (&it2end) T2iter(mp.second);
                    //it2 = mp.first;
                    //it2end = mp.second;
                }
            }
        }
        
        void operator++() {
            ++it2;
            while (it1 != it1end && it2 == it2end) {
                if (it1 != it1end) {
                    ++it1;
                }
                if (it1 != it1end) {
                    auto mp = foo(*it1);
                    new (&it2) T2iter(mp.first);
                    new (&it2end) T2iter(mp.second);
                    //it2 = mp.first;
                    //it2end = mp.second;
                }
            }
        }
        
        inline bool operator==(const iterator& other) const {
            //TODO: implement condition for not end
            return it1 == other.it1;
        }
        
        inline bool operator!=(const iterator& other) const {
            return !operator==(other);
        }
        
        value_type operator*() {
            return value_type(*it1, *it2);
        }
    };
    
    iterator begin() { return iterator(t1.begin(), t1.end(), foo(*t1.begin()), foo); }
    iterator end() { return iterator(t1.end(), t1.end(), foo(*t1.begin()), foo); }
};

template<typename T1, typename F>
inline auto makeCross(T1 t1, F foo) { return Cross<T1, F, decltype(foo(*t1.begin()))>(move(t1), foo); }


template<typename C>
class Chain
{
    typedef typename C::iterator Citer;
    typedef typename C::value_type T;
    
    C c1;
    C c2;
    
public:
    typedef pair<T,T> value_type;
    Chain(C c1, C c2) : c1(move(c1)), c2(move(c2)) {}
    
    class iterator {
        Citer it1;
        Citer it1end;
        Citer it2;
        
    public:
        iterator(Citer it1, Citer it1end, Citer it2) : it1(move(it1)), it1end(move(it1end)), it2(move(it2)) {}
        
        void operator++() { if (it1 != it1end) it1++; else it2++; }
        
        inline bool operator==(const iterator& other) const {
            return it2 == other.it2;
        }
        
        inline bool operator!=(const iterator& other) const {
            return !operator==(other);
        }
        
        auto operator*() const {
            if (it1 != it1end) return *it1; else return *it2;
        }
    };
    
    iterator begin() { return iterator(c1.begin(), c1.end(), c2.begin()); }
    iterator end() { return iterator(c1.end(), c1.end(), c2.end()); }
};

template <typename C>
inline Chain<C> makeChain(C c1, C c2) { return Chain<C>(move(c1), move(c2)); }

template<typename C, typename F>
class Filter {
    typedef typename C::value_type T;
    typedef typename C::iterator Citer;
    
    C cont;
    F foo;
public:
    typedef T value_type;
    Filter(C cont, F foo) : cont(cont), foo(foo) {
        
    }
    
    class iterator {
        Citer it;
        Citer itend;
        F foo;
        
    public:
        typedef T value_type;
        iterator(Citer itp, Citer itendp, F foo) : it(move(itp)), itend(move(itendp)), foo(foo) {
            while (true) {
                bool p1 = it == itend;
                if (p1) break;
                bool p2 = foo(*it) == true;
                if (p2) break;
                ++it;
            }
        }
        
        void operator++() { 
            ++it;
            while (it != itend && foo(*it) == false) {
                ++it;
            }
        }
        
        T operator*() {
            return *it;
        }
        
        bool operator==(const iterator& other) const { return it == other.it; }
        bool operator!=(const iterator& other) const { return it != other.it; }
    };
    
    iterator begin() { return iterator(cont.begin(), cont.end(), foo); }
    iterator end() { return iterator(cont.end(), cont.end(), foo); }
};

template<typename C, typename F>
Filter<C,F> makeFilter(C cont, F foo) { return Filter<C,F>(move(cont), foo); }

template<typename C, typename F, typename R>
class Map {
    typedef typename C::value_type T;
    typedef typename C::iterator Titer;
    //typedef typename F::result_type R;
    
    
    C cont;
    F foo;
public:
    typedef R value_type;
    Map(C cont, F foo) : cont(move(cont)), foo(foo) {}
    
    class iterator {
        Titer it;
        F foo;
        shared_ptr<R> cache;
        
    public:
        iterator(Titer it, F foo) : it(move(it)), foo(foo), cache(NULL) {}
        
        void operator++() { ++it; cache.reset(); }
        
        R operator*() {
            if (!cache) {
                cache.reset(new R(foo(*it)));
            }
            return *cache;
        }
        
        bool operator==(const iterator& other) const { return it == other.it; }
        bool operator!=(const iterator& other) const { return it != other.it; }
    };
    
    iterator begin() { return iterator(cont.begin(), foo); }
    iterator end() { return iterator(cont.end(), foo); }
};

template<typename C, typename F>
auto makeMap(C cont, F foo) { return Map<C,F,decltype(foo(*cont.begin()))>(move(cont), foo); }

template<typename T>
class Range 
{
    T beginval;
    T endval;
public:
    typedef T value_type;
    
    Range(T beginval, T endval) : beginval(move(beginval)), endval(move(endval)) {}
    
    class iterator
    {
        T value;
    public:
        iterator(T value) : value(move(value)) {}
        void operator++() { ++value; }
        bool operator==(const iterator& other) const { return value == other.value; }
        bool operator!=(const iterator& other) const { return value != other.value; }
        T operator*() const { return value; }
    };
    
    iterator begin() const { return iterator(beginval); }
    iterator end() const { return iterator(endval); }
};

template<typename T>
Range<T> makeRange(T beginval, T endval) { return Range<T>(move(beginval), move(endval)); }

//Turn iterable reference into copyable iterable
template<typename C>
class ReferenceIterable
{
    typedef typename C::iterator Citer;
    typedef typename C::value_type T;
    Citer begit;
    Citer endit;
public:
    typedef typename C::value_type value_type;
    
    ReferenceIterable(C& c) : begit(c.begin()), endit(c.end()) {}
    ReferenceIterable(C&& c) : begit(c.begin()), endit(c.end()) {}
    
    class iterator
    {
        Citer it;
    public:
        iterator(Citer it) : it(it) {}
        void operator++() { ++it; }
        bool operator==(const iterator& other) const { return it == other.it; }
        bool operator!=(const iterator& other) const { return it != other.it; }
        T operator*() { return *it; }
    };
    
    iterator begin() const { return iterator(begit); }
    iterator end() const { return iterator(endit); }
};

template<typename C>
ReferenceIterable<C> makeReferenceIterable(C& cont) { return ReferenceIterable<C>(cont); }

template<typename C>
ReferenceIterable<C> makeReferenceIterable(C&& cont) { return ReferenceIterable<C>(move(cont)); }

template <typename T>
auto toVector(T cont) -> vector<typename T::value_type> { 
    typedef typename T::value_type R;
    vector<R> retval;
    for (auto val : cont) {
        retval.emplace_back(move(val));
    }
    return retval;
}

template <typename T>
void testIter(T cont) { 
    typedef typename T::value_type R;
    for (auto val : cont) {
        cout << "ITER" << endl;
    }
}


template<typename T>
struct memfun_type
{
    using type = void;
};

template<typename Ret, typename Class, typename... Args>
struct memfun_type<Ret(Class::*)(Args...) const>
{
    using type = std::function<Ret(Args...)>;
};

template<typename F>
typename memfun_type<decltype(&F::operator())>::type
FFL(F const &func)
{ // Function from lambda !
    return func;
}

//template <typename T>
//vector<typename T::value_type> toVector(T&& cont) { return vector<typename T::value_type>(cont.begin(), cont.end()); }

#endif
#include "RandomJoins.hpp"
#include <iostream>

using namespace std;



int main()
{
    int n, numRel;
    std::cin>>numRel>>n;
    GeneratePermutation genPermutation(numRel);
    //std::cin>>n;
    genPermutation.generate(n);
    
    GenerateRandomTree genRandTree(numRel);
    genRandTree.generateTree(n);
    return 0;
}

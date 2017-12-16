#include "RandomJoins.hpp"
#include <vector>
#include <iostream>
#include <string>

void GeneratePermutation::generate(int rank){
    std::vector<int> res;
    res.resize(relationSize);
    for(int i = 0; i < relationSize; ++i)
        res[i] = i + 1;

    for(int i = relationSize - 1; i > -1; --i){
        //res[i] = rank % (i + 1);
        std::swap(res[i], res[rank % (i + 1)]);
        rank = rank / (i + 1);
    }
    for(int i : res)
        std::cout<<i<<" ";
    std::cout<<std::endl;
}

void GenerateRandomTree::generateTree(int rank){
    int cur_x = 0, cur_y = 0;
    std::string res = "";
    while(cur_x != 2*relationSize){
        if(dp[cur_x + 1][cur_y + 1] > rank){
            res += "(";
            cur_x++;
            cur_y++;
        } else{
            rank -= dp[cur_x + 1][cur_y + 1];
            res += ")";
            cur_x++;
            cur_y--;
        }
    }
    std::cout<<res<<std::endl;
}
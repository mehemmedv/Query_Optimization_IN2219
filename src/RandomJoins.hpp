#ifndef H_RandomJoins
#define H_RandomJoins

#include <vector>
#include <iostream>

class GeneratePermutation{
private:
    int relationSize;
public:
    GeneratePermutation(int relationSize) : relationSize(relationSize){}
    void generate(int rank);
};

class GenerateRandomTree{
private:
    int relationSize;
    int **dp;
public:
    GenerateRandomTree(int relationSize) : relationSize(relationSize){
        dp = new int*[relationSize * 2 + 2];

        for(int i = 0; i <= relationSize * 2 + 1; ++i)
            dp[i] = new int[relationSize + 2];
        for(int i = 0; i <= relationSize * 2 + 1; ++i)
        for(int j = 0; j <= relationSize + 1; ++j)
            dp[i][j] = 0;
        
        int last_y = relationSize;
        dp[2*relationSize][0] = 1;
        for(int i = 2 * relationSize; i >= 0; i -= 2){
            int cur_x = i;
            int cur_y = 0;
            for(int j = 0; j <= last_y; ++j){
               // std::cout<<cur_x<<" "<<cur_y<<std::endl;
                if(dp[cur_x][cur_y] != 0){
                    cur_x--;
                    cur_y++;
                    continue;
                }
                dp[cur_x][cur_y] = dp[cur_x+1][cur_y+1];
                if(cur_y > 0)
                    dp[cur_x][cur_y] += dp[cur_x+1][cur_y-1];
                cur_x--;
                cur_y++;
            }

            last_y--;
        }
        
    }

    void generateTree(int rank);
};

#endif
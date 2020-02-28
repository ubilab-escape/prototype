// Copyright 2018
// Author: Lukas König

#ifndef COUNTER_H_
#define COUNTER_H_

#include <vector>
#include <iostream>

class Counter {
 public:
    std::vector<std::vector<int>> _one = {{0,1,1,1,1,0,0},
                                          {1,1,1,1,1,0,0},
                                          {0,0,1,1,1,0,0},
                                          {0,0,1,1,1,0,0},
                                          {0,0,1,1,1,0,0},
                                          {0,0,1,1,1,0,0},
                                          {0,0,1,1,1,0,0},
                                          {1,1,1,1,1,1,1}};
    
    std::vector<std::vector<int>> _two = {{0,1,1,1,1,1,1,1,1,0},
                                          {1,1,1,1,1,1,1,1,1,1},
                                          {0,0,0,0,0,0,0,1,1,1},
                                          {0,0,0,0,0,1,1,1,1,1},
                                          {0,1,1,1,1,1,1,1,1,0},
                                          {1,1,1,1,1,0,0,0,0,0},
                                          {1,1,1,1,0,0,0,0,0,0},
                                          {1,1,1,1,1,1,1,1,1,0}};
    

    std::vector<std::vector<int>> _three = {{}};
    std::vector<std::vector<int>> _turn = {{}};
    
    int _current_value;

    Counter(int start_value=0);

    void count();
    void draw(int posY, int posX);
};
#endif  // COUNTER_H_

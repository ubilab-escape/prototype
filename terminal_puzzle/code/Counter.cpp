// Copyright 2019
// Author: Lukas König

#include "./Counter.h"
#include <ncurses.h>


// class variables
int _current_value;

std::vector<std::vector<int>> _one;
std::vector<std::vector<int>> _two;
std::vector<std::vector<int>> _three;
std::vector<std::vector<int>> _turn;


Counter::Counter(int start_value) {
}

void Counter::draw(int posY, int posX) {
}

void Counter::count() {
    _current_value = (_current_value + 1) % 3;
}

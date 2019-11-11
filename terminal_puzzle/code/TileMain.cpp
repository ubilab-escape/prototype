// Copyright 2019

#include <ncurses.h>
#include <unistd.h>
#include <iostream>
#include <vector>
#include "./Tile.h"
#include "./TileStructure.h"

int main(int argc, char** argv) {
  std::vector<std::vector<bool>> pixelsA = {{1, 0, 1}, {1, 0, 1}, {1, 1, 1}};
  std::vector<std::vector<bool>> pixelsB = {{1, 1, 1}, {1, 0, 1}, {1, 0, 1}};
  std::vector<std::vector<bool>> pixelsC = {{1, 1, 1}, {0, 0, 1}, {1, 1, 1}};
  std::vector<std::vector<bool>> pixelsD = {{1, 1, 1}, {1, 0, 0}, {1, 1, 1}};
  std::vector<std::vector<bool>> picture =
               {{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                {0, 1, 1, 1, 1, 0, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0},
                {0, 1, 1, 1, 1, 0, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0},
                {0, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 1, 1, 0, 0, 0, 0},
                {0, 1, 1, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 1, 1, 0, 0, 0, 0},
                {0, 1, 1, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0},
                {0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 1, 1, 0, 0, 0, 0},
                {0, 1, 1, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 1, 1, 0, 0, 0, 0},
                {0, 1, 1, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 1, 1, 0, 0, 0, 0},
                {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                {0, 1, 1, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0},
                {0, 1, 1, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0},
                {0, 1, 1, 0, 0, 0, 1, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                {0, 1, 1, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0},
                {0, 1, 1, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0},
                {0, 0, 0, 1, 1, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0},
                {0, 1, 1, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0},
                {0, 1, 1, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0},
                {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}};

  TileStructure tstruct = TileStructure(picture);
  initscr();
  cbreak();
  noecho();
  curs_set(false);
  nodelay(stdscr, true);
  keypad(stdscr, true);
  refresh();
  int key;
  int i = 0;
  tstruct.draw(5, 5);
  while (true) {
    key = getch();
    if (key == 27) break;
    if (key == 'a') {
        tstruct.turnTilesRight(0);
        clear();
    }
    if (key == 's') {
        tstruct.turnTilesRight(1);
        clear();
    }
    if (key == 'd') {
        tstruct.turnTilesRight(2);
        clear();
    }
    if (key == 'f') {
        tstruct.turnTilesRight(3);
        clear();
    }
    if (key == 'q') {
        tstruct.turnTilesAround(0);
        clear();
    }
    if (key == 'w') {
        tstruct.turnTilesAround(1);
        clear();
    }
    if (key == 'e') {
        tstruct.turnTilesAround(2);
        clear();
    }
    if (key == 'r') {
        tstruct.turnTilesAround(3);
        clear();
    }

    tstruct.draw(5, 5);
    refresh();
    i = (i + 1) % 4;
  }
  endwin();
}


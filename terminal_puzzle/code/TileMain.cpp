// Copyright 2019

#include <ncurses.h>
#include <unistd.h>
#include <iostream>
#include <vector>
#include "./Tile.h"
#include "./TileStructure.h"

int main(int argc, char** argv) {
  std::vector<std::vector<int>> pixelsA = {{1, 0, 1}, {1, 0, 1}, {1, 1, 1}};
  std::vector<std::vector<int>> pixelsB = {{1, 1, 1}, {1, 0, 1}, {1, 0, 1}};
  std::vector<std::vector<int>> pixelsC = {{1, 1, 1}, {0, 0, 1}, {1, 1, 1}};
  std::vector<std::vector<int>> pixelsD = {{1, 1, 1}, {1, 0, 0}, {1, 1, 1}};
  std::vector<std::vector<int>> picture =
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
  TileStructure originalstruct = tstruct;
  initscr();
  cbreak();
  noecho();
  curs_set(false);
  nodelay(stdscr, true);
  keypad(stdscr, true);
  clear();
  tstruct.pixelsFromFile("picture.txt");
  refresh();
  int key;
  clear();
  tstruct.draw(5, 5);
  refresh();
  sleep(2);
  tstruct.shuffle();
  clear();
  tstruct.draw(5, 5);
  refresh();
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
    if (tstruct == originalstruct) {
        sleep(1);
        clear();
        mvprintw(5, 5, "YOU SOLVED IT");
        refresh();
        sleep(5);
        break;
    }
  }
  endwin();
}


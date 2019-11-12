// Copyright 2019

#include <ncurses.h>
#include <unistd.h>
#include <iostream>
#include <vector>
#include "./Tile.h"
#include "./TileStructure.h"

int main(int argc, char** argv) {
  // init TileStructure
  TileStructure tstruct = TileStructure("adidas.tex");

  // ncurses init stuff
  initscr();
  cbreak();
  noecho();
  curs_set(false);
  nodelay(stdscr, true);
  keypad(stdscr, true);
  int key;

  // show TileStruct before shuffeled
  clear();
  tstruct.draw(5, 5);
  refresh();
  sleep(2);

  // shuffle and show shuffeled TileStruct
  tstruct.shuffle();
  clear();
  tstruct.draw(5, 5);
  refresh();

  // start loop
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
    if (tstruct.solved()) {
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


// Copyright 2019

#include <ncurses.h>
#include <unistd.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <cstring>
#include <fstream>
#include <sys/stat.h>
#include <sys/mount.h>
#include <sys/syslog.h>
#include <vector>
#include <chrono>
#include <thread>
//#include "ctime"
#include "./Tile.h"
#include "./TileStructure.h"

using namespace std;

void custom_print(char* s) {
  //clear();
  mvprintw(100, 5, s);
  refresh();
  //sleep(2);
}

inline bool exist_test (const std::string& name) {
  struct stat buffer;
  return (stat (name.c_str(), &buffer) == 0);
}

int main(int argc, char** argv) {
  //init window size
  system("/usr/bin/xvkbd -text \"\\C\\S_\"");
  system("/usr/bin/xvkbd -text \"\\C\\S_\"");
  system("/usr/bin/xvkbd -text \"\\C\\S_\"");
  system("/usr/bin/xvkbd -text \"\\C\\S_\"");
  system("/usr/bin/xvkbd -text \"\\C\\S_\"");
  system("/usr/bin/xvkbd -text \"\\C\\S_\"");
  sleep(1);
  system("/usr/bin/xvkbd -text \"\\[F11]\"");

  // init TileStructure
  TileStructure tstruct = TileStructure("adidas_35.txt");

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

  FILE *fp;
  int status;
  char old_color [6];
  char new_color [6];
  char color [6];

  fp = popen("sudo blkid /dev/sda | grep -o LABEL.* | cut -d\\\" -f2", "r");


  while (fgets(color, 6, fp) != NULL) {
    std::copy(std::begin(color), std::end(color), std::begin(new_color));
    std::copy(std::begin(color), std::end(color), std::begin(old_color));
    //custom_print(new_color);
    //custom_print(old_color);
  }

  // start loop
  while (true) {
    //key = getch();
    FILE *fp;
    int status;

    // if (key == 27) break;
    // if (test_var == 1) {
    //     tstruct.turnTilesRight(0);
    //     clear();
    // }
    // if (/*key == 's'*/exist_test("/media/floppy/gruen.txt")) {
    //     tstruct.turnTilesRight(1);
    //     clear();
    // }
    // if (/*key == 'd'*/exist_test("/media/floppy/blau.txt")) {
    //     tstruct.turnTilesRight(2);
    //     clear();
    // }
    // if (/*key == 'f'*/exist_test("/media/floppy/gelb.txt")) {
    //     tstruct.turnTilesRight(3);
    //     clear();
    // }
    // if (key == 'q') {
    //     tstruct.turnTilesAround(0);
    //     clear();
    // }
    // if (key == 'w') {
    //     tstruct.turnTilesAround(1);
    //     clear();
    // }
    // if (key == 'e') {
    //     tstruct.turnTilesAround(2);
    //     clear();
    // }
    // if (key == 'r') {
    //     tstruct.turnTilesAround(3);
    //     clear();
    // }
    // tstruct.draw(5, 5);

    int counter = 0;

    while (counter < 30) {
      key = getch();
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

      if (counter == 29) {
        //time_t t1 = time(NULL);
        fp = popen("sudo blkid /dev/sda | grep -o LABEL.* | cut -d\\\" -f2", "r");

        std::copy(std::begin(new_color), std::end(new_color), std::begin(old_color));
        if (fgets(color, 6, fp) != NULL) {

          std::copy(std::begin(color), std::end(color), std::begin(new_color));
        }
        else {
          new_color[0] = 'X';
          old_color[0] = 'X';
        }
        //custom_print(new_color);

        //if (strcmp(new_color, old_color) != 0) {
          //custom_print("test");
        if (strcmp(new_color, "Rottt") == 0) {
          tstruct.turnTilesRight(0);
          clear();
        }
        else if (strcmp(new_color, "Gruen") == 0) {
            tstruct.turnTilesRight(1);
            clear();
        }
        else if (strcmp(new_color, "Blauu") == 0) {
            tstruct.turnTilesRight(2);
            clear();
        }
        else if (strcmp(new_color, "Gelbb") == 0) {
            tstruct.turnTilesRight(3);
            clear();
        }
        //}

        status = pclose(fp);
        //time_t t2 = time(NULL);
        //endwin();

        //cout << t2-t1;
        sleep(3);

      }
      tstruct.draw(5, 5);
      refresh();
      this_thread::sleep_for(chrono::microseconds(100));
      counter++;

    }
    //sleep(3);
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

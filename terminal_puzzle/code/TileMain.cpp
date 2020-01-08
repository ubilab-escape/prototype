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
  system("/usr/bin/xvkbd -text \"\\C\\S_\"");
  system("/usr/bin/xvkbd -text \"\\C\\S_\"");
  sleep(1);

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

  FILE *fp_a;
  FILE *fp_b;
  int status;
  char old_color_a [6];
  char new_color_a [6];
  char color_a [6];
  char old_color_b [6];
  char new_color_b [6];
  char color_b [6];

  while (true){
    fp_a = popen("sudo blkid /dev/sda | grep -o LABEL.* | cut -d\\\" -f2", "r");
    fgets(color_a, 6, fp_a);
    fp_b = popen("sudo blkid /dev/sdb | grep -o LABEL.* | cut -d\\\" -f2", "r");
    fgets(color_b, 6, fp_b);

    if ((strcmp(color_a, "Rottt") == 0) || (strcmp(color_a, "Blauu") == 0) || (strcmp(color_a, "Gelbb") == 0) || (strcmp(color_a, "Gruen") == 0)){
      tstruct.shuffle();
      clear();
      tstruct.draw(5, 5);
      refresh();
      break;
    }
    else if ((strcmp(color_b, "Rottt") == 0) || (strcmp(color_b, "Blauu") == 0) || (strcmp(color_b, "Gelbb") == 0) || (strcmp(color_b, "Gruen") == 0)){
      tstruct.shuffle();
      clear();
      tstruct.draw(5, 5);
      refresh();
      break;
    }
  }

 /* TODO DELETE
  * while (fgets(color_a, 6, fp_a) != NULL) {
    std::copy(std::begin(color_a), std::end(color_a), std::begin(new_color_a));
    std::copy(std::begin(color_a), std::end(color_a), std::begin(old_color_a));
    //custom_print(new_color);
    //custom_print(old_color);
  }*/

  // start loop
  while (true) {
    //key = getch();
    //FILE *fp_a;
    //int status;

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
      
      if (counter == 15) {
        fp_b = popen("sudo blkid /dev/sdb | grep -o LABEL.* | cut -d\\\" -f2", "r");
      
        std::copy(std::begin(new_color_b), std::end(new_color_b), std::begin(old_color_b));
      
        if (fgets(color_b, 6, fp_b) != NULL) {
          std::copy(std::begin(color_b), std::end(color_b), std::begin(new_color_b));
        }
        else {
          new_color_b[0] = 'X';
          old_color_b[0] = 'X';
        }
        
        if (strcmp(new_color_b, "Rottt") == 0) {
          tstruct.turnTilesAround(0);
          clear();
        }
        else if (strcmp(new_color_b, "Gruen") == 0) {
          tstruct.turnTilesAround(1);
          clear();
        }
        else if (strcmp(new_color_b, "Blauu") == 0) {
          tstruct.turnTilesAround(2);
         clear();
        }
        else if (strcmp(new_color_b, "Gelbb") == 0) {
          tstruct.turnTilesAround(3);
          clear();
        }
        status = pclose(fp_b);
        
        //this_thread::sleep_for(chrono::milliseconds(800));
      }
      
      if (counter == 29) {
        fp_a = popen("sudo blkid /dev/sda | grep -o LABEL.* | cut -d\\\" -f2", "r");

        std::copy(std::begin(new_color_a), std::end(new_color_a), std::begin(old_color_a));
        if (fgets(color_a, 6, fp_a) != NULL) {

          std::copy(std::begin(color_a), std::end(color_a), std::begin(new_color_a));
        }
        else {
          new_color_a[0] = 'X';
          old_color_a[0] = 'X';
        }
        //custom_print(new_color);

        //if (strcmp(new_color, old_color) != 0) {
          //custom_print("test");
        if (strcmp(new_color_a, "Rottt") == 0) {
          tstruct.turnTilesRight(0);
          clear();
        }
        else if (strcmp(new_color_a, "Gruen") == 0) {
            tstruct.turnTilesRight(1);
            clear();
        }
        else if (strcmp(new_color_a, "Blauu") == 0) {
            tstruct.turnTilesRight(2);
            clear();
        }
        else if (strcmp(new_color_a, "Gelbb") == 0) {
            tstruct.turnTilesRight(3);
            clear();
        }
        //}
        status = pclose(fp_a);

        this_thread::sleep_for(chrono::milliseconds(1600));
      }
      tstruct.draw(5, 5);
      refresh();
      this_thread::sleep_for(chrono::microseconds(100));
      counter++;
    }
    
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

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
#include "./Tile.h"
#include "./TileStructure.h"

using namespace std;

inline bool exist_test (const std::string& name) {
  struct stat buffer;
  return (stat (name.c_str(), &buffer) == 0);
}

int main(int argc, char** argv) {
  // init file
  ifstream infile;
  char data [1];
  // init mount points
  const char* floppy_1 = "/media/floppy";

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
  char sdd_msg[23];

  char new_sdd_msg [23];
  char old_sdd_msg [23];

  fp = popen("dmesg | grep \"sdd:\"", "r");

  std::copy(std::begin(new_sdd_msg), std::end(new_sdd_msg), std::begin(old_sdd_msg));

  while (fgets(sdd_msg, 23, fp) != NULL)
      //printf("%s\n", sdd_msg);
      std::copy(std::begin(sdd_msg), std::end(sdd_msg), std::begin(new_sdd_msg));
      std::copy(std::begin(new_sdd_msg), std::end(new_sdd_msg), std::begin(old_sdd_msg));

  // start loop
  while (true) {
    //infile.open("/media/floppy/test.txt");
    //key = getch();
    //infile >> data;
    //key = data[0];
    FILE *fp;
    int status;
    char sdd_msg[23];


    fp = popen("dmesg | grep \"sdd:\"", "r");

    std::copy(std::begin(new_sdd_msg), std::end(new_sdd_msg), std::begin(old_sdd_msg));

    while (fgets(sdd_msg, 23, fp) != NULL)
        //printf("%s\n", sdd_msg);
        std::copy(std::begin(sdd_msg), std::end(sdd_msg), std::begin(new_sdd_msg));

    //printf(new_sdd_msg);
    //printf(old_sdd_msg);
    if (strcmp(new_sdd_msg, old_sdd_msg) != 0) {
      //mount("/dev/sdd", floppy_1, "fat", 0, )
      popen("mount -o umask=222 /dev/sdd /media/floppy", "r");
    }

    status = pclose(fp);


    //mount("/dev/sdd", floppy_1, "-o", 0, "");

    if (key == 27) break;
    if (/*key == 'a'*/exist_test("/media/floppy/rot.txt")) {
        tstruct.turnTilesRight(0);
        clear();
    }
    if (/*key == 's'*/exist_test("/media/floppy/gruen.txt")) {
        tstruct.turnTilesRight(1);
        clear();
    }
    if (/*key == 'd'*/exist_test("/media/floppy/blau.txt")) {
        tstruct.turnTilesRight(2);
        clear();
    }
    if (/*key == 'f'*/exist_test("/media/floppy/gelb.txt")) {
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
    //infile.close();
    umount2(floppy_1, MNT_FORCE);

    data[0] = 0;
    //sleep(1);
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

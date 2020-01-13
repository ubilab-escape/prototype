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
#include "./Tile.h"
#include "./TileStructure.h"

using namespace std;

void custom_print(char* s) {
  clear();
  mvprintw(0, 0, s);
  refresh();
  sleep(2);
}

inline bool exist_test (const std::string& name) {
  struct stat buffer;
  return (stat (name.c_str(), &buffer) == 0);
}

int main(int argc, char** argv) {
  // init TileStructure
  TileStructure tstruct = TileStructure("STASIS.txt");

  // ncurses init stuff
  initscr();
  cbreak();
  noecho();
  curs_set(false);
  nodelay(stdscr, true);
  keypad(stdscr, true);

  // show TileStruct before shuffeled
  clear();
  tstruct.draw(5, 5);
  refresh();

  // puzzle activation
  bool puzzle_active = false;
  while (!puzzle_active){

    FILE *fp_a = popen("sudo fdisk -l | grep \"\/dev\/sd\\|Disk identifier: 0x00\"", "r");
    char buffer [120];

    while (fgets(buffer, 120, fp_a) != NULL){
      pclose(fp_a);
      if(strstr(buffer, "\/dev\/sd") != NULL) {
        tstruct.shuffle();
        clear();
        tstruct.draw(5, 5);
        refresh();
        puzzle_active = true;
        break;
      }
    }
  }

  // start loop
  while (true) {

    auto t1 = std::chrono::high_resolution_clock::now();

    FILE *fp_a = popen("sudo fdisk -l | grep \"\/dev\/sd\\|Disk identifier: 0x00\"", "r");
    char buffer [120];
    int sda_id = -1;
    int sdb_id = -1;
    bool sda_found = false;
    bool sdb_found = false;
    while (fgets(buffer, 120, fp_a) != NULL){
      if(strstr(buffer, "\/dev\/sda") != NULL) {
        sda_found = true;
      }
      else if(sda_found) {
        sda_id = buffer[26] - '0';
        sda_found = false;
      }
      else if(strstr(buffer, "\/dev\/sdb") != NULL) {
        sdb_found = true;
      }
      else if(sdb_found) {
        sdb_id = buffer[26] - '0';
        sdb_found = false;
      }
    }

    pclose(fp_a);
    if(sda_id != -1) {
      tstruct.turnTilesRight(sda_id);
      clear();
    }

    if(sdb_id != -1) {
      tstruct.turnTilesAround(sdb_id);
      clear();
    }

    auto t2 = std::chrono::high_resolution_clock::now();
       
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>( t2 - t1 ).count();
    auto sleep_duration = 1000000-duration;
    if (sleep_duration > 0) {
      usleep(sleep_duration);
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

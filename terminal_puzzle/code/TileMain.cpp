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
  system("/usr/bin/xvkbd -text \"\\C-\"");
  system("/usr/bin/xvkbd -text \"\\C-\"");  
  system("/usr/bin/xvkbd -text \"\\C-\"");  
  system("/usr/bin/xvkbd -text \"\\C-\"");  
  system("/usr/bin/xvkbd -text \"\\C-\"");  
  system("/usr/bin/xvkbd -text \"\\C-\"");  
  system("/usr/bin/xvkbd -text \"\\C-\"");  
  system("/usr/bin/xvkbd -text \"\\C-\"");  
  system("/usr/bin/xvkbd -text \"\\C-\"");  



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
    FILE *fp_a = popen("sudo ./check_floppy.sh 2>&1", "r");
    char buffer [120];
    bool sd_found = false;
    while (fgets(buffer, 120, fp_a) != NULL){
      if(strstr(buffer, "/dev/sd") != NULL) {
        sd_found = true;
      }
      else if (sd_found) { 
        if(buffer[26] != '0') {
          tstruct.shuffle();
          clear();
          tstruct.draw(5, 5);
          refresh();
          puzzle_active = true;
          break;
        }
      }
    }
    pclose(fp_a);
  }

  // start loop
  while (true) {

    auto t1 = std::chrono::high_resolution_clock::now();
    FILE *fp_a = popen("sudo ./check_floppy.sh 2>&1", "r");
    char buffer [120];
    int sda_id = -1;
    int sdb_id = -1;
    bool sda_found = false;
    bool sdb_found = false;
    while (fgets(buffer, 120, fp_a) != NULL){
      if(strstr(buffer, "Killed")||strstr(buffer, "SIGKILL")) {
		//custom_print("restart floppy");
	      FILE *reset_usb_process = popen("sudo timeout -s SIGKILL 3 uhubctl -a off -p 2", "r");
	      char second_buffer [120];
	      //while(fgets(second_buffer, 120, reset_usb_process) != NULL);
      	      pclose(reset_usb_process);
		//custom_print("restart finished");
		break;
      }
      if(strstr(buffer, "/dev/sda") != NULL) {
        sda_found = true;
      }
      else if(sda_found) {
        sda_id = buffer[26] - '0';
        sda_found = false;
      }
      else if(strstr(buffer, "/dev/sdb") != NULL) {
        sdb_found = true;
      }
      else if(sdb_found) {
        sdb_id = buffer[26] - '0';
        sdb_found = false;
      }
    }
    
    
    pclose(fp_a);

    if(sda_id != 0) {
      tstruct.turnTilesRight(sda_id % 4);
      clear();
    }

    if(sdb_id != 0) {
      tstruct.turnTilesAround(sdb_id % 4);
      clear();
    }

    auto t2 = std::chrono::high_resolution_clock::now();
       
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>( t2 - t1 ).count();
    auto sleep_duration = 2000000-duration;
    if (sleep_duration > 0) {
      usleep(sleep_duration);
    }

    tstruct.draw(5, 5);
    refresh();    
    
    if (tstruct.solved()) {
      tstruct.colorWhite();
      tstruct.draw(5, 5);
      refresh(); 
      sleep(5);
      break;
    }
  }
  //endwin();
  while(1);
}

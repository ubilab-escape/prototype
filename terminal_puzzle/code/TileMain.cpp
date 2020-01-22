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
#include "MQTTClient.h"

#define ADDRESS     "10.0.0.2:1883"
#define CLIENTID    "raspberry-terminal"
#define TOPIC       "6/puzzle/terminal"
#define QOS         1
#define TIMEOUT     10000L

enum states {inactive, active, solved, failed};

using namespace std;

void custom_print(char* s) {
  clear();
  mvprintw(0, 0, s);
  refresh();
  sleep(2);
}

void custom_print(int i) {
  std::string s = std::to_string(i);
  char const *c = s.c_str();
  clear();
  mvprintw(0, 0, c);
  refresh();
  sleep(2);
}


inline bool exist_test (const std::string& name) {
  struct stat buffer;
  return (stat (name.c_str(), &buffer) == 0);
}

void zoom_out(int level) {
    for (int i = 0; i < level; i++) {
        system("/usr/bin/xvkbd -text \"\\C-\"");
    }
}

void publish_state(string str, MQTTClient* cl) {
  MQTTClient_message pubmsg = MQTTClient_message_initializer;
  MQTTClient_deliveryToken token;
  str = "{\"method\":\"status\",\"state\":\"" + str + "\",\"data\":\"" + str + "\"}";
  char* msg = strdup(str.c_str());
  pubmsg.payload = (void*)msg; 
  pubmsg.payloadlen = strlen(msg);
  pubmsg.qos = QOS;
  pubmsg.retained = 1;
  MQTTClient_publishMessage(*cl, TOPIC, &pubmsg, &token);
  MQTTClient_waitForCompletion(*cl, token, TIMEOUT);
  free(msg);
}

int msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message)
{
    string msg = (char*)message->payload;
    // custom_print((char*)message->payload);
    
    MQTTClient_freeMessage(&message);
    MQTTClient_free(topicName);
    
    return 1;
}

MQTTClient client;
  MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
void connlost(void *context, char *cause)
{
    printf("\nConnection lost\n");
    while(MQTTClient_connect(client, &conn_opts) != MQTTCLIENT_SUCCESS);
}
int main(int argc, char** argv) {

  // init mqtt
  MQTTClient_create(&client, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL);
  conn_opts.keepAliveInterval = 10;
  conn_opts.cleansession = 1;

  MQTTClient_setCallbacks(client, NULL, connlost, msgarrvd, NULL);

  int rc;
  if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS)
  {
      printf("Failed to connect, return code %d\n", rc);
      exit(-1);
  }

  MQTTClient_subscribe(client, TOPIC, QOS);

  

  zoom_out(10);

  // init TileStructure
  TileStructure tstruct = TileStructure("STASIS.txt");

  // ncurses init stuff
  int width, height;
  initscr();
  getmaxyx(stdscr, height, width);
  int POSX = width/2 - tstruct.size();
  int POSY = height/2 - tstruct.size()/2;
  cbreak();
  noecho();
  curs_set(false);
  nodelay(stdscr, true);
  keypad(stdscr, true);


  //custom_print(tstruct.size());
  // show TileStruct before shuffeled
  clear();
  tstruct.draw(POSX, POSY);
  refresh();

  states current_state = inactive;
  publish_state("inactive", &client);

  // state machine
  while(1) {
    // waiting for activation
    while (current_state == inactive){    
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
            tstruct.draw(POSX, POSY);
            refresh();
            current_state = active;
            publish_state("active", &client);
          }
        }
      }
      pclose(fp_a);
    }

    // riddle is active
    while (current_state == active) {
      auto t1 = std::chrono::high_resolution_clock::now();
      FILE *fp_a = popen("sudo ./check_floppy.sh 2>&1", "r");
      char buffer [120];
      int sda_id = -1;
      int sdb_id = -1;
      bool sda_found = false;
      bool sdb_found = false;
      while (fgets(buffer, 120, fp_a) != NULL){
        if(strstr(buffer, "Killed")||strstr(buffer, "SIGKILL")) {
          FILE *reset_usb_process = popen("sudo timeout -s SIGKILL 3 uhubctl -a off -p 2", "r");
          pclose(reset_usb_process);
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
        tstruct.draw(POSX, POSY);
        refresh();    
      }

      if(sdb_id != 0) {
        tstruct.turnTilesAround(sdb_id % 4);
        clear();
        tstruct.draw(POSX, POSY);
        refresh();    
      }

      auto t2 = std::chrono::high_resolution_clock::now();
        
      auto duration = std::chrono::duration_cast<std::chrono::microseconds>( t2 - t1 ).count();
      auto sleep_duration = 2000000-duration;
      if (sleep_duration > 0) {
        usleep(sleep_duration);
      }
      
      if (tstruct.solved()) {
        tstruct.colorWhite();
        tstruct.draw(POSX, POSY);
        refresh(); 
        current_state = solved;
        publish_state("solved", &client);
      }
    }
    
    while(current_state == solved || current_state == failed);
  }
  //MQTTClient_disconnect(client, 10000);
  //MQTTClient_destroy(&client); 
}

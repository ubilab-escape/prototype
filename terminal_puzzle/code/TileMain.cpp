// Copyright 2019
// Author: Florian Diederichs
// Author: Lukas König
// Author: Niklas Steinwachs

extern "C"
{
#include "MQTTClient.h"
#include "MQTTClientPersistence.h"
}
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
#include <algorithm>
#include "./Tile.h"
#include "./TileStructure.h"

// Constants for MQTT
#define ADDRESS "10.0.0.2:1883"
#define CLIENTID "raspberry-terminal"
#define TOPIC_TERMINAL "6/puzzle/terminal"
#define TOPIC_SCALE "6/puzzle/scale"
#define QOS 1
#define TIMEOUT 30000L

using namespace std;

// Possible states of the riddle
enum states
{
    inactive,
    active,
    solved,
    failed
};

// Global state of the riddle
volatile states current_state = inactive;

// Global MQTT triggers and client
volatile bool trigger_on = false;
volatile bool trigger_skipped = false;
volatile bool trigger_off = false;
volatile MQTTClient_deliveryToken deliveredtoken;
MQTTClient client;
MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;

// Go to max zoom level in the terminal
void zoom_out(int level)
{
    for (int i = 0; i < level; i++)
    {
        system("/usr/bin/xvkbd -text \"\\C-\"");
    }
}

// Publish the current state of the riddle
void publish_state(string str, MQTTClient *cl)
{
    MQTTClient_message pubmsg = MQTTClient_message_initializer;
    MQTTClient_deliveryToken token;
    str = "{\"method\":\"status\",\"state\":\"" + str + "\",\"data\":\"" + str + "\"}";
    char *msg = strdup(str.c_str());
    pubmsg.payload = (void *)msg;
    pubmsg.payloadlen = strlen(msg);
    pubmsg.qos = QOS;
    pubmsg.retained = 1;
    MQTTClient_publishMessage(*cl, TOPIC_TERMINAL, &pubmsg, &token);
    MQTTClient_waitForCompletion(*cl, token, TIMEOUT);
    free(msg);
}

void delivered(void *context, MQTTClient_deliveryToken dt)
{
    deliveredtoken = dt;
}

// Callback for arrived MQTT messages
int msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message)
{
    string msg = (char *)message->payload;
    string str_topicName = topicName;

    // Set triggers to change the riddle state
    if (str_topicName.find("6/puzzle/terminal") != string::npos)
    {
        if (msg.find("trigger") != string::npos)
        {
            if (msg.find("skipped") != string::npos)
            {
                trigger_skipped = true;
            }
            else if (msg.find("on") != string::npos)
            {
                trigger_on = true;
            }
            else if (msg.find("off") != string::npos)
            {
                trigger_off = true;
            }
        }
    }
    MQTTClient_freeMessage(&message);
    MQTTClient_free(topicName);
    return 1;
}

// MQTT reconnect
void connlost(void *context, char *cause)
{
    while (MQTTClient_connect(client, &conn_opts) != MQTTCLIENT_SUCCESS)
    {
        this_thread::sleep_for(chrono::microseconds(500000));
    };
}

int main(int argc, char **argv)
{

    // init MQTT
    MQTTClient_create(&client, ADDRESS, CLIENTID,
                      MQTTCLIENT_PERSISTENCE_DEFAULT, NULL);
    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = 1;

    MQTTClient_setCallbacks(client, NULL, connlost, msgarrvd, NULL);

    MQTTClient_connect(client, &conn_opts);
    MQTTClient_subscribe(client, TOPIC_TERMINAL, QOS);
    MQTTClient_subscribe(client, TOPIC_SCALE, QOS);

    zoom_out(10);

    // init TileStructure
    TileStructure tstruct = TileStructure("STASIS.txt");

    // ncurses init stuff
    int width, height;
    initscr();
    getmaxyx(stdscr, height, width);
    int POSX = width / 2 - tstruct.size();
    int POSY = height / 2 - tstruct.size() / 2;
    cbreak();
    noecho();
    curs_set(false);
    nodelay(stdscr, true);
    keypad(stdscr, true);

    //show TileStruct before shuffeled
    clear();
    tstruct.draw(POSX, POSY);
    refresh();

    current_state = inactive;

    // state machine
    while (1)
    {
        // waiting for activation
        while (current_state == inactive)
        {
            if (trigger_on)
            {
                tstruct = TileStructure("STASIS.txt");
                clear();
                tstruct.draw(POSX, POSY);
                refresh();
                trigger_on = false;
                publish_state("inactive", &client);
            }
            else if (trigger_off || trigger_skipped)
            {
                current_state = solved;
            }
            FILE *fp_a = popen("sudo ./check_floppy.sh 2>&1", "r");
            char buffer[120];
            bool sd_found = false;
            while (fgets(buffer, 120, fp_a) != NULL)
            {
                if (strstr(buffer, "/dev/sd") != NULL)
                {
                    sd_found = true;
                }
                else if (sd_found)
                {
                    if (buffer[26] != '0')
                    {
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
        while (current_state == active)
        {
            if (trigger_skipped || trigger_off)
            {
                current_state = solved;
            }
            if (trigger_on)
            {
                trigger_on = false;
                publish_state("active", &client);
            }
            auto t1 = chrono::high_resolution_clock::now();
            // Check the disk identifers and pipe error output to standard output (for the timeout)
            FILE *fp_a = popen("sudo ./check_floppy.sh 2>&1", "r");
            char buffer[120];
            int sda_id = -1;
            int sdb_id = -1;
            bool sda_found = false;
            bool sdb_found = false;
            while (fgets(buffer, 120, fp_a) != NULL)
            {
                // Restart floppy drive if it got stuck
                if (strstr(buffer, "Killed") || strstr(buffer, "SIGKILL"))
                {
                    FILE *reset_usb_process = popen("sudo timeout -s SIGKILL 3 uhubctl -a off -p 2", "r");
                    pclose(reset_usb_process);
                    break;
                }
                // Check which floppy was inserted in which drive
                if (strstr(buffer, "/dev/sda") != NULL)
                {
                    sda_found = true;
                }
                else if (sda_found)
                {
                    sda_id = buffer[26] - '0';
                    sda_found = false;
                }
                else if (strstr(buffer, "/dev/sdb") != NULL)
                {
                    sdb_found = true;
                }
                else if (sdb_found)
                {
                    sdb_id = buffer[26] - '0';
                    sdb_found = false;
                }
            }
            pclose(fp_a);

            // Rotate if necessary
            if (sda_id > 0 && sda_id < 5)
            {
                tstruct.turnTilesRight(sda_id % 4);
                clear();
                tstruct.draw(POSX, POSY);
                refresh();
                sda_id = 0;
            }
            if (sdb_id > 0 && sdb_id < 5)
            {
                tstruct.turnTilesAround(sdb_id % 4);
                tstruct.draw(POSX, POSY);
                refresh();
                sdb_id = 0;
            }

            // Limit the rotation speed to once per 2 seconds
            auto t2 = chrono::high_resolution_clock::now();
            auto duration = chrono::duration_cast<chrono::microseconds>(t2 - t1).count();
            auto sleep_duration = 2000000 - duration;
            if (sleep_duration > 0)
            {
                this_thread::sleep_for(chrono::microseconds(sleep_duration));
            }

            // Check if the puzzle was solved by the last rotation
            if (tstruct.solved())
            {
                tstruct.colorWhiteInitial();
                tstruct.drawInitial(POSX, POSY);
                refresh();
                current_state = solved;
                publish_state("solved", &client);
            }
        }

        // React to triggers
        while (current_state == solved || current_state == failed)
        {
            // Emulate the solved state for skipping
            if (trigger_skipped)
            {
                tstruct.colorWhiteInitial();
                clear();
                tstruct.drawInitial(POSX, POSY);
                refresh();
                trigger_skipped = false;
                publish_state("inactive", &client);
            }
            if (trigger_off)
            {
                trigger_off = false;
                publish_state("inactive", &client);
            }
            // Restart riddle
            if (trigger_on)
            {
                current_state = inactive;
            }

            this_thread::sleep_for(chrono::microseconds(500000));
        }
    }
}

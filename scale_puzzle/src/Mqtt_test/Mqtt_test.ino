#include <Arduino.h>
#include "HX711.h"
#include <ESP8266WiFi.h>

#define MQTT_KEEPALIVE 10
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "wifi_pw.h"

//#define DEBUG

// State variables
typedef enum {INIT, WIFI_SETUP, MQTT_SETUP, MQTT_CONNECT, PUZZLE_START,
              SCALE_CALIBRATION, WAIT_FOR_BEGIN, SCALE_GREEN, SCALE_RED,
              PUZZLE_SOLVED, WAIT_FOR_RESTART, MQTT_LOST, WIFI_LOST,
              ERROR_STATE, RESTART} state_t;

typedef enum {LED_STATE_GREEN, LED_STATE_RED, LED_STATE_ORANGE} led_state_t;

// TODO Struct
static state_t state = INIT;
static state_t reconnect_state = ERROR_STATE;
static led_state_t led_state = LED_STATE_GREEN;
static bool puzzle_start = false;
static bool mqtt_connected = false;
static bool scale_measure = false;
static bool led_control = false;
static bool floppys_taken = false;
static bool puzzle_solved = false;
static bool puzzle_restart = false;
static uint8_t connection_check_counter = 0;  

// HX711 circuit wiring
const int LED_GREEN = 0;
const int LED_RED = 5;

HX711 scale;

// WiFi, Mqtt init
WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;
int test_var = 0;
StaticJsonDocument<300> rxdoc;

// MQTT Topics
const char* Safe_activate_topic = "5/safe/activate";
const char* Mqtt_topic = "6/puzzle/scale";
const char* Mqtt_terminal = "6/puzzle/terminal";
// MQTT Messages
const char* Msg_inactive = "{\"method\":\"STATUS\",\"state\":\"inactive\"}";
const char* Msg_active = "{\"method\":\"STATUS\",\"state\":\"active\"}";
const char* Msg_solved = "{\"method\":\"STATUS\",\"state\":\"solved\"}";
const char* Msg_green = "{\"method\":\"TRIGGER\",\"state\":\"on\",\"data\":\"2:2\"}";
const char* Msg_red = "{\"method\":\"TRIGGER\",\"state\":\"on\",\"data\":\"1:2\"}";
const char* Msg_orange = "{\"method\":\"TRIGGER\",\"state\":\"on\",\"data\":\"4:2\"}";


void setup() {
#ifdef DEBUG
  Serial.begin(115200);
  delay(3000);
  Serial.println();
#endif
}

void loop() {

  switch (state) {
    case INIT:
      puzzle_start = false;
      mqtt_connected = false;
      scale_measure = false;
      led_control = false;
      floppys_taken = false;
      puzzle_solved = false;
      puzzle_restart = false;
      debug_state();
      state = WIFI_SETUP;
      break;
    case WIFI_SETUP:
      if (wifi_setup() != false) {
        connection_check_counter = 0;
        debug_state();
        state = MQTT_SETUP;
      }
      break;
    case MQTT_SETUP:
      if (mqtt_setup() != false) {
        delay(1000);
        debug_state();
        state = MQTT_CONNECT;
      }
      break;
    case MQTT_CONNECT:
      if (mqtt_connect() != false) {
        delay(1000);
        mqtt_connected = true;
        debug_state();
        state = PUZZLE_START;
      }
      break;
    case PUZZLE_START:
      // TODO Restart the puzzle here?
        debug_state();
        state = SCALE_CALIBRATION;
      //}
      break;
    case SCALE_CALIBRATION:
      if (calibration_setup() != false) {
        scale_measure = true;
        debug_state();
        if (client.publish(Mqtt_topic, Msg_inactive, true) != false) {
          state = WAIT_FOR_BEGIN;
        }
      }
      break;
    case WAIT_FOR_BEGIN:
      if (mqtt_check() != false) {
        debug_state();
        state = SCALE_GREEN;
      }
      break;
    case SCALE_GREEN:
      if (floppys_taken != false) {
        debug_state();
        if (client.publish(Mqtt_topic, Msg_active, true) != false) {
          state = SCALE_RED;
        }
      }
      break;
    case SCALE_RED:
      if (floppys_taken == false) {
        debug_state();
        if (client.publish(Mqtt_topic, Msg_inactive, true) != false) {
          state = SCALE_GREEN;
        }
      }
      break;
    case PUZZLE_SOLVED:
      if (client.publish(Mqtt_topic, Msg_solved, true) != false) {
        if ((led_state != LED_STATE_GREEN) && (led_control != false)) {
          if (client.publish(Safe_activate_topic, Msg_green, true) != false) {
            led_state = LED_STATE_GREEN;
            puzzle_solved = false;
          }
        }
        // TODO Set gyrosphare off
        debug_state();
        state = WAIT_FOR_RESTART;
      }
      break;
    case WAIT_FOR_RESTART:
      if (puzzle_restart != false) {
        debug_state();
        state = RESTART;
      }
      break;
    case MQTT_LOST:
      if (mqtt_connect() != false) {
        debug_state();
        state = reconnect_state;
      } else {
        mqtt_connected = false;
      }
      
      break;
    case WIFI_LOST:
    // TODO WIFI Überwachung
      //if(wifi_reconnect() != false) {
        debug_state();
        state = INIT;
      //}
      break;     
    case ERROR_STATE:
      debug_state();
      state = INIT;
      break;    
    case RESTART:
      debug_state();
      state = INIT;
      break;
    default:
      break;
  }

  // immediately switch to the solved state when the terminal puzzle begins or the operator determines it
  if ((puzzle_solved != false) && (puzzle_restart == false)) {
    state = PUZZLE_SOLVED; // TODO Can operator send 'solved'?
  }

  // run mqtt handling
  if (mqtt_connected != false) {
    client.loop();
  }

  delay(100);
  
  // run scale measurements
  if (scale_measure != false) {
    scale_loop();
  }

  // run network and mqtt checks approx. every 5s
  if (connection_check_counter > 49) {
    if ((state > WIFI_SETUP) && (state < WIFI_LOST) && (WiFi.status() != WL_CONNECTED)) {
      state = WIFI_LOST;
      Serial.println("WIFI LOST");
    }
    
    if ((state > MQTT_CONNECT) && (state < MQTT_LOST)  && (client.state() != 0)){
      reconnect_state = state;
      state = MQTT_LOST;
    }
    connection_check_counter = 0;
  }
  connection_check_counter += 1;
}


/*****************************************************************************************
                                       FUNCTION INFO
NAME: 
    scale_loop
DESCRIPTION:
    
*****************************************************************************************/
void scale_loop() {
  static int old_reading = 4;
  static int new_reading = 4;
  static int green_count = 0;
  
  if (scale.is_ready()) {
    int reading = scale.get_units(3);

    if (state == SCALE_GREEN) {
      if (reading == 0) {
#ifdef DEBUG
        digitalWrite(LED_GREEN, HIGH);
        digitalWrite(LED_RED, LOW);
#endif
        if ((led_state != LED_STATE_GREEN) && (led_control != false)) {
          if (client.publish(Safe_activate_topic, Msg_green, true) != false) {
            led_state = LED_STATE_GREEN;
          }
        }
      } else {
        floppys_taken = true;
        if ((led_state != LED_STATE_RED) && (led_control != false)) {
          if (client.publish(Safe_activate_topic, Msg_red, true) != false) {
            led_state = LED_STATE_RED;
          }
        }
      }
    }

    if (state == SCALE_RED) {
#ifdef DEBUG
      Serial.print("Value for known weight is: ");
      Serial.println(reading);
#endif
      old_reading = new_reading;
      new_reading = reading;
      if (old_reading == new_reading) {
        if (new_reading == 0) {
          green_count = green_count + 1;
#ifdef DEBUG
          digitalWrite(LED_GREEN, HIGH);
          digitalWrite(LED_RED, LOW);
#endif
          if (led_state != LED_STATE_GREEN) {
            if (client.publish(Safe_activate_topic, Msg_green, true) != false) {
              led_state = LED_STATE_GREEN;
            }
          }
          if (green_count == 3) {
            floppys_taken = false;
          }
        } else if (new_reading == 1 || new_reading == -1) {
          if (led_state != LED_STATE_ORANGE) {
            if (client.publish(Safe_activate_topic, Msg_orange, true) != false) {
              led_state = LED_STATE_ORANGE;
            }
          }
          green_count = 0;
        } else {
#ifdef DEBUG
          digitalWrite(LED_RED, HIGH);
          digitalWrite(LED_GREEN, LOW);
#endif
          if (led_state != LED_STATE_RED) {
            if (client.publish(Safe_activate_topic, Msg_red, true) != false) {
              led_state = LED_STATE_RED;
            }
          }
          green_count = 0;
        }
      } else {
#ifdef DEBUG
        digitalWrite(LED_RED, HIGH);
        digitalWrite(LED_GREEN, LOW);
#endif
        if (led_state != LED_STATE_RED) {
          if (client.publish(Safe_activate_topic, Msg_red, true) != false) {
            led_state = LED_STATE_RED;
          }
        }
        green_count = 0;
      }
    }
  } else {
#ifdef DEBUG
    Serial.println("HX711 not found.");
#endif
    // TODO Go to Error State
    delay(1000);
  }
}


/*****************************************************************************************
                                       FUNCTION INFO
NAME: 
    wifi_setup
DESCRIPTION:
    Connect to the given WiFi. For the password create a file named wifi_pw.h and define 
    a variable "const char* PASSWORD" with the password.
*****************************************************************************************/
bool wifi_setup() {
  bool wifi_finished = false;
  const char* ssid = "ubilab_wifi";
  const char* pw = PASSWORD;

  // TODO Static IP
  
  WiFi.begin(ssid, pw);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
#ifdef DEBUG
    Serial.print(".");
#endif
    if (WiFi.status() == WL_CONNECTED) {
      wifi_finished = true;
#ifdef DEBUG
      Serial.printf("Connection status: %d\n", WiFi.status());
      Serial.println();
      Serial.print("Connected, IP address: ");
      Serial.println(WiFi.localIP());
      Serial.println();
#endif
    }
  }

  return wifi_finished;
}

/*****************************************************************************************
                                       FUNCTION INFO
NAME: 
    mqtt_setup
DESCRIPTION:
    

*****************************************************************************************/
bool mqtt_setup() {
  bool mqtt_setup_finished = false;
  const IPAddress mqttServerIP(10,0,0,2);
  //uint8_t mqtt_server_port = 1883; //TODO
  
  client.setServer(mqttServerIP, 1883);
  client.setCallback(mqtt_callback);
  delay(100);
#ifdef DEBUG
  Serial.println("Keepalive setting:");
  Serial.println(MQTT_KEEPALIVE);
#endif
  
  mqtt_setup_finished = true;
  
  return mqtt_setup_finished;
}

/*****************************************************************************************
                                       FUNCTION INFO
NAME: 
    mqtt_connect
DESCRIPTION:
    

*****************************************************************************************/
bool mqtt_connect() {
  bool reconnect_finished = false;

#ifdef DEBUG
  Serial.print("Attempting MQTT connection...");
#endif
  // Create a random client ID
  String clientId = "ESP8266Client-";
  clientId += String(random(0xffff), HEX);
  // Attempt to connect
  if (client.connect(clientId.c_str())) {  
#ifdef DEBUG
    Serial.println("connected");
#endif
    // Subscribe
    client.subscribe(Mqtt_topic);
    client.subscribe(Mqtt_terminal);
    reconnect_finished = true;
  } else {
#ifdef DEBUG
    Serial.print("failed, rc=");
    Serial.print(client.state());
    Serial.println(" try again in 5 seconds");
#endif
    // Wait 5 seconds before retrying
    delay(5000);
  }

  return reconnect_finished;
}

/*****************************************************************************************
                                       FUNCTION INFO
NAME: 
    mqtt_check
DESCRIPTION:
    

*****************************************************************************************/
bool mqtt_check() {
  bool mqtt_check_finished = false;
// TODO could be done without a function
  if (puzzle_start != false) {
    mqtt_check_finished = true;
  }

  return mqtt_check_finished;
}


/*****************************************************************************************
                                       FUNCTION INFO
NAME: 
    mqtt_callback
DESCRIPTION:
    

*****************************************************************************************/
void mqtt_callback(char* topic, byte* message, unsigned int length) {
#ifdef DEBUG
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
#endif
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
#ifdef DEBUG
    Serial.print((char)message[i]);
#endif
    messageTemp += (char)message[i];
  }
  deserializeJson(rxdoc, message);
  const char* method1 = rxdoc["method"];
  const char* state1 = rxdoc["state"];
  const char* daten = rxdoc["data"];
#ifdef DEBUG
  Serial.println();
  Serial.print("Methode: "); Serial.println(method1);
  Serial.print("State: "); Serial.println(state1);
  Serial.print("Daten: "); Serial.println(daten);
#endif

  // If a message is received on the topic 6/puzzle/scale, check the message.
  if (String(topic).equals(Mqtt_topic) != false) {
    if (String(method1).equals("trigger") != false) {
      if (String(state1).equals("on") != false) {
        if (String(daten).equals("active") != false) { // TODO wird gelöscht
#ifdef DEBUG
          Serial.print("Start Puzzle");
#endif
          puzzle_start = true;
          led_control = true; // TODO Zeitpunkt ausreichend?
        }
      } else if (String(state1).equals("off") != false){
        puzzle_start = false;
      }
    } else if (String(method1).equals("") != false) {
#ifdef DEBUG
      Serial.print("Restart Puzzle");
#endif
      puzzle_restart = true;
    }
  }

  // If a message is received on the topic 6/puzzle/terminal, check the message.
  if (String(topic).equals(Mqtt_terminal) != false) {
    if (String(method1).equals("status") != false) {
      if (String(state1).equals("active") != false) {
#ifdef DEBUG
        Serial.print("Puzzle Soved");
#endif
        puzzle_solved = true;
      }
    }
  }

}

/*****************************************************************************************
                                       FUNCTION INFO
NAME: 
    calibration_setup
DESCRIPTION:
    

*****************************************************************************************/
bool calibration_setup() {
  bool calibration_finished = false;
  const int LOADCELL_DOUT_PIN = 13;
  const int LOADCELL_SCK_PIN = 12;
  // const long offset = -1693891;
  const long divider = 6500;

#ifdef DEBUG
  Serial.println("Beginning:");
#endif
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  scale.set_scale(divider);
  scale.tare();
  //scale.set_offset(offset);
#ifdef DEBUG
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
#endif
  while (!scale.is_ready()) {
    Serial.println("Scale is not ready");
    delay(100);
  }
  calibration_finished = true;

  return calibration_finished;
}

/*****************************************************************************************
                                       FUNCTION INFO
NAME: 
    debug_state
DESCRIPTION:
    

*****************************************************************************************/
void debug_state() {
#ifdef DEBUG
  Serial.println(String(state));
  Serial.println();
#endif
}

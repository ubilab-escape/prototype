#include <Arduino.h>
#include "HX711.h"
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "wifi_pw.h"

#define DEBUG
#define MQTT_KEEPALIVE 10

// State variables
typedef enum {INIT, WIFI_SETUP, MQTT_SETUP, MQTT_CONNECT, PUZZLE_START,
              SCALE_CALIBRATION, WAIT_FOR_BEGIN, SCALE_GREEN, SCALE_RED,
              PUZZLE_SOLVED, WAIT_FOR_RESTART, MQTT_LOST, WIFI_LOST,
              ERROR_STATE, RESTART} state_t;

typedef enum {LED_STATE_GREEN, LED_STATE_RED, LED_STATE_ORANGE} led_state_t;


typedef struct ScalePuzzle_HandlerType {
  state_t state = INIT;
  state_t reconnect_state = ERROR_STATE;
  led_state_t led_state = LED_STATE_GREEN;
  bool puzzle_start = false;
  bool mqtt_connected = false;
  bool scale_measure = false;
  bool led_control = false;
  bool floppys_taken = false;
  bool puzzle_solved = false;
  bool puzzle_restart = false;
  uint8_t connection_check_counter = 0;
};

// HX711 circuit wiring
const int LOADCELL_DOUT_PIN = 13;
const int LOADCELL_SCK_PIN = 12;
const int LED_GREEN = 0;
const int LED_RED = 5; 

HX711 scale;
const long scale_divider = 7000;

ScalePuzzle_HandlerType ScaleStruct;

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
const char* Msg_inactive = "{\"method\":\"status\",\"state\":\"inactive\"}";
const char* Msg_active = "{\"method\":\"status\",\"state\":\"active\"}";
const char* Msg_solved = "{\"method\":\"status\",\"state\":\"solved\"}";
const char* Msg_green = "{\"method\":\"trigger\",\"state\":\"on\",\"data\":\"2:2\"}";
const char* Msg_red = "{\"method\":\"trigger\",\"state\":\"on\",\"data\":\"1:2\"}";
const char* Msg_orange = "{\"method\":\"trigger\",\"state\":\"on\",\"data\":\"4:2\"}";


void setup() {
#ifdef DEBUG
  Serial.begin(115200);
  delay(3000);
  Serial.println();
#endif
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  scale.set_scale(scale_divider);
}

void loop() {

  switch (ScaleStruct.state) {
    case INIT:
      InitScalePuzzleStructure();
      debug_state();
      ScaleStruct.state = WIFI_SETUP;
      break;
    case WIFI_SETUP:
      if (wifi_setup() != false) {
        delay(500);
        ScaleStruct.connection_check_counter = 0;
        debug_state();
        ScaleStruct.state = MQTT_SETUP;
      }
      break;
    case MQTT_SETUP:
      if (mqtt_setup() != false) {
        delay(500);
        debug_state();
        ScaleStruct.state = MQTT_CONNECT;
      }
      break;
    case MQTT_CONNECT:
      if (mqtt_connect() != false) {
        delay(500);
        ScaleStruct.mqtt_connected = true;
        debug_state();
        ScaleStruct.state = PUZZLE_START;
      }
      break;
    case PUZZLE_START:
      // TODO Restart the puzzle here! Call struct Init
        debug_state();
        ScaleStruct.state = SCALE_CALIBRATION;
      //}
      break;
    case SCALE_CALIBRATION:
      if (calibration_setup() != false) {
        ScaleStruct.scale_measure = true;
        debug_state();
        if (client.publish(Mqtt_topic, Msg_inactive, true) != false) {
          ScaleStruct.state = WAIT_FOR_BEGIN;
        }
      }
      break;
    case WAIT_FOR_BEGIN:
      if (mqtt_check() != false) {
        debug_state();
        ScaleStruct.state = SCALE_GREEN;
      }
      break;
    case SCALE_GREEN:
      if (ScaleStruct.floppys_taken != false) {
        debug_state();
        if (client.publish(Mqtt_topic, Msg_active, true) != false) {
          ScaleStruct.state = SCALE_RED;
        }
      }
      break;
    case SCALE_RED:
      if (ScaleStruct.floppys_taken == false) {
        debug_state();
        if (client.publish(Mqtt_topic, Msg_inactive, true) != false) {
          ScaleStruct.state = SCALE_GREEN;
        }
      }
      break;
    case PUZZLE_SOLVED:
      if (client.publish(Mqtt_topic, Msg_solved, true) != false) {
        set_safe_color(LED_STATE_GREEN);
        ScaleStruct.puzzle_solved = false;
        debug_state();
        ScaleStruct.state = WAIT_FOR_RESTART;
      }
      break;
    case WAIT_FOR_RESTART:
      if (ScaleStruct.puzzle_restart != false) {
        debug_state();
        ScaleStruct.state = RESTART;
      }
      break;
    case MQTT_LOST:
      if (mqtt_connect() != false) {
        debug_state();
        ScaleStruct.state = ScaleStruct.reconnect_state;
      } else {
        ScaleStruct.mqtt_connected = false;
      }
      break;
    case WIFI_LOST:
      WiFi.disconnect();
      debug_state();
      ScaleStruct.state = INIT;
      break;     
    case ERROR_STATE:
      debug_state();
      ScaleStruct.state = INIT;
      break;    
    case RESTART:
      debug_state();
      ScaleStruct.state = PUZZLE_START;
      break;
    default:
      break;
  }

  // immediately switch to the solved state when the terminal puzzle begins or the operator determines it
  if ((ScaleStruct.puzzle_solved != false) && (ScaleStruct.puzzle_restart == false)) {
    ScaleStruct.state = PUZZLE_SOLVED;
  }
  if ((ScaleStruct.puzzle_solved == false) && (ScaleStruct.puzzle_restart == true)) {
    ScaleStruct.puzzle_restart = false;
    ScaleStruct.state = RESTART;
  }

  // run mqtt handling
  if (ScaleStruct.mqtt_connected != false) {
    client.loop();
  }

  delay(100);
  
  // run scale measurements
  if (ScaleStruct.scale_measure != false) {
    scale_loop();
  }

  // run network and mqtt checks approx. every 5s
  if (ScaleStruct.connection_check_counter > 49) {
    if ((ScaleStruct.state > WIFI_SETUP) && 
        (ScaleStruct.state < WIFI_LOST) && 
        (WiFi.status() != WL_CONNECTED)) {
      ScaleStruct.state = WIFI_LOST;
      Serial.println("WIFI LOST");
    }
    
    if ((ScaleStruct.state > MQTT_CONNECT) && 
        (ScaleStruct.state < MQTT_LOST)  && 
        (client.state() != 0)){
      ScaleStruct.reconnect_state = ScaleStruct.state;
      ScaleStruct.state = MQTT_LOST;
    }
    ScaleStruct.connection_check_counter = 0;
  }
  ScaleStruct.connection_check_counter += 1;
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
    int reading = round(scale.get_units(3));

    if (ScaleStruct.state == SCALE_GREEN) {
      if (reading == 0) {
#ifdef DEBUG
        digitalWrite(LED_GREEN, HIGH);
        digitalWrite(LED_RED, LOW);
#endif
        set_safe_color(LED_STATE_GREEN);
      } else {
        ScaleStruct.floppys_taken = true;
        set_safe_color(LED_STATE_RED);
      }
    }

    if (ScaleStruct.state == SCALE_RED) {
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
          set_safe_color(LED_STATE_GREEN);
          
          if (green_count == 3) {
            ScaleStruct.floppys_taken = false;
          }
        } else if (new_reading == 1 || new_reading == -1) {
          set_safe_color(LED_STATE_ORANGE);
          green_count = 0;
        } else {
#ifdef DEBUG
          digitalWrite(LED_RED, HIGH);
          digitalWrite(LED_GREEN, LOW);
#endif
          set_safe_color(LED_STATE_RED);
          green_count = 0;
        }
      } else {
#ifdef DEBUG
        digitalWrite(LED_RED, HIGH);
        digitalWrite(LED_GREEN, LOW);
#endif
        set_safe_color(LED_STATE_RED);
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
    set_safe_color
DESCRIPTION:
    
*****************************************************************************************/
void set_safe_color(led_state_t color_state){
  const char* color_message;
  
  if (ScaleStruct.led_control != false) {
    if ((ScaleStruct.led_state != color_state)) {
      switch (color_state) {
        case LED_STATE_GREEN:
          color_message = Msg_green;
          break;
        case LED_STATE_ORANGE:
          color_message = Msg_orange;
          break;
        case LED_STATE_RED:
          color_message = Msg_red;
          break;
        default:
          color_message = Msg_green;
          break;
      }
      if (client.publish(Safe_activate_topic, color_message, true) != false) {
        ScaleStruct.led_state = color_state;
      }
    }
  }
}

/*****************************************************************************************
                                       FUNCTION INFO
NAME: 
    InitScalePuzzleStructure
DESCRIPTION:

*****************************************************************************************/
void InitScalePuzzleStructure(void) {
  ScaleStruct.state = INIT;
  ScaleStruct.reconnect_state = ERROR_STATE;
  ScaleStruct.led_state = LED_STATE_GREEN;
  ScaleStruct.puzzle_start = false;
  ScaleStruct.mqtt_connected = false;
  ScaleStruct.scale_measure = false;
  ScaleStruct.led_control = false;
  ScaleStruct.floppys_taken = false;
  ScaleStruct.puzzle_solved = false;
  ScaleStruct.puzzle_restart = false;
  ScaleStruct.connection_check_counter = 0;
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
  uint16_t mqtt_server_port = 1883;
  
  client.setServer(mqttServerIP, mqtt_server_port);
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
  if (ScaleStruct.puzzle_start != false) {
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
      if (String(state1).equals("on") != false) { // TODO wird gelöscht
#ifdef DEBUG
        Serial.print("Start Puzzle");
#endif
        ScaleStruct.puzzle_start = true;
        ScaleStruct.led_control = true; // TODO Zeitpunkt ausreichend?
      } else if (String(state1).equals("off") != false){
        ScaleStruct.puzzle_start = false;
      }
    } else if (String(method1).equals("") != false) {
#ifdef DEBUG
      Serial.print("Restart Puzzle");
#endif
      ScaleStruct.puzzle_restart = true;
    }
  }

  // If a message is received on the topic 6/puzzle/terminal, check the message.
  if (String(topic).equals(Mqtt_terminal) != false) {
    if (String(method1).equals("status") != false) {
      if (String(state1).equals("active") != false) {
#ifdef DEBUG
        Serial.print("Puzzle Solved");
#endif
        ScaleStruct.puzzle_solved = true;
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

#ifdef DEBUG
  Serial.println("Beginning:");
#endif
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
void debug_state() { // TODO DEBUG PRINT FUNKTION
#ifdef DEBUG
  Serial.println(String(ScaleStruct.state));
  Serial.println();
#endif
}

// TODO Ab und zu stuck at active trotz weight = 0;

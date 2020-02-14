#include <Arduino.h>
#include <ESP8266WiFi.h>
/* define keepalive settings before PubSubClient.h */
#define MQTT_KEEPALIVE 10
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "HX711.h"
#include "Mqtt_test.h"
#include "wifi_pw.h"

/* uncomment to activate debug mode */
//#define DEBUG

/* State machine variables */
ScalePuzzle_HandlerType ScaleStruct;

/* Scale variables */
HX711 scale;
const long scale_divider = 7000;
const int LOADCELL_DOUT_PIN = 13;
const int LOADCELL_SCK_PIN = 12;
const int LED_GREEN = 0;
const int LED_RED = 5; 

/* WiFi, Mqtt variables */
WiFiClient espClient;
PubSubClient client(espClient);
StaticJsonDocument<300> rxdoc;

/* MQTT Topics */
const char* Safe_activate_topic = "5/safe/control";
const char* Mqtt_topic = "6/puzzle/scale";
const char* Mqtt_terminal = "6/puzzle/terminal";
/* MQTT Messages */
const char* Msg_inactive = "{\"method\": \"status\", \"state\": \"inactive\"}";
const char* Msg_active = "{\"method\": \"status\", \"state\": \"active\"}";
const char* Msg_solved = "{\"method\":\"status\",\"state\":\"solved\"}";
const char* Msg_green = "{\"method\":\"trigger\",\"state\":\"on\",\"data\":\"2:0\"}"; // Constant green
const char* Msg_red = "{\"method\":\"trigger\",\"state\":\"on\",\"data\":\"1:3\"}"; // Blinking red
const char* Msg_orange = "{\"method\":\"trigger\",\"state\":\"on\",\"data\":\"4:3\"}"; // Blinking orange


/* Arduino main functions */

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
  /* scale puzzle state machine */
  switch (ScaleStruct.state) {
    case INIT:
      InitScalePuzzleStructure();
      debug_print(String(ScaleStruct.state));
      ScaleStruct.state = WIFI_SETUP;
      wifi_set_sleep_type(NONE_SLEEP_T);
      break;
    case WIFI_SETUP:
      if (wifi_setup() != false) {
        delay(500);
        ScaleStruct.connection_check_counter = 0;
        debug_print(String(ScaleStruct.state));
        ScaleStruct.state = MQTT_SETUP;
      }
      break;
    case MQTT_SETUP:
      if (mqtt_setup() != false) {
        delay(500);
        debug_print(String(ScaleStruct.state));
        ScaleStruct.state = MQTT_CONNECT;
      }
      break;
    case MQTT_CONNECT:
      if (mqtt_connect() != false) {
        delay(500);
        ScaleStruct.mqtt_connected = true;
        debug_print(String(ScaleStruct.state));
        ScaleStruct.state = PUZZLE_START;
      }
      break;
    case PUZZLE_START:
      ReInitScalePuzzleStructure();
      debug_print(String(ScaleStruct.state));
      ScaleStruct.state = SCALE_CALIBRATION;
      break;
    case SCALE_CALIBRATION:
      if (calibration_setup() != false) {
        debug_print(String(ScaleStruct.state));
        if (client.publish(Mqtt_topic, Msg_inactive, true) != false) {
          ScaleStruct.state = WAIT_FOR_BEGIN;
        }
      }
      break;
    case WAIT_FOR_BEGIN:
      if (ScaleStruct.puzzle_start != false) {
        debug_print(String(ScaleStruct.state));
        ScaleStruct.state = SCALE_GREEN;
      }
      break;
    case SCALE_GREEN:
      if (is_scale_unbalanced() != false) {
        debug_print(String(ScaleStruct.state));
        if (client.publish(Mqtt_topic, Msg_active, true) != false) {
          ScaleStruct.state = SCALE_RED;
        }
      }
      break;
    case SCALE_RED:
      if (is_scale_balanced() != false) {
        debug_print(String(ScaleStruct.state));
        if (client.publish(Mqtt_topic, Msg_inactive, true) != false) {
          ScaleStruct.state = SCALE_GREEN;
        }
      }
      break;
    case PUZZLE_SOLVED:
      if (client.publish(Mqtt_topic, Msg_solved, true) != false) {
        set_safe_color(LED_STATE_GREEN);
        ScaleStruct.puzzle_solved = false;
        debug_print(String(ScaleStruct.state));
        ScaleStruct.state = WAIT_FOR_RESTART;
      }
      break;
    case WAIT_FOR_RESTART:
      if (ScaleStruct.puzzle_restart != false) {
        debug_print(String(ScaleStruct.state));
        ScaleStruct.state = RESTART;
      }
      break;
    case MQTT_LOST:
      if (mqtt_connect() != false) {
        debug_print(String(ScaleStruct.state));
        ScaleStruct.state = ScaleStruct.reconnect_state;
      } else {
        ScaleStruct.mqtt_connected = false;
      }
      break;
    case WIFI_LOST:
      WiFi.disconnect();
      debug_print(String(ScaleStruct.state));
      ScaleStruct.state = INIT;
      break;     
    case ERROR_STATE:
      WiFi.disconnect();
      debug_print(String(ScaleStruct.state));
      ScaleStruct.state = INIT;
      break;    
    case RESTART:
      debug_print(String(ScaleStruct.state));
      ScaleStruct.state = PUZZLE_START;
      break;
    default:
      break;
  }

  /* puzzle solved trigger or skip puzzle trigger arrived */
  if ((ScaleStruct.puzzle_solved != false) && (ScaleStruct.puzzle_restart == false)) {
    ScaleStruct.state = PUZZLE_SOLVED;
  }
  /* restart trigger arrived */
  if ((ScaleStruct.puzzle_solved == false) && (ScaleStruct.puzzle_restart != false)) {
    ScaleStruct.puzzle_restart = false;
    ScaleStruct.state = RESTART;
  }
  /* restart if both triggers are sent */
  if ((ScaleStruct.puzzle_solved != false) && (ScaleStruct.puzzle_restart != false)) {
    ScaleStruct.state = RESTART;
    ScaleStruct.puzzle_solved = false;
    ScaleStruct.puzzle_restart = false;
  }

  /* run mqtt handling */
  if (ScaleStruct.mqtt_connected != false) {
    client.loop();
  }

  delay(100);
  
  /* run wifi and mqtt checks approx. every 5s */
  if (ScaleStruct.connection_check_counter > 49) {
    if ((ScaleStruct.state > WIFI_SETUP) && 
        (ScaleStruct.state < WIFI_LOST) && 
        (WiFi.status() != WL_CONNECTED)) {
      ScaleStruct.state = WIFI_LOST;
      debug_print("WIFI LOST");
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

/* State Machine functions */

/*****************************************************************************************
                                       FUNCTION INFO
NAME: 
    InitScalePuzzleStructure
DESCRIPTION:
    Initialization function to initialize the Scale Puzzle Structure.
    To call at every startup of the device.
*****************************************************************************************/
void InitScalePuzzleStructure(void) {
  ScaleStruct.state = INIT;
  ScaleStruct.mqtt_connected = false;
  ScaleStruct.connection_check_counter = 0;
  ScaleStruct.scale_failure = 0;
  ReInitScalePuzzleStructure();
}

/*****************************************************************************************
                                       FUNCTION INFO
NAME: 
    ReInitScalePuzzleStructure
DESCRIPTION:
    Initialization function to initialize the Scale Puzzle Structure.
    To call at every restart of the puzzle.
*****************************************************************************************/
void ReInitScalePuzzleStructure(void) {
  ScaleStruct.reconnect_state = ERROR_STATE;
  ScaleStruct.led_state = LED_STATE_GREEN;
  ScaleStruct.puzzle_start = false;
  ScaleStruct.led_control = false;
  ScaleStruct.puzzle_solved = false;
  ScaleStruct.puzzle_restart = false;
}

/*****************************************************************************************
                                       FUNCTION INFO
NAME: 
    wifi_setup
DESCRIPTION:
    Connect to the given WiFi. For the password create a file named wifi_pw.h and define 
    a variable "const char* PASSWORD" with the password.
*****************************************************************************************/
bool wifi_setup(void) {
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
      delay(500);
      debug_print(String("Connection status: " + String(WiFi.status())));
      delay(500);
      debug_print("Connected, IP address: ");
      debug_print(WiFi.localIP().toString());
    } else {
      delay(500);
    }
  }

  return wifi_finished;
}

/*****************************************************************************************
                                       FUNCTION INFO
NAME: 
    mqtt_setup
DESCRIPTION:
    Submits the MQTT connection with the specified IP address and MQTT server port to the
    MQTT client
    Submits the callback function with a given set of messages to the MQTT client.
*****************************************************************************************/
bool mqtt_setup(void) {
  bool mqtt_setup_finished = false;
  const IPAddress mqttServerIP(10,0,0,2);
  uint16_t mqtt_server_port = 1883;
  
  client.setServer(mqttServerIP, mqtt_server_port);
  client.setCallback(mqtt_callback);
  delay(100);
  debug_print("Keepalive setting:");
  debug_print(String(MQTT_KEEPALIVE));
  
  mqtt_setup_finished = true;
  
  return mqtt_setup_finished;
}

/*****************************************************************************************
                                       FUNCTION INFO
NAME: 
    mqtt_connect
DESCRIPTION:
    Creates a new client ID and connects to the MQTT server.
    Creates subscription to the specified MQTT topics.
    If no connection is possible, wait 5 seconds before the next try.
*****************************************************************************************/
bool mqtt_connect(void) {
  bool reconnect_finished = false;

  debug_print("Attempting MQTT connection...");

  /* Create a random client ID */
  String clientId = "ESP8266Scale-";
  clientId += String(random(0xffff), HEX);
  /* Attempt to connect */
  if (client.connect(clientId.c_str())) {  
    debug_print("connected");
    
    /* Subscribe */
    client.subscribe(Mqtt_topic);
    client.subscribe(Mqtt_terminal);
    
    reconnect_finished = true;
  } else {
    debug_print("failed, rc=");
    debug_print(String(client.state()));
    debug_print("try again in 5 seconds");

    /* Wait 2 seconds before retrying */
    delay(2000);
  }

  return reconnect_finished;
}

/*****************************************************************************************
                                       FUNCTION INFO
NAME: 
    calibration_setup
DESCRIPTION:
    Tare the scale to the current weight and wait until the scale is ready.
*****************************************************************************************/
bool calibration_setup(void) {
  bool calibration_finished = false;

  scale.tare();

  debug_print("Beginning:");
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
    is_scale_unbalanced
DESCRIPTION:
    Measure the weight on the scale and if a wrong weight is measured, change the color
    of the safe to red. The measuring process is debounced.
*****************************************************************************************/
bool is_scale_unbalanced(void) {
  bool unbalanced = false;
  static int red_count = 0;

  if (scale_measure_floppy_disks() == 0) {
    red_count = 0;
  } else {
    red_count++;
    if (red_count == RED_DELAY) {
      debug_led(LED_RED);
      set_safe_color(LED_STATE_RED);
      red_count = 0;
      unbalanced = true;
    }
  }

  return unbalanced;
}

/*****************************************************************************************
                                       FUNCTION INFO
NAME: 
    is_scale_balanced
DESCRIPTION:
    Measure the weight on the scale and if the initially tared weight is measured, change
    the color of the safe to green. The measuring process is debounced.
*****************************************************************************************/
bool is_scale_balanced(void) {
  bool balanced = false;
  static int old_reading = 4;
  static int new_reading = 4;
  static int green_count = 0;
  int reading = scale_measure_floppy_disks();
   
  old_reading = new_reading;
  new_reading = reading;
  if (old_reading == new_reading) {
    if (new_reading == 0) {
      green_count++;
      
      if (green_count == GREEN_DELAY) {
        debug_led(LED_GREEN);
        set_safe_color(LED_STATE_GREEN);
        green_count = 0;
        balanced = true;
      }
      
    } else {
      green_count = 0;
    }
  } else {
    green_count = 0;
  }
  
  return balanced;
}

/* Utility Functions */

/*****************************************************************************************
                                       FUNCTION INFO
NAME: 
    mqtt_callback
DESCRIPTION:
    Evaluates the received MQTT messages. The following messages are processed:
    Topic: 6/puzzle/scale
    Messages: trigger on            || starts the puzzle
              trigger off           || restarts the puzzle
              trigger off skipped   || skips the puzzle and starts puzzle solved handling
    Topic: 6/puzzle/terminal
    Message: status active          || sets the puzzle to solved
*****************************************************************************************/
void mqtt_callback(char* topic, byte* message, unsigned int length) {
  debug_print("Message arrived on topic: ");
  debug_print(topic);
  debug_print(". Message: ");

  String messageTemp;
  
  for (unsigned int i = 0; i < length; i++) {
    messageTemp += (char)message[i];
  }
  debug_print(messageTemp);
  
  deserializeJson(rxdoc, message);
  const char* method1 = rxdoc["method"];
  const char* state1 = rxdoc["state"];
  const char* data1 = rxdoc["data"];
  /*debug_print("Methode: "); 
  debug_print(method1);
  debug_print("State: "); 
  debug_print(state1);
  debug_print("Daten: "); 
  debug_print(daten);*/

  /* If a message is received on the topic 6/puzzle/scale, check the message. */
  if (String(topic).equals(Mqtt_topic) != false) {
    if (String(method1).equals("trigger") != false) {
      if (String(state1).equals("on") != false) {
        debug_print("Start Puzzle");
        ScaleStruct.puzzle_start = true;
        ScaleStruct.led_control = true;
      } else if (String(state1).equals("off") != false){
        if (String(data1).equals("skipped") != false) {
          debug_print("Puzzle Skipped");
          ScaleStruct.puzzle_solved = true;
        } else {
          debug_print("Restart Puzzle");
          ScaleStruct.puzzle_restart = true;
        }
      }
    }
  }

  /* If a message is received on the topic 6/puzzle/terminal, check the message. */
  if (String(topic).equals(Mqtt_terminal) != false) {
    if (String(method1).equals("status") != false) {
      if (String(state1).equals("active") != false) {
        debug_print("Puzzle Solved");
        ScaleStruct.puzzle_solved = true;
      }
    }
  }

}

/*****************************************************************************************
                                       FUNCTION INFO
NAME: 
    scale_measure_floppy_disks
DESCRIPTION:
    Measures the weight on the scale. The scale is tared to a certain amount of floppy
    disks. The amount of floppy disks is returned.
    If the scale is not reacting for a specific time go to the error state.
*****************************************************************************************/
int scale_measure_floppy_disks() {
  int measure = 0;
  
  if (scale.is_ready()) {
    measure = round(scale.get_units(10));
    ScaleStruct.scale_failure = 0;

    debug_print("Value for known weight is: ");
    debug_print(String(measure));
  } else {
    debug_print("HX711 not found.");
    ScaleStruct.scale_failure = ScaleStruct.scale_failure + 1;
    if (ScaleStruct.scale_failure > 20) {
      ScaleStruct.state = ERROR_STATE;
    }
    delay(500);
  }

  return measure;
}

/*****************************************************************************************
                                       FUNCTION INFO
NAME: 
    set_safe_color
DESCRIPTION:
    Sends a MQTT message to the safe with a specific color. The colors green, red and
    orange are possible.
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
      if (client.publish(Safe_activate_topic, color_message, false) != false) {
        ScaleStruct.led_state = color_state;
      }
    }
  }
}

/*****************************************************************************************
                                       FUNCTION INFO
NAME: 
    debug_print
DESCRIPTION:
    Debug function to print messages via serial connection.
*****************************************************************************************/
void debug_print(String print_string) {
#ifdef DEBUG
  Serial.println(print_string);
#endif
}

/*****************************************************************************************
                                       FUNCTION INFO
NAME: 
    debug_led
DESCRIPTION:
    Debug function to connect LEDs to show the current scale status.
*****************************************************************************************/
void debug_led(int led_color) {
#ifdef DEBUG
  if (led_color == LED_RED) {
    digitalWrite(LED_RED, HIGH);
    digitalWrite(LED_GREEN, LOW);
  }
  if (led_color == LED_GREEN) {
    digitalWrite(LED_RED, LOW);
    digitalWrite(LED_GREEN, HIGH);
  }
#endif
}

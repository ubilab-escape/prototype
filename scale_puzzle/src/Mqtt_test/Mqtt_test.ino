#include <Arduino.h>
#include "HX711.h"
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "wifi_pw.h"


// State variables
typedef enum { INIT, WIFI_SETUP, MQTT_SETUP, MQTT_CONNECT, 
               WAIT_FOR_BEGIN, PUZZLE_START, SCALE_CALIBRATION,
               SCALE_GREEN, SCALE_RED, PUZZLE_SOLVED, RESTART,
               ERROR_STATE, MQTT_LOST, WIFI_LOST} state_t;

typedef enum { LED_STATE_GREEN, LED_STATE_RED, LED_STATE_ORANGE} led_state_t;

state_t state = INIT;
static bool puzzle_start = false;
static bool mqtt_connected = false;
static bool scale_measure = false;
static bool floppys_taken = false;

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

const char* Mqtt_topic = "6/puzzle/scale";
const char* Msg_inactive = "{\"method\":\"STATUS\",\"state\":\"inactive\"}";
const char* Msg_active = "{\"method\":\"STATUS\",\"state\":\"active\"}";
const char* Msg_solved = "{\"method\":\"STATUS\",\"state\":\"solved\"}";
const char* Led_topic = "5/safe/activate";
const char* Msg_green = "{\"method\":\"TRIGGER\",\"state\":\"on\",\"data\":\"2:2\"}";
const char* Msg_red = "{\"method\":\"TRIGGER\",\"state\":\"on\",\"data\":\"1:2\"}";
const char* Msg_orange = "{\"method\":\"TRIGGER\",\"state\":\"on\",\"data\":\"4:2\"}";


void setup() {
  Serial.begin(115200);
  delay(3000);
}

void loop() {

  switch (state) {
    case INIT:
      //if (true != false) {
        puzzle_start = false;
        mqtt_connected = false;
        scale_measure = false;
        floppys_taken = false;
        Serial.println(String(state));
        Serial.println();
        state = WIFI_SETUP;
      //}
      break;
    case WIFI_SETUP:
      if (wifi_setup() != false) {
        Serial.println(String(state));
        Serial.println();
        state = MQTT_SETUP;
      }
      break;
    case MQTT_SETUP:
      if (mqtt_setup() != false) {
        Serial.println(String(state));
        Serial.println();
        state = MQTT_CONNECT;
      }
      break;
    case MQTT_CONNECT:
      if (mqtt_connect() != false) {
        client.publish(Mqtt_topic, Msg_inactive, true);
        mqtt_connected = true;
        state = WAIT_FOR_BEGIN;
      }
      break;
    case WAIT_FOR_BEGIN:
      if (mqtt_check() != false) {
        Serial.println(String(state));
        Serial.println();
        state = PUZZLE_START;
      }
      break;
    case PUZZLE_START:
      //TODO publish -> if ( != false) {
        value = client.publish(Mqtt_topic, Msg_active, true);
        Serial.println(String(state));
        Serial.println();
        state = SCALE_CALIBRATION;
      //}
      break;
    case SCALE_CALIBRATION:
      if (calibration_setup() != false) {
        Serial.println(String(state));
        Serial.println();
        state = SCALE_GREEN;
        scale_measure = true;
      }
      break;
    case SCALE_GREEN:
      if (floppys_taken != false) {
        Serial.println(String(state));
        Serial.println();
        state = SCALE_RED;
      }
      break;
    case SCALE_RED:
      if (floppys_taken == false) {
        //TODO verify message sent
        Serial.println(String(state));
        Serial.println();
        state = SCALE_GREEN;
      }
      break;
    case PUZZLE_SOLVED:
      //if ( != false) {
        value = client.publish(Mqtt_topic, Msg_solved, true);
        Serial.println(String(state));
        Serial.println();
        state = RESTART;
      //}
      break;
    case RESTART:
      state = INIT;
      break;
    case ERROR_STATE:
      break;
    case MQTT_LOST:
      // TODO restart scale?
      if (mqtt_connect() != false) {
        state = WAIT_FOR_BEGIN;
      } else {
        mqtt_connected = false;
      }
      
      break;
    case WIFI_LOST:
    // TODO WIFI Überwachung
      break;
  }
  
  if (mqtt_connected != false) {
    client.loop();
  }
  delay(10);
  //Serial.printf("MQTT Return: %d\n", value);
  if (scale_measure != false) {
    scale_loop();
  }
}

void scale_loop() {
  static int old_reading = 4;
  static int new_reading = 4;
  static int green_count = 0;
  static led_state_t led_state = LED_STATE_GREEN;
  
  if (scale.is_ready()) {
    int reading = scale.get_units(3);

    if (state == SCALE_GREEN) {
      if (reading == 0) {
        digitalWrite(LED_GREEN, HIGH);
        digitalWrite(LED_RED, LOW);
        if (led_state != LED_STATE_GREEN) {
          value = client.publish(Led_topic, Msg_green, true);
          led_state = LED_STATE_GREEN;
        }
      } else {
        floppys_taken = true;
        if (led_state != LED_STATE_RED) {
        value = client.publish(Led_topic, Msg_red, true);
        led_state = LED_STATE_RED;
        }
        // -> Alarm
      }
    }

    if (state == SCALE_RED) {
      Serial.print("Value for known weight is: ");
      Serial.println(reading);
      old_reading = new_reading;
      new_reading = reading;
      if (old_reading == new_reading) {
        if (new_reading == 0) {
          green_count = green_count + 1;
          digitalWrite(LED_GREEN, HIGH);
          digitalWrite(LED_RED, LOW);
          if (led_state != LED_STATE_GREEN) {
            value = client.publish(Led_topic, Msg_green, true);
            led_state = LED_STATE_GREEN;
          }
          if (green_count == 3) {
            floppys_taken = false;
          }
        } else if (new_reading == 1 || new_reading == -1) {
          if (led_state != LED_STATE_ORANGE) {
            value = client.publish(Led_topic, Msg_orange, true);
            led_state = LED_STATE_ORANGE;
          }
          green_count = 0;
        } else {
          digitalWrite(LED_RED, HIGH);
          digitalWrite(LED_GREEN, LOW);
          if (led_state != LED_STATE_RED) {
            value = client.publish(Led_topic, Msg_red, true);
            led_state = LED_STATE_RED;
          }
          green_count = 0;
        }
      } else {
        digitalWrite(LED_RED, HIGH);
        digitalWrite(LED_GREEN, LOW);
        if (led_state != LED_STATE_RED) {
          value = client.publish(Led_topic, Msg_red, true);
          led_state = LED_STATE_RED;
        }
        green_count = 0;
      }
    }
  } else {
    // Serial.println("HX711 not found.");
    // TODO Go to Error State
    delay(200);
  }
  //delay(1000);
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
    Serial.print(".");
    if (WiFi.status() == WL_CONNECTED) {
      wifi_finished = true;
      Serial.printf("Connection status: %d\n", WiFi.status());
      Serial.println();
      Serial.print("Connected, IP address: ");
      Serial.println(WiFi.localIP());
      Serial.println();
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
  uint8_t mqtt_server_port = 1883;
  
  client.setServer(mqttServerIP, 1883);
  client.setCallback(mqtt_callback);
  delay(100);
  
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
  
  Serial.print("Attempting MQTT connection...");
  // Create a random client ID
  String clientId = "ESP8266Client-";
  clientId += String(random(0xffff), HEX);
  // Attempt to connect
  if (client.connect(clientId.c_str())) {
    Serial.println("connected");
    // Subscribe
    client.subscribe(Mqtt_topic);
    reconnect_finished = true;
  } else {
    Serial.print("failed, rc=");
    Serial.print(client.state());
    Serial.println(" try again in 5 seconds");
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
//TODO could be done without a function
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
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();
  deserializeJson(rxdoc, message);
  const char* method1 = rxdoc["method"];
  const char* state1 = rxdoc["state"];
  const char* daten = rxdoc["data"];
  Serial.print("Methode: "); Serial.println(method1);
  Serial.print("State: "); Serial.println(state1);
  Serial.print("Daten: "); Serial.println(daten);
  
  
  //String str_topic = String(topic);
  //str_topic.remove(0,2);
  //Serial.println(str_topic);
  
  // Feel free to add more if statements to control more GPIOs with MQTT

  // If a message is received on the topic esp32/output, you check if the message is either "on" or "off". 
  // Changes the output state according to the message
  if (String(topic).equals(Mqtt_topic) != false) {
    // ODER AUS MESSAGETEMP AUSLESEN
    if (String(method1).equals("trigger") != false) {
      if (String(state1).equals("on") != false) {
        if (String(daten).equals("active") != false) {
          Serial.print("Start Puzzle");
          puzzle_start = true;
        }
      } else if (String(state1).equals("off") != false){
        puzzle_start = false;
      }
    }
    if (String(method1).equals("trigger") != false) {
      if (String(state1).equals("on") != false) {
        if (String(daten).equals("solved") != false) {
          Serial.print("Puzzle Soved");
          state = PUZZLE_SOLVED; //TODO über flag
        }
      }
    }

    

    // TODO set to solved from operator -> grüne LEDs? Alarm aus
        
    /*Serial.print("Changing output to ");
    if(messageTemp == "on"){
      Serial.println("on");
    
    }
    else if(messageTemp == "off"){
      Serial.println("off");
      
    }*/
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
  
  Serial.println("Beginning:");
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  scale.set_scale(divider);
  scale.tare();
  //scale.set_offset(offset);
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);

  calibration_finished = true;

  return calibration_finished;
}

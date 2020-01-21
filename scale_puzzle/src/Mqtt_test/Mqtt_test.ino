#include <Arduino.h>
#include "HX711.h"
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "wifi_pw.h"


// State variables
typedef enum { INIT, WIFI_SETUP, MQTT_SETUP, WAIT_FOR_BEGIN,
               PUZZLE_START, SCALE_CALIBRATION, SCALE_GREEN,
               SCALE_RED, PUZZLE_SOLVED, RESTART, ERROR_STATE,
               MQTT_LOST, WIFI_LOST} state_t;

state_t state = INIT;
static bool puzzle_start = false;
static bool mqtt_connected = false;
static bool scale_measure = false;

// HX711 circuit wiring
const int LED_GREEN = 0;
const int LED_RED = 5;

const long offset = -1693891;
const long divider = 6500;
int old_reading = 4;
int new_reading = 4;

HX711 scale;

// WiFi, Mqtt init
WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;
StaticJsonDocument<300> rxdoc;

const char* mqtt_topic = "6/puzzle/scale";
const char* Msg_inactive = "{\"method\":\"STATUS\",\"state\":\"inactive\"}";
const char* Msg_active = "{\"method\":\"STATUS\",\"state\":\"active\"}";
const char* Msg_solved = "{\"method\":\"STATUS\",\"state\":\"solved\"}";

void calibration_loop();

bool calibration_setup();

void setup() {
  Serial.begin(115200);
  delay(3000);

  // // initialize visual feedback
  // pinMode(INTERNAL_LED_PIN, OUTPUT);
  calibration_setup();
}

void loop() {

  switch (state) {
    case INIT:
      if (true != false) {
        puzzle_start = false;
        mqtt_connected = false;
        scale_measure = false;
        state = WIFI_SETUP;
      }
      break;
    case WIFI_SETUP:
      if (wifi_setup() != false) {
        state = MQTT_SETUP;
      }
      break;
    case MQTT_SETUP:
      if (mqtt_setup() != false) {
        mqtt_connected = true;
        value = client.publish(mqtt_topic, Msg_inactive, true);
        state = WAIT_FOR_BEGIN;
      }
      break;
    case WAIT_FOR_BEGIN:
      if (mqtt_check() != false) {
        state = PUZZLE_START;
      }
      break;
    case PUZZLE_START:
      //if ( != false) {
        state = SCALE_CALIBRATION;
      //}
      break;
    case SCALE_CALIBRATION:
      if (calibration_setup() != false) {
        state = SCALE_GREEN;
        scale_measure = true;
      }
      break;
    case SCALE_GREEN:
      //if ( != false) {
        state = SCALE_RED;
      //}
      break;
    case SCALE_RED:
      //if ( != false) {
        //TODO verify message sent
        //TODO How long should be green
        value = client.publish(mqtt_topic, Msg_active, true);
        state = PUZZLE_SOLVED;
      //}
      break;
    case PUZZLE_SOLVED:
      //if ( != false) {
        value = client.publish(mqtt_topic, Msg_solved, true);
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
      if (mqtt_reconnect() != false) {
        state = WAIT_FOR_BEGIN;
      } else {
        mqtt_connected = false;
      }
      
      break;
    case WIFI_LOST:
      break;
  }
  
  if (mqtt_connected != false) {
    client.loop();
  }
  //delay(3000);
  Serial.printf("MQTT Return: %d\n", value);
  if (scale_measure != false) {
    calibration_loop();
  }
}

void calibration_loop() {
  if (scale.is_ready()) {
    long reading = scale.get_units(3);

    Serial.print("Value for known weight is: ");
    Serial.println(reading);
    old_reading = new_reading;
    new_reading = reading;
    if (old_reading == new_reading) {
      if (new_reading == 0) {
        digitalWrite(LED_GREEN, HIGH);
        digitalWrite(LED_RED, LOW);
        // TODO Send MQTT to LED Stripes
      }
      else {
        digitalWrite(LED_RED, HIGH);
        digitalWrite(LED_GREEN, LOW);
        // TODO Send MQTT to LED Stripes
      }
    }
    else {
      digitalWrite(LED_RED, HIGH);
        digitalWrite(LED_GREEN, LOW);
        // TODO Send MQTT to LED Stripes
    }
  } else {
   // Serial.println("HX711 not found.");
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
  // DEBUG
  Serial.println();
  Serial.printf("Connection status: %d\n", WiFi.status());
  Serial.print("Connected, IP address: ");
  Serial.println(WiFi.localIP());

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
  const char* mqtt_server = "10.0.0.2";
  int mqtt_server_port = 1883;
  
  client.setServer(mqtt_server, mqtt_server_port);
  client.setCallback(mqtt_callback);

  mqtt_setup_finished = true;
  
  return mqtt_setup_finished;
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
  const char* state = rxdoc["state"];
  int daten = rxdoc["data"];
  Serial.print("Methode: "); Serial.println(method1);
  Serial.print("State: "); Serial.println(state);
  Serial.print("Daten: "); Serial.println(daten);
  
  
  //String str_topic = String(topic);
  //str_topic.remove(0,2);
  //Serial.println(str_topic);
  
  // Feel free to add more if statements to control more GPIOs with MQTT

  // If a message is received on the topic esp32/output, you check if the message is either "on" or "off". 
  // Changes the output state according to the message
  if (String(topic).equals(mqtt_topic) != false) {
    // ODER AUS MESSAGETEMP AUSLESEN
    if (String(method1).equals("TRIGGER") != false) {
      if (String(state).equals("ON") != false) {
        Serial.print("Start Puzzle");
        puzzle_start = true;
      }
      else if (String(state).equals("OFF") != false){
        puzzle_start = false;
      }
    }

        
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
  
  Serial.println("Beginning:");
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  scale.set_scale(divider);
  scale.tare();
  //scale.set_offset(offset);
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);

  return calibration_finished;
}

/*****************************************************************************************
                                       FUNCTION INFO
NAME: 
    mqtt_reconnect
DESCRIPTION:
    

*****************************************************************************************/
bool mqtt_reconnect() {
  bool reconnect_finished = false;
  
  Serial.print("Attempting MQTT connection...");
  // Create a random client ID
  String clientId = "ESP8266Client-";
  clientId += String(random(0xffff), HEX);
  // Attempt to connect
  if (client.connect(clientId.c_str())) {
    Serial.println("connected");
    // Subscribe
    client.subscribe(mqtt_topic);
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

#include <Arduino.h>
#include "HX711.h"
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "wifi_pw.h"

typedef enum { INIT, WIFI_SETUP, MQTT_SETUP, WAIT_FOR_PUZZLE_START,
               PUZZLE_START, SCALE_CALIBRATION, SCALE_GREEN,
               SCALE_RED, PUZZLE_SOLVED, RESTART, ERROR_STATE,
               MQTT_LOST, WIFI_LOST} state_t;

// HX711 circuit wiring
const int LOADCELL_DOUT_PIN = 13;
const int LOADCELL_SCK_PIN = 12;
const int INTERNAL_LED_PIN = 16;

const int LED_GREEN = 0;
const int LED_RED = 5;

const long offset = -1693891;
const long divider = 6500;
int old_reading = 4;
int new_reading = 4;

HX711 scale;

// WiFi, Mqtt init
const char* ssid = "ubilab_wifi";
const char* pw = PASSWORD;
const char* mqtt_server = "10.0.0.2";
WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;
StaticJsonDocument<300> rxdoc;
StaticJsonDocument<100> doc;

const char* mqtt_topic = "6/puzzle/scale";
char* JSONMessageBuffer = "{\"method\":\"STATUS\",\"state\":\"active\"}";

void calibration_loop();

void calibration_setup();

state_t state = INIT;

void setup() {
  Serial.begin(115200);
  delay(3000);

  wifi_setup();

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  

  // initialize scale
  // scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  // scale.set_scale();
  // scale.tare();

  // // initialize visual feedback
  // pinMode(INTERNAL_LED_PIN, OUTPUT);
  calibration_setup();
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  //serializeJson(doc, JSONMessageBuffer, 100);
  value = client.publish(mqtt_topic, JSONMessageBuffer, true);
  delay(3000);
  Serial.printf("MQTT Return: %d\n", value);
  // check scale
  // long reading = 0;
  // if (scale.is_ready()) {
  //   reading = scale.get_units(10);
  // }
  // // map reading to weights of floppys
  // if (reading > 300000) {
  //   digitalWrite(INTERNAL_LED_PIN, LOW);
  // }
  // else {
  //   digitalWrite(INTERNAL_LED_PIN, HIGH);
  // }
  // delay(3000);
  calibration_loop();
}

void calibration_loop() {
  if (scale.is_ready()) {
    long reading = scale.get_units(3);
    Serial.printf("Connection status: %d\n", WiFi.status());
    Serial.println();
    Serial.print("Connected, IP address: ");
    Serial.println(WiFi.localIP());
    Serial.println();
    Serial.print("Value for known weight is: ");
    Serial.println(reading);
    old_reading = new_reading;
    new_reading = reading;
    if (old_reading == new_reading) {
      if (new_reading == 0) {
        digitalWrite(LED_GREEN, HIGH);
        digitalWrite(LED_RED, LOW);
      }
      else {
        digitalWrite(LED_RED, HIGH);
        digitalWrite(LED_GREEN, LOW);
      }
    }
    else {
      digitalWrite(LED_RED, HIGH);
        digitalWrite(LED_GREEN, LOW);
    }
  } else {
   // Serial.println("HX711 not found.");
    delay(200);
  }
  //delay(1000);
}



void calibration_setup() {
  Serial.println("Beginning:");
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  scale.set_scale(divider);
  scale.tare();
  //scale.set_offset(offset);
  pinMode(LED_RED, OUTPUT);
pinMode(LED_GREEN, OUTPUT);


}

void wifi_setup() {
  Serial.begin(115200);
  Serial.println();

  WiFi.begin(ssid, pw);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.printf("Connection status: %d\n", WiFi.status());
  Serial.print("Connected, IP address: ");
  Serial.println(WiFi.localIP());
}

// MQTT Send message
// void send_mqtt(char* message)

// MQTT Callback
void callback(char* topic, byte* message, unsigned int length) {
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
  /*if (String(topic) == mqtt_topic) {
    Serial.print("Changing output to ");
    if(messageTemp == "on"){
      Serial.println("on");
      
    }
    else if(messageTemp == "off"){
      Serial.println("off");
      
    }
  }
  */
}

// MQTT reconnect
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Subscribe
      client.subscribe(mqtt_topic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

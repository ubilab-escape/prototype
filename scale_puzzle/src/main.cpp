#include <Arduino.h>
#include "HX711.h"

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

void calibration_loop();

void calibration_setup();

void setup() {
  // connect to wifi & mqtt stuff

  // initialize scale
  // scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  // scale.set_scale();
  // scale.tare();

  // // initialize visual feedback
  // pinMode(INTERNAL_LED_PIN, OUTPUT);
  calibration_setup();
}

void loop() {
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
  Serial.begin(115200);
  delay(3000);
  Serial.println("Beginning:");
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  scale.set_scale(divider);
  scale.tare();
  //scale.set_offset(offset);
  pinMode(LED_RED, OUTPUT);
pinMode(LED_GREEN, OUTPUT);


}
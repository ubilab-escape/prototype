#ifndef Mqtt_test_H
#define Mqtt_test_H

#define GREEN_DELAY 1
#define RED_DELAY 2

// State variables
typedef enum {INIT, WIFI_SETUP, MQTT_SETUP, MQTT_CONNECT, PUZZLE_START,
              SCALE_CALIBRATION, WAIT_FOR_BEGIN, SCALE_GREEN, SCALE_RED,
              PUZZLE_SOLVED, WAIT_FOR_RESTART, MQTT_LOST, WIFI_LOST,
              ERROR_STATE, RESTART} state_t;

typedef enum {LED_STATE_GREEN, LED_STATE_RED, LED_STATE_ORANGE} led_state_t;


typedef struct ScalePuzzle_HandlerType_Tag {
  state_t state;
  state_t reconnect_state;
  led_state_t led_state;
  bool puzzle_start;
  bool mqtt_connected;
  bool led_control;
  bool puzzle_solved;
  bool puzzle_restart;
  uint8_t connection_check_counter;
} ScalePuzzle_HandlerType;


#endif /* Mqtt_test_H */

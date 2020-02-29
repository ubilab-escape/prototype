#ifndef Scale_puzzle_H
#define Scale_puzzle_H

#define GREEN_DELAY 1
#define RED_DELAY 2

/* State variables */
typedef enum {INIT, WIFI_SETUP, MQTT_SETUP, MQTT_CONNECT, PUZZLE_START,
              SCALE_CALIBRATION, WAIT_FOR_BEGIN, SCALE_GREEN, SCALE_RED,
              PUZZLE_SOLVED, WAIT_FOR_RESTART, MQTT_LOST, WIFI_LOST,
              ERROR_STATE, RESTART} state_t;

typedef enum {LED_STATE_GREEN, LED_STATE_RED, LED_STATE_ORANGE} led_state_t;


typedef struct ScalePuzzle_HandlerType_Tag {
  state_t state; /* current state of the state machine */
  state_t reconnect_state; /* last state before MQTT connection was lost */
  led_state_t led_state; /* current LED color state */
  bool puzzle_start; /* shall the puzzle be started? */
  bool mqtt_connected; /* is MQTT connected? */
  bool led_control; /* may the safe LEDs be controlled? */
  bool puzzle_solved; /* is the puzzle solved? */
  bool puzzle_restart; /* shall the puzzle be restarted? */
  uint8_t connection_check_counter; /* shall a connection check be performed?*/
  uint8_t scale_failure; /* is the connection to the scale ok? */
} ScalePuzzle_HandlerType;


#endif /* Scale_Puzzle_H */

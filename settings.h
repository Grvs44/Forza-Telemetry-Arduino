// Software version
#define VERSION \
  "v1.0.0"
// Maximum size for received UDP packet
#define BUFFER_SIZE 400
// Milliseconds to wait at the start of each loop
#define LOOP_DELAY 16
// Pin number for LED to show when packet is being processed
// #define PACKET_LED 13

// MAC address
#define MAC_ADDRESS \
  { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED }
// Port to receive UDP packets
#define PORT 8888

// RPM LED pin numbers
#define RPM_LEDS \
  { 32, 33, 34, 35, 36, 37, 38, 39, 40, 41 }
// Time in ms between each update in RPM step
#define STEP_PERIOD 500
// Display RPM on LCD
// #define DISPLAY_RPM

// Gravitational field strength (g) ms^-2
#define GFS 9.81
// Board type (UNO or MEGA)
#define BOARD MEGA
// Acceleration (G-force) direction LED pin numbers
// #define GFORCE_LEDS {right, left, up, down, forward, backward}
#define GFORCE_LEDS \
  { 24, 25, 26, 27, 28, 29 }
// Acceleration thresholds (turn direction LEDs when acceleration larger than this value)
// {right/left, up/down, forward/backward}
#define ACC_THRESHOLDS \
  { 0.5, 1.0, 0.5 }

// Include header to optionally override default settings
#include "_conf.h"

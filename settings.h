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
// #define RPM_LEDS { 2, 3, 5, 6, 7, 8 }
// Time in ms between each update in RPM step
#define STEP_PERIOD 500

// Gravitational field strength (g) ms^-2
#define GFS 9.81
// Acceleration (G-force) direction LED pin numbers
// #define GFORCE_LEDS {right, left, up, down, forward, backward}

// Include header to optionally override default settings
#include "_conf.h"

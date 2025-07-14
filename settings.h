// Maximum size for received UDP packet
#define BUFFER_SIZE 400
// Ethernet device type
#define ETHERNET_INIT 10
// IPv4 address
#define IP_ADDRESS 192, 168, 0, 171
// Milliseconds to wait at the start of each loop
#define LOOP_DELAY 16
// MAC address
#define MAC_ADDRESS \
  { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED }
// Port to receive UDP packets
#define PORT 8888

// RPM LED pin numbers
#define RPM_LEDS \
  { 2, 3, 5, 6, 7, 8 }
// Number of RPM LEDs (length of above array) minus 1
#define RPM_LEDS_MAX 5
// Time in ms between each update in RPM step
#define STEP_PERIOD 500

#include <LedControl.h>

// Arduino board types
#define UNO 1
#define MEGA 2

#define TENS 3
#define UNITS 2
#define TENTHS 1
#define HUNDREDTHS 0

#define MAX_ROWS 7     // 8  // Row 8 is not used so skipped for performance
#define MAX_COLUMNS 7  // 8  // Column 8 is not used so skipped for performance

#define MAX_DEVICES 4

// Define pins for the Arduino board
#if BOARD == UNO
#define CS_PIN 10
#define CLK_PIN 13
#define DIN_PIN 11
#elif BOARD == MEGA
#define CS_PIN 47
#define DIN_PIN 46
#define CLK_PIN 9
#else
#error BOARD not supported (must be UNO or MEGA)
#endif

LedControl lc(DIN_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);

byte displayPixels[15][8] = {
  // 0
  { B00000000,
    B00000000,
    B01111100,
    B10000010,
    B10000010,
    B10000010,
    B01111100,
    B00000000 },

  // 1
  { B00000000,
    B00000000,
    B00000000,
    B11111110,
    B10000000,
    B01000000,
    B00000000,
    B00000000 },

  // 2
  { B00000000,
    B00000000,
    B01100010,
    B10010010,
    B10010010,
    B10001010,
    B01000110,
    B00000000 },

  // 3
  { B00000000,
    B00000000,
    B01101100,
    B10010010,
    B10010010,
    B10000010,
    B01000100,
    B00000000 },

  // 4
  { B00000000,
    B00000000,
    B11111110,
    B00010000,
    B00010000,
    B11110000,
    B00000000 },

  // 5
  { B00000000,
    B00000000,
    B10001100,
    B10010010,
    B10010010,
    B10010010,
    B11110010,
    B00000000 },

  // 6
  { B00000000,
    B00000000,
    B00001100,
    B10010010,
    B10010010,
    B10010010,
    B01111100,
    B00000000 },

  // 7
  { B00000000,
    B00000000,
    B01100000,
    B10011000,
    B10000110,
    B10000000,
    B10000000,
    B00000000 },

  // 8
  { B00000000,
    B00000000,
    B01101100,
    B10010010,
    B10010010,
    B10010010,
    B01101100,
    B00000000 },

  // 9
  { B00000000,
    B00000000,
    B01111100,
    B10010010,
    B10010010,
    B10010010,
    B01100000,
    B00000000 },

  // !
  { B00000000,
    B00000000,
    B00000000,
    B11110110,
    B11110110,
    B00000000,
    B00000000,
    B00000000 },

  // X
  { B10000001,
    B01000010,
    B00100100,
    B00011000,
    B00011000,
    B00100100,
    B01000010,
    B10000001 },

  // b
  { B00000000,
    B00000000,
    B00001100,
    B00010010,
    B00010010,
    B01111110,
    B00000000,
    B00000000 },

  // o
  { B00000000,
    B00000000,
    B00011100,
    B00100010,
    B00100010,
    B00011100,
    B00000000,
    B00000000 },

  // x
  { B00000000,
    B00100010,
    B00010100,
    B00001000,
    B00010100,
    B00100010,
    B00000000,
    B00000000 },
};

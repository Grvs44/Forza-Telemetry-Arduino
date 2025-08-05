// UDP server adapted from https://docs.arduino.cc/tutorials/ethernet-shield-rev2/udp-send-receive-string/
#include <Ethernet.h>
#include <EthernetUdp.h>
#include <LiquidCrystal_I2C.h>
#include "structs.cpp"
#include "settings.h"
#ifdef GFORCE_LEDS
#include "led_matrix.h"
#endif

// Round acceleration value
#define roundAcc(x) roundf(x * 10.0) / 10.0

byte mac[] = MAC_ADDRESS;
char packetBuffer[BUFFER_SIZE];
EthernetUDP Udp;

LiquidCrystal_I2C lcd(0x27, 20, 4);

typedef enum {
  MENU,
  RACE,
  WAITING,
} State;

State state = WAITING;
#ifdef RPM_LEDS
byte rpmLeds[] = RPM_LEDS;
#endif
#ifdef GFORCE_LEDS
byte gforceLeds[6] = GFORCE_LEDS;
#endif

void setup() {
#ifdef RPM_LEDS
  setOutputPins(rpmLeds, sizeof(rpmLeds));
#endif
#ifdef GFORCE_LEDS
  setOutputPins(gforceLeds, sizeof(gforceLeds));
#endif
#ifdef PACKET_LED
  pinMode(PACKET_LED, OUTPUT);
#endif

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Connecting...");

#ifdef GFORCE_LEDS
  setupMatrix();
#endif

  Ethernet.init();
  while (Ethernet.begin(mac) == 0) {
    findEthernetIssue();
  }

  Udp.begin(PORT);
  lcd.clear();
  lcd.setCursor(2, 0);
  lcd.print("Waiting for data");
  lcd.setCursor(0, 1);
  lcd.print("IP");
  lcd.setCursor(5, 1);
  lcd.print(Ethernet.localIP());
  lcd.setCursor(0, 2);
  lcd.print("Port ");
  lcd.print(PORT);
  lcd.setCursor(0, 3);
  lcd.print(VERSION);
#ifdef GFORCE_LEDS
  printMatrixDigit(THOUSANDS, PORT / 1000);
  printMatrixDigit(HUNDREDS, PORT / 100 % 10);
  printMatrixDigit(TENS, PORT / 10 % 10);
  printMatrixDigit(UNITS, PORT % 10);
#endif
}

void setOutputPins(uint8_t* pins, unsigned int size) {
  while (size > 0) {
    pinMode(pins[--size], OUTPUT);
  }
}

void findEthernetIssue() {
  if (Ethernet.hardwareStatus() == EthernetNoHardware) {
    lcd.clear();
    lcd.setCursor(6, 0);
    lcd.print("Ethernet");
    lcd.setCursor(7, 1);
    lcd.print("shield");
    lcd.setCursor(4, 2);
    lcd.print("disconnected");
    while (true) {
      delay(-1);
    }
  }
  if (Ethernet.linkStatus() == LinkOFF) {
    lcd.clear();
    lcd.setCursor(6, 0);
    lcd.print("Ethernet");
    lcd.setCursor(7, 1);
    lcd.print("cable");
    lcd.setCursor(4, 2);
    lcd.print("disconnected");
    while (Ethernet.linkStatus() == LinkOFF) delay(10000);
  }
}

void loop() {
  static int lastPacketSize = 0;

  delay(LOOP_DELAY);
  int packetSize = Udp.parsePacket();
  if (packetSize == 0) {
#if defined(RPM_LEDS) || defined(GFORCE_LEDS)
    if (state != RACE) stepLeds();
#endif
    return;
  }
#ifdef PACKET_LED
  digitalWrite(PACKET_LED, HIGH);
#endif
  if (packetSize != lastPacketSize) {
    lastPacketSize = packetSize;
    lcd.setCursor(18, 3);
    lcd.print(packetSizeChar(packetSize));
  }
  Udp.read(packetBuffer, BUFFER_SIZE);

  State newState = (State)(((Sled*)packetBuffer)->IsRaceOn);
  if (newState != state) {
    if (state == WAITING) {
      lcd.clear();
      lcd.setCursor(0, 1);
      lcd.print("Best lap:");
      lcd.setCursor(0, 2);
      lcd.print("Last lap:");
    }
    state = newState;
    lcd.setCursor(0, 0);
    if (state) {
#ifdef DISPLAY_RPM
      lcd.print("RPM:                ");
#else
      lcd.setCursor(9, 0);
      lcd.print("race");
#endif
    } else {
      lcd.print("      In menu       ");
    }
  }

  switch (packetSize) {
    case sizeof(Sled):
      {
        Sled* packet = (Sled*)packetBuffer;
        renderSled(packet);
        break;
      }
    case sizeof(Dash7):
    case sizeof(DashM):
      {
        Dash7* packet = (Dash7*)packetBuffer;
        renderSled(&(packet->sled));
        renderDash(&(packet->dash));
        break;
      }
    case sizeof(DashH):
      {
        DashH* packet = (DashH*)packetBuffer;
        renderSled(&(packet->sled));
        renderDash(&(packet->dash));
        break;
      }
    default:
      lcd.setCursor(5, 0);
      lcd.print("?");
      lcd.print(packetSize);
  }
#ifdef PACKET_LED
  digitalWrite(PACKET_LED, LOW);
#endif
}

char packetSizeChar(int packetSize) {
  switch (packetSize) {
    case sizeof(Sled): return 'S';
    case sizeof(Dash7): return 'D';
    case sizeof(DashH): return 'H';
    case sizeof(DashM): return 'M';
    default: return '?';
  }
}

void renderSled(Sled* packet) {
  if (state != RACE) {
#if defined(RPM_LEDS) || defined(GFORCE_LEDS)
    stepLeds();
#endif
#ifdef GFORCE_LEDS
    displayNoGForce();
#endif
    return;
  };
#ifdef RPM_LEDS
  updateRpmLeds(packet);
#endif
#ifdef DISPLAY_RPM
  displayRpm(packet);
#endif
#ifdef GFORCE_LEDS
  renderGForce(packet);
#endif
}

#ifdef DISPLAY_RPM
void displayRpm(Sled* packet) {
  lcd.setCursor(5, 0);
  char buffer[8];
  dtostrf(packet->CurrentEngineRpm, 5, 0, buffer);
  lcd.print(buffer);
}
#endif

#ifdef GFORCE_LEDS
void renderGForce(Sled* packet) {
  static float a[] = { -1.0, -1.0, -1.0 };

  if (
    packet->AccelerationX == a[0]
    && packet->AccelerationY == a[1]
    && packet->AccelerationZ == a[2]) {
    return;
  }

  a[0] = roundAcc(packet->AccelerationX);
  a[1] = roundAcc(packet->AccelerationY);
  a[2] = roundAcc(packet->AccelerationZ);
  float size = sqrtf(sq(a[0]) + sq(a[1]) + sq(a[2])) / GFS;
  displayGForce(size);
#ifdef GFORCE_LEDS
  for (int i = 2; i >= 0; i--) {
    digitalWrite(gforceLeds[i * 2], a[i] > 0);
    digitalWrite(gforceLeds[(i * 2) + 1], a[i] < 0);
  }
#endif
}
#endif

void renderDash(Dash* dash) {
  renderBestLap(dash);
  renderLastLap(dash);
}

void renderBestLap(Dash* packet) {
  static float bestLap = -1.0;
  if (packet->BestLap == bestLap) return;
  bestLap = packet->BestLap;
  lcd.setCursor(10, 1);
  printLap(bestLap);
}

void renderLastLap(Dash* packet) {
  static float lastLap = -1.0;
  if (packet->LastLap == lastLap) return;
  lastLap = packet->LastLap;
  lcd.setCursor(10, 2);
  printLap(lastLap);
}

void printLap(float lap) {
  if (lap == 0.0) {
    lcd.print("-:--.---");
    return;
  }
  int mins = (int)lap / 60;
  char secs[6];
  dtostrf(lap - (mins * 60), 6, 3, secs);
  if (secs[0] == ' ') secs[0] = '0';
  lcd.print(mins);
  lcd.print(':');
  lcd.print(secs);
}

#ifdef RPM_LEDS
void updateRpmLeds(Sled* packet) {
  float value = packet->CurrentEngineRpm - packet->EngineIdleRpm;
  unsigned int ledsOn;
  if (value > 0.0) {
    unsigned int increment = (packet->EngineMaxRpm - packet->EngineIdleRpm) / sizeof(rpmLeds);
    ledsOn = ((unsigned int)value) / (increment - 1);
  } else {
    ledsOn = 0;
  }
  unsigned int i = 0;
  while (i <= min(ledsOn, sizeof(rpmLeds) - 1)) {
    digitalWrite(rpmLeds[i++], HIGH);
  }
  while (i < sizeof(rpmLeds)) {
    digitalWrite(rpmLeds[i++], LOW);
  }
}
#endif

#if defined(RPM_LEDS) || defined(GFORCE_LEDS)
void stepLeds() {
  static unsigned long lastUpdate = 0;

  if (millis() < lastUpdate + STEP_PERIOD) return;
#ifdef RPM_LEDS
  stepRpmLeds();
#endif
#ifdef GFORCE_LEDS
  stepGForceLeds();
#endif
  lastUpdate = millis();
}
#endif

#ifdef RPM_LEDS
void stepRpmLeds() {
  static unsigned int position = 1;
  static bool direction = false;

  digitalWrite(rpmLeds[position], LOW);
  digitalWrite(rpmLeds[sizeof(rpmLeds) - 1 - position], LOW);

  if (position == 0 || position == (sizeof(rpmLeds) - 1) / 2) {
    direction = !direction;
  }

  if (direction) {  // left half moving right
    position++;
  } else {  // left half moving left
    position--;
  }
  digitalWrite(rpmLeds[position], HIGH);
  digitalWrite(rpmLeds[sizeof(rpmLeds) - 1 - position], HIGH);
}
#endif

#ifdef GFORCE_LEDS
void stepGForceLeds() {
  static unsigned int position = sizeof(gforceLeds) - 1;

  digitalWrite(gforceLeds[position], LOW);
  position = (position + 1) % sizeof(gforceLeds);
  digitalWrite(gforceLeds[position], HIGH);
}

void setupMatrix() {
  // Wake up the matrices
  lc.shutdown(UNITS, false);
  lc.shutdown(TENS, false);
  lc.shutdown(HUNDREDS, false);
  lc.shutdown(THOUSANDS, false);

  // Set the brightness to a medium level
  lc.setIntensity(UNITS, 0);
  lc.setIntensity(TENS, 0);
  lc.setIntensity(HUNDREDS, 0);
  lc.setIntensity(THOUSANDS, 0);

  // Clear the displays
  lc.clearDisplay(UNITS);
  lc.clearDisplay(TENS);
  lc.clearDisplay(HUNDREDS);
  lc.clearDisplay(THOUSANDS);
}

void displayGForce(float value) {
  static float currentValue = -1.0;
  // {hundredths, tenths, units, tens}
  static int currentDisplay[] = { 0, 0, 0, 0 };

  if (value == currentValue) return;

  int newDisplay[] = {
    int(value * 100) % 10,
    int(value * 10) % 10,
    int(value) % 10,
    int(value / 10)
  };

  for (int i = 3; i >= 0; i--) {
    if (newDisplay[i] == currentDisplay[i]) continue;
    currentDisplay[i] = newDisplay[i];
    if (i == THOUSANDS && newDisplay[i] == 0) {
      lc.clearDisplay(THOUSANDS);
      continue;
    } else {
      printMatrixDigit(i, newDisplay[i]);
    }
    if (i == HUNDREDS) {
      // Add decimal point
      lc.setLed(HUNDREDS, 0, 6, 1);
    }
  }
}

void printMatrixDigit(int display, int number) {
  for (int row = 2; row < MAX_ROWS; row++)  // Skip rows 0 and 1 for performance
  {
    for (int column = 0; column < MAX_COLUMNS; column++) {
      lc.setLed(display, row, column, displayPixels[number][row][column]);
    }
  }
}

void displayNoGForce() {
  printMatrixDigit(0, 10);
  printMatrixDigit(1, 10);
  printMatrixDigit(2, 10);
  printMatrixDigit(3, 10);
}
#endif

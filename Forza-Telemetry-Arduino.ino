// UDP server adapted from https://docs.arduino.cc/tutorials/ethernet-shield-rev2/udp-send-receive-string/
#include <Ethernet.h>
#include <EthernetUdp.h>
#include <LiquidCrystal_I2C.h>
#include "structs.cpp"
#include "led_matrix.h"
#include "settings.h"

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
byte rpmLeds[] = RPM_LEDS;
#ifdef GFORCE_LEDS
byte gforceLeds[6] = GFORCE_LEDS;
#endif

void setup() {
  for (int i = RPM_LEDS_MAX; i >= 0; i--) {
    pinMode(rpmLeds[i], OUTPUT);
  }

  for (int j = GFORCE_LEDS_MAX; j >=0; j-- ){
    pinMode(gforceLeds[j], OUTPUT);
  }

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Connecting...");

  setupMatrix();

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
  _printNumber(float(PORT) / 100.0);
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
    if (state != RACE) stepRpmLeds();
    return;
  } else if (packetSize != lastPacketSize) {
    lastPacketSize = packetSize;
    lcd.setCursor(19, 3);
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
      lcd.print("RPM:                ");
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
    stepRpmLeds();
    _printNumber(0.0);
    return;
  };
  renderRpm(packet);
  renderGForce(packet);
}

void renderRpm(Sled* packet) {
  updateRpmLeds(packet);
  lcd.setCursor(5, 0);
  char buffer[8];
  dtostrf(packet->CurrentEngineRpm, 5, 0, buffer);
  lcd.print(buffer);
}

void renderGForce(Sled* packet) {
  static float x = -1.0;
  static float y = -1.0;
  static float z = -1.0;

  if (
    packet->AccelerationX == x
    && packet->AccelerationY == y
    && packet->AccelerationZ == z) {
    return;
  }

  x = packet->AccelerationX;
  y = packet->AccelerationY;
  z = packet->AccelerationZ;
  float size = sqrtf(sq(x) + sq(y) + sq(z)) / GFS;
  _printNumber(size);
#ifdef GFORCE_LEDS
  digitalWrite(gforceLeds[0], x > 0);
  digitalWrite(gforceLeds[1], x < 0);
  digitalWrite(gforceLeds[2], y > 0);
  digitalWrite(gforceLeds[3], y < 0);
  digitalWrite(gforceLeds[4], z > 0);
  digitalWrite(gforceLeds[5], z < 0);
#endif
}

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

void updateRpmLeds(Sled* packet) {
  float value = packet->CurrentEngineRpm - packet->EngineIdleRpm;
  int increment = (packet->EngineMaxRpm - packet->EngineIdleRpm) / (RPM_LEDS_MAX + 1);
  int ledsOn = ((int)value) / (increment - 1);
  int i = 0;
  while (i <= min(ledsOn, RPM_LEDS_MAX)) {
    digitalWrite(rpmLeds[i++], HIGH);
  }
  while (i <= RPM_LEDS_MAX) {
    digitalWrite(rpmLeds[i++], LOW);
  }
}

void stepRpmLeds() {
  static int position = 1;
  static bool direction = false;
  static unsigned long lastUpdate = 0;

  if (millis() < lastUpdate + STEP_PERIOD) return;
  digitalWrite(rpmLeds[position], LOW);
  digitalWrite(rpmLeds[RPM_LEDS_MAX - position], LOW);

  if (position == 0 || position == RPM_LEDS_MAX / 2) {
    direction = !direction;
  }

  if (direction) {  // left half moving right
    position++;
  } else {  // left half moving left
    position--;
  }
  digitalWrite(rpmLeds[position], HIGH);
  digitalWrite(rpmLeds[RPM_LEDS_MAX - position], HIGH);
  lastUpdate = millis();
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

void _printNumber(float _newNumber) {
  int _thousands = 0;
  int _hundreds = 0;
  int _tens = 0;
  int _units = 0;

  _newNumber = _newNumber * 100;

  _thousands = int(_newNumber / 1000);
  _hundreds = int(_newNumber / 100) % 10;
  _tens = int(_newNumber / 10) % 10;
  _units = int(_newNumber) % 10;

  // Only run this routine if the number has changed
  if (_newNumber != _oldNumber) {
    // Only display THOUSANDS digit if it has changed
    if (_thousands != _oldThousands) {
      // Mask _thousands if it is zero
      if (_thousands == 0) {
        lc.clearDisplay(THOUSANDS);
      } else {
        _printDigit(THOUSANDS, _thousands);
      }
    }

    // Only display the HUNDREDS if it has changed
    if (_hundreds != _oldHundreds) {
      // Mask _hundreds if it is zero
      //if (( _thousands == 0 ) && ( _hundreds == 0 ))
      //{
      //  lc.clearDisplay(HUNDREDS);
      //}
      //else
      {
        _printDigit(HUNDREDS, _hundreds);
      }
    }
    // Only display TENS digit if it has changed
    if (_tens != _oldTens) {
      // Mask _tens if it is zero
      //if (( _hundreds == 0 ) && ( _tens == 0 ))
      //{
      //  lc.clearDisplay(TENS);
      //}
      //else
      {
        _printDigit(TENS, _tens);
      }
    }
    // Only display the UNITS digit if it has changed
    // The UNITS digit will always be displayed, even if zero
    if (_units != _oldUnits) {
      _printDigit(UNITS, _units);
    }
  } else {
    _printCode();
  }
  _oldThousands = _thousands;
  _oldHundreds = _hundreds;
  _oldTens = _tens;
  _oldUnits = _units;
}

void _printDigit(int _display, int _number) {
  for (int _row = 2; _row < MAX_ROWS; _row++)  // Skip rows 0 and 1 for performance
  {
    for (int _column = 0; _column < MAX_COLUMNS; _column++) {
      lc.setLed(_display, _row, _column, _displayPixels[_number][_row][_column]);
    }
  }
  // Add the decimal place to the HUNDREDS matrix
  if (_display == HUNDREDS) {
    lc.setLed(_display, 0, 6, 1);
  }
}

void _printCode() {
  _printDigit(0, 10);
  _printDigit(1, 10);
  _printDigit(2, 10);
  _printDigit(3, 10);
}

void _testdisplay() {
  for (int _number = 0; _number < 10; _number++) {
    for (int _row = 0; _row < 8; _row++) {
      for (int _column = 0; _column < 8; _column++) {
        lc.setLed(1, _row, _column, _displayPixels[_number][_row][_column]);
      }
    }
    delay(50);
  }
}

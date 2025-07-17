// UDP server adapted from https://docs.arduino.cc/tutorials/ethernet-shield-rev2/udp-send-receive-string/
#include <Ethernet.h>
#include <EthernetUdp.h>
#include <LiquidCrystal_I2C.h>
#include "structs.cpp"
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
float bestLap = -1.0;
float lastLap = -1.0;

byte rpmLeds[] = RPM_LEDS;

void setup() {
  for (int i = RPM_LEDS_MAX; i >= 0; i--) {
    pinMode(rpmLeds[i], OUTPUT);
  }

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Connecting...");

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
  lcd.setCursor(4, 3);
  lcd.print(size);
}

void renderDash(Dash* dash) {
  renderBestLap(dash);
  renderLastLap(dash);
}

void renderBestLap(Dash* packet) {
  if (packet->BestLap == bestLap) return;
  bestLap = packet->BestLap;
  lcd.setCursor(10, 1);
  printLap(bestLap);
}

void renderLastLap(Dash* packet) {
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

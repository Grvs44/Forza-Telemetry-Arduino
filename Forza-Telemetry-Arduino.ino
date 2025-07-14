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
float bestLap = 0.0;
float lastLap = 0.0;

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
  lcd.setCursor(0, 0);
  lcd.print("Waiting for data");
  lcd.setCursor(0, 1);
  lcd.print(Ethernet.localIP());
  lcd.print(":");
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
  delay(LOOP_DELAY);
  int packetSize = Udp.parsePacket();
  if (packetSize == 0) {
    if (state != RACE) stepRpmLeds();
    return;
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
  if (state == RACE) showRace(packetSize);
}

void showRace(int packetSize) {
  switch (packetSize) {
    case sizeof(Sled):
      {
        Sled* packet = (Sled*)packetBuffer;
        renderRpm(packet);
        break;
      }
    case sizeof(Dash7):
    case sizeof(DashM):
      {
        Dash7* packet = (Dash7*)packetBuffer;
        renderRpm(&(packet->sled));
        renderDash(&(packet->dash));
        break;
      }
    case sizeof(DashH):
      {
        DashH* packet = (DashH*)packetBuffer;
        renderRpm(&(packet->sled));
        renderDash(&(packet->dash));
        break;
      }
    default:
      lcd.setCursor(5, 0);
      lcd.print("?");
      lcd.print(packetSize);
  }
}

void renderRpm(Sled* packet) {
  lcd.setCursor(5, 0);
  char buffer[8];
  dtostrf(packet->CurrentEngineRpm, 5, 0, buffer);
  lcd.print(buffer);
}

void renderDash(Dash* dash) {
  renderBestLap(dash);
  renderLastLap(dash);
}

void renderBestLap(Dash* packet) {
  if (packet->BestLap == bestLap) return;
  bestLap = packet->BestLap;
  lcd.setCursor(10, 1);
  lcd.print(bestLap);
}

void renderLastLap(Dash* packet) {
  if (packet->LastLap == lastLap) return;
  lastLap = packet->LastLap;
  lcd.setCursor(10, 2);
  lcd.print(lastLap);
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

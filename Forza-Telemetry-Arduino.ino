// UDP server adapted from https://docs.arduino.cc/tutorials/ethernet-shield-rev2/udp-send-receive-string/
#include <Ethernet.h>
#include <EthernetUdp.h>
#include <LiquidCrystal_I2C.h>
#include "structs.cpp"
#define BUFFER_SIZE 400

byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};
IPAddress ip(192, 168, 0, 171);
unsigned int port = 8888;
char packetBuffer[BUFFER_SIZE];
EthernetUDP Udp;

LiquidCrystal_I2C lcd(0x27, 20, 4);

typedef enum {
  MENU,
  RACE,
  WAITING,
} State;

State state = WAITING;

void setup() {
  Ethernet.init(10);
  Ethernet.begin(mac, ip);

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Connecting...");

  if (Ethernet.hardwareStatus() == EthernetNoHardware) {
    lcd.clear();
    lcd.setCursor(6, 0);
    lcd.print("Ethernet");
    lcd.setCursor(8, 1);
    lcd.print("shield");
    lcd.setCursor(4, 2);
    lcd.print("disconnected");
    while (true) {
      delay(1);
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

  Udp.begin(port);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Waiting for data");
  lcd.setCursor(0, 1);
  lcd.print(sizeof(Sled));
  lcd.print(" ");
  lcd.print(sizeof(Dash));
}

void loop() {
  delay(10);
  int packetSize = Udp.parsePacket();
  if (packetSize == 0) return;
  Udp.read(packetBuffer, BUFFER_SIZE);

  State newState = (State)(((Sled*)packetBuffer)->IsRaceOn);
  if (newState != state) {
    if (state == WAITING) {
      lcd.clear();
    }
    state = newState;
    lcd.setCursor(0, 0);
    if (state) {
      lcd.print("RPM:                ");
    } else {
      lcd.print("      In menu       ");
    }
  }

  if (packetSize == sizeof(Sled)) {
    Sled* packet = (Sled*)packetBuffer;
    renderRpm(packet->CurrentEngineRpm);
  } else if (packetSize == sizeof(Dash)) {
    Dash* packet = (Dash*)packetBuffer;
    renderRpm(packet->CurrentEngineRpm);
  } else {
    lcd.setCursor(5, 0);
    lcd.print("?");
  }
}

void renderRpm(float rpm) {
  if (rpm == 0.0) return;
  lcd.setCursor(5, 0);
  char buffer[8];
  dtostrf(rpm, 5, 0, buffer);
  lcd.print(buffer);
}

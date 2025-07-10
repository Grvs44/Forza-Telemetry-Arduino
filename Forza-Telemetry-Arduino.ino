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
  lcd.setCursor(0,1);
  lcd.print(sizeof(Sled));
  lcd.print(" ");
  lcd.print(sizeof(Dash));
}

void loop() {
  delay(10);
  int packetSize = Udp.parsePacket();
  if (packetSize == 0) return;
  lcd.clear();
  lcd.setCursor(0, 0);
  switch (packetSize) {
    case sizeof(Sled):
      lcd.print("Sled packet");
      break;
    case sizeof(Dash):
      lcd.print("Dash packet");
      break;
    default:
      lcd.print("Unknown packet");
      return;
  }

  lcd.setCursor(0, 2);
  Udp.read(packetBuffer, UDP_TX_PACKET_MAX_SIZE);
  Dash* packet = (Dash*)packetBuffer;
  lcd.print("RPM: ");
  lcd.print(round(packet->CurrentEngineRpm));
}

// UDP server adapted from https://docs.arduino.cc/tutorials/ethernet-shield-rev2/udp-send-receive-string/
#include <Ethernet.h>
#include <EthernetUdp.h>
#include <LiquidCrystal_I2C.h>

byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};
IPAddress ip(192, 168, 0, 171);
unsigned int port = 8888;
char packetBuffer[UDP_TX_PACKET_MAX_SIZE];
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
  lcd.println("Waiting for data");
}

void loop() {
  int packetSize = Udp.parsePacket();
  if (packetSize) {
    lcd.clear();
    lcd.print("Received packet of size ");
    lcd.print(packetSize);
    lcd.setCursor(0, 1);
    lcd.print("From ");
    IPAddress remote = Udp.remoteIP();
    for (int i = 0; i < 4; i++) {
      lcd.print(remote[i], DEC);
      if (i < 3) {
        lcd.print(".");
      }
    }
    lcd.print(", port ");
    lcd.println(Udp.remotePort());

    // read the packet into packetBufffer
    lcd.setCursor(0, 3);
    Udp.read(packetBuffer, UDP_TX_PACKET_MAX_SIZE);
    lcd.print("Contents: ");
    lcd.println(packetBuffer);
  }
  delay(10);
}

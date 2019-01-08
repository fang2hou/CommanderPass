#include <SoftwareSerial.h>
SoftwareSerial XBee(2, 3); // RX, TX

void setup() {
  XBee.begin(9600);
  Serial.begin(9600);
}

void loop() {
  if (XBee.available()) {
    String c = XBee.readStringUntil('\n');
    Serial.print(c);
  }

  if (Serial.available()) {
    String c = Serial.readStringUntil('\n');
    XBee.print(c);
  }
}

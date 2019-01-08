#include <SoftwareSerial.h>
SoftwareSerial XBee(2, 3);
const String MODEL = "AEL0106";

String bufferString;
String token;
String pass;

void setup() {
  Serial.begin(9600);
  XBee.begin(9600);
}

void loop() {
  if(Serial.available()) {
    bufferString = Serial.readStringUntil('\n');
    handleData();
  }
  if (XBee.available()) {
    bufferString = XBee.readStringUntil('\n');
    handleData();
  }
}

void handleData() {
  // Client Command
  if (bufferString.startsWith("C")) {
    if (bufferString == "CWaitSender") {
      Serial.print("ASenderReady\n");
    } else if (bufferString == "CWaitUser") {
      XBee.print("AWaitUser\n");
    } else if (bufferString.startsWith("C#token#")) {
      Serial.print("ATokenGot\n");
      const int leftSide = String("C#token#").length();
      token = bufferString.substring(leftSide);
      XBee.print("AStartInput\n");
    }
  }

  // Arduino Command
  if (bufferString.startsWith("A")) {
    if (bufferString == "AUserReady") {
      Serial.print("AUserReadyViaSender\n");
    } else if (bufferString.startsWith("A#pass#")) {
      XBee.print("APassGot\n");
      const int leftSide = String("A#pass#").length();
      pass = bufferString.substring(leftSide);
      encrypt();
    }
  }
}

void encrypt() {
  Serial.print("A#auth#" + MODEL + token + pass);
  Serial.print("\n");
}

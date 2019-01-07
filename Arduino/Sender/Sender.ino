const String MODEL = "AEL0106";

String bufferString;
String token;
String pass;

void setup() {
  Serial.begin(9600);
}

void loop() {
  if(Serial.available()) {
    bufferString = Serial.readStringUntil('\n');
    handleData();
  }
}

void handleData() {
  if (bufferString.startsWith("C")) {
    // Client Command
    if (bufferString == "CWaitSender") {
      // Sender Arduino is ready!
      Serial.print("ASenderReady\n");
    } else if (bufferString == "CWaitUser") {
      // Xbee.print("AWaitUser\n");
      
      // DEBUG:
      Serial.print("AUserReadyViaSender\n");
    } else if (bufferString.startsWith("C#token#")) {
      Serial.print("ATokenGot\n");
      
      // Xbee.print("AStartInput\n");
      
      const int leftSide = String("C#token#").length();
      token = bufferString.substring(leftSide);
      
      // DEBUG:
      pass = "1234";
      encrypt();
    }
  }
  
  if (bufferString.startsWith("A")) {
    // Arduino Command
    if (bufferString == "AUserReady") {
      Serial.print("AUserReadyViaSender\n");
    } else if (bufferString.startsWith("A#pass#")) {
      // Xbee.print("APassGot\n");
      pass = "1234";
      encrypt();
    }
  }
}

void encrypt() {
  delay(1000);

  Serial.print("A#auth#" + MODEL + token + pass);
  Serial.print("\n");
}

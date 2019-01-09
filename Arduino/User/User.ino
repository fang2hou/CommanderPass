#include <SoftwareSerial.h>
#include <Adafruit_GFX.h> // Core graphics library
#include <Adafruit_TFTLCD.h> // Hardware-specific library
#include <TouchScreen.h>

#if defined(__SAM3X8E__)
#undef __FlashStringHelper::F(string_literal)
#define F(string_literal) string_literal
#endif

#ifndef USE_ADAFRUIT_SHIELD_PINOUT
#error \
  "This sketch is intended for use with the TFT LCD Shield. Make sure that USE_ADAFRUIT_SHIELD_PINOUT is #defined in the Adafruit_TFTLCD.h library file."
#endif

// These are the pins for the shield!
#define YP A1 // must be an analog pin, use "An" notation!
#define XM A2 // must be an analog pin, use "An" notation!
#define YM 7 // can be a digital pin
#define XP 6 // can be a digital pin

#ifdef __SAM3X8E__
#define TS_MINX 125
#define TS_MINY 170
#define TS_MAXX 880
#define TS_MAXY 940
#else
#define TS_MINX 150
#define TS_MINY 120
#define TS_MAXX 920
#define TS_MAXY 940
#endif

// For better pressure precision, we need to know the resistance
// between X+ and X- Use any multimeter to read it
// For the one we're using, its 300 ohms across the X plate
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);

#define LCD_CS A3
#define LCD_CD A2
#define LCD_WR A1
#define LCD_RD A0

// Assign human-readable names to some common 16-bit color values:
#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF

#define BUTTONSIZE 72
#define MARGIN 6

#define MINPRESSURE 10
#define MAXPRESSURE 1000

typedef struct {
  int x, y, number;
} position;

Adafruit_TFTLCD tft;
SoftwareSerial XBee(2, 3);

bool isInputMode = false;
bool isAuthFailed = false;
unsigned long lastTouchedTime = 0;


String bufferString = "";
String passwordString = "";

position positions[9] = {
  { 42, 42,  0},
  {120, 42,  0},
  {198, 42,  0},
  { 42, 123, 0},
  {120, 123, 0},
  {198, 123, 0},
  { 42, 204, 0},
  {120, 204, 0},
  {198, 204, 0},
};

void setup(){
  // Ready to send data
  XBee.begin(9600);
  Serial.begin(9600);

  // Initialize TFTLCD
  tft.reset();
  tft.begin(tft.readID());
  pinMode(13, OUTPUT);

  // UI Rendering
  tft.fillScreen(WHITE);
  renderMessage("Waiting...", RED);
  renderTitle();
}

void loop() {
  if (isAuthFailed) {
    
  } else if (isInputMode) {
    detectTouch();
    if (passwordString.length() >= 4) {
      XBee.print("A#pass#" + passwordString + "\n");
      isInputMode = false;
    }
  } else if (XBee.available()) {
    bufferString = XBee.readStringUntil('\n');
    handleData();
  }
}

void handleData() {
  if (bufferString.startsWith("A")) {
    if (bufferString == "AWaitUser") {
      XBee.print("AUserReady\n");
    } else if (bufferString == "AStartInput") {
      isInputMode = true;
      tft.fillScreen(WHITE);
      renderButtonsInRandom();
      renderTitle();
    } else if (bufferString == "APassGot") {
        tft.fillScreen(WHITE);
        renderMessage("Sending...", BLACK);
        renderTitle();
    } else if (bufferString == "AAuthSuccess") {
        tft.fillScreen(WHITE);
        renderMessage(" Success!", BLUE);
        renderTitle();
    } else if (bufferString == "AAuthFailed") {
        tft.fillScreen(WHITE);
        isAuthFailed = true;
        renderMessage("  Wrong!", RED);
        renderRetryButton();
        renderTitle();
    }
  }
}
  
void detectTouch() {
  digitalWrite(13, HIGH);
  TSPoint p = ts.getPoint();
  digitalWrite(13, LOW);

  // if sharing pins, you'll need to fix the directions of the touchscreen pins
  // pinMode(XP, OUTPUT);
  pinMode(XM, OUTPUT);
  pinMode(YP, OUTPUT);
  // pinMode(YM, OUTPUT);
  
  // we have some minimum pressure we consider 'valid'
  // pressure of 0 means no pressing!

  if (p.z > MINPRESSURE && p.z < MAXPRESSURE) {
    // scale from 0->1023 to tft.width
    p.x = map(p.x, TS_MINX, TS_MAXX, tft.width(), 0);
    p.y = map(p.y, TS_MINY, TS_MAXY, tft.height(), 0);
  
    int row = 0;
    int currentRowBottomY = 42 + (BUTTONSIZE + MARGIN) / 2;
    while (p.y > currentRowBottomY) {
      ++row;
      currentRowBottomY += 81;
    }
  
    int col = 0;
    int currentColRightX = 42 + (BUTTONSIZE + MARGIN) / 2;
    while (p.x > currentColRightX) {
      ++col;
      currentColRightX += 78;
    }
  
    if (row < 3 && col < 3) {
      int index = 3 * row + col;
      saveInputedNumber(positions[index].number);
    }
  }       
}

void saveInputedNumber(int number) {
    unsigned long currentTouchedTime = micros();
    if (currentTouchedTime - lastTouchedTime > 500000) {
      passwordString += String(number);
      lastTouchedTime = currentTouchedTime;
    }
}

void renderRetryButton() {
  tft.fillRect(40, 120, 160, 40, BLACK);
  tft.setCursor(40, 120);
  tft.setTextSize(5);
  tft.setTextColor(WHITE);
  tft.println("Retry");
}

void renderButtonsInRandom() {
  // Initialized button index.
  for (int i = 0; i < 9; ++i) {
    positions[i].number = 0;
  }
  
  for (int i = 0; i < 9; ++i) {
    int randomIndex = rand() % 9;
    
    while(positions[randomIndex].number != 0) {
      randomIndex = rand() % 9;
    }
    
    positions[randomIndex].number = i + 1;

    renderButton(positions[randomIndex]); 
  }
  
}

void renderButton(position pos) {
  int centerXValue = pos.x;
  int centerYValue = pos.y;
  int number       = pos.number;
  
  const uint16_t color = BLACK;
  int h = BUTTONSIZE / 6;

  // Set radius
  tft.fillCircle(centerXValue-h, centerYValue-h, 2*h, color);
  tft.fillCircle(centerXValue-h, centerYValue+h-1, 2*h, color);
  tft.fillCircle(centerXValue+h-1, centerYValue-h, 2*h, color);
  tft.fillCircle(centerXValue+h-1, centerYValue+h-1, 2*h, color);

  // Fill rectangle
  tft.fillRect(centerXValue-3*h, centerYValue-h, BUTTONSIZE, 2*h, color);
  tft.fillRect(centerXValue-h, centerYValue-3*h, 2*h, BUTTONSIZE, color);

  // Button number
  tft.setTextColor(WHITE);
  tft.setTextSize(5);
  tft.setCursor(centerXValue-h, centerYValue-1.5*h);
  tft.println(number);
}

void renderTitle() {
  // Text - CommanderPass
  tft.setTextSize(2);
  tft.setCursor(43, 261);
  tft.setTextColor(BLACK);
  tft.println("CommanderPass");
  tft.setCursor(42, 260);
  tft.setTextColor(RED);
  tft.println("CommanderPass");
  
  // Trademark
  tft.setCursor(193, 251);
  tft.setTextColor(BLACK);
  tft.setTextSize(1);
  tft.println("TM");
  tft.setCursor(192, 250);
  tft.setTextColor(RED);
  tft.println("TM");

  // Seperator
  tft.fillRect(42, 280, 155, 1, RED);
  tft.fillRect(42, 283, 155, 2, RED);

  // Text - Ritsumeikan University
  tft.setCursor(55, 290);
  tft.setTextColor(RED);
  tft.setTextSize(1);
  tft.println("Ritsumeikan University");
}

void renderMessage(String message, uint16_t color) {
  tft.setCursor(60, 60);
  tft.setTextSize(2);
  tft.setTextColor(color);
  tft.println(message);
}

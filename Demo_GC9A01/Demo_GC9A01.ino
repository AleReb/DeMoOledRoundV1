/*
  Por Alejandro Rebolledo
  arebolledo@udd.cl
*/

#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_GC9A01A.h>

// GC9A01 1.28" Round TFT LCD 240x240 RGB - 4-Wire SPI

#define TFT_SCLK 12
#define TFT_MOSI 11
#define TFT_CS   8
#define TFT_DC   10
#define TFT_RST  9
#define TFT_BL   13

#define SCREEN_W 240
#define SCREEN_H 240

#define CENTER_X 120
#define CENTER_Y 120

bool displayOk = false;
bool backlightOk = false;

uint32_t lastFrameMs = 0;
uint32_t lastSerialMs = 0;
uint16_t frameCounter = 0;

int previousNeedleX = CENTER_X;
int previousNeedleY = CENTER_Y;
int previousValue = -1;

Adafruit_GC9A01A tft(TFT_CS, TFT_DC, TFT_RST);

void setupBacklight()
{
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);
  backlightOk = true;
}

void drawStaticInterface()
{
  tft.fillScreen(GC9A01A_BLACK);

  tft.drawCircle(CENTER_X, CENTER_Y, 118, GC9A01A_BLUE);
  tft.drawCircle(CENTER_X, CENTER_Y, 117, GC9A01A_CYAN);
  tft.drawCircle(CENTER_X, CENTER_Y, 102, GC9A01A_DARKGREY);

  for (int angle = -140; angle <= 140; angle += 10) {
    float rad = angle * DEG_TO_RAD;

    int x1 = CENTER_X + cos(rad) * 88;
    int y1 = CENTER_Y + sin(rad) * 88;

    int tickLength = (angle % 30 == 0) ? 16 : 8;

    int x2 = CENTER_X + cos(rad) * (88 + tickLength);
    int y2 = CENTER_Y + sin(rad) * (88 + tickLength);

    uint16_t tickColor = (angle % 30 == 0) ? GC9A01A_WHITE : GC9A01A_DARKGREY;
    tft.drawLine(x1, y1, x2, y2, tickColor);
  }

  tft.setTextColor(GC9A01A_WHITE, GC9A01A_BLACK);
  tft.setTextSize(2);
  tft.setCursor(56, 28);
  tft.print("GC9A01");

  tft.setTextColor(GC9A01A_CYAN, GC9A01A_BLACK);
  tft.setTextSize(1);
  tft.setCursor(58, 51);
  tft.print("Hardware SPI demo");

  tft.fillRoundRect(35, 185, 170, 34, 8, GC9A01A_NAVY);
  tft.drawRoundRect(35, 185, 170, 34, 8, GC9A01A_CYAN);

  tft.setTextColor(GC9A01A_WHITE, GC9A01A_NAVY);
  tft.setTextSize(1);
  tft.setCursor(62, 199);
  tft.print("partial redraw animation");
}

void drawValueText(int value)
{
  tft.fillRect(82, 138, 80, 28, GC9A01A_BLACK);

  tft.setTextSize(3);
  tft.setTextColor(GC9A01A_YELLOW, GC9A01A_BLACK);
  tft.setCursor(82, 140);

  if (value < 10) {
    tft.print(" ");
  }

  tft.print(value);
  tft.print("%");
}

void drawNeedle(int value)
{
  if (previousValue >= 0) {
    tft.drawLine(CENTER_X, CENTER_Y, previousNeedleX, previousNeedleY, GC9A01A_BLACK);
    tft.drawLine(CENTER_X - 1, CENTER_Y, previousNeedleX, previousNeedleY, GC9A01A_BLACK);
    tft.drawLine(CENTER_X + 1, CENTER_Y, previousNeedleX, previousNeedleY, GC9A01A_BLACK);
  }

  float angle = map(value, 0, 100, -140, 140) * DEG_TO_RAD;

  int needleX = CENTER_X + cos(angle) * 76;
  int needleY = CENTER_Y + sin(angle) * 76;

  tft.drawLine(CENTER_X, CENTER_Y, needleX, needleY, GC9A01A_RED);
  tft.drawLine(CENTER_X - 1, CENTER_Y, needleX, needleY, GC9A01A_RED);
  tft.drawLine(CENTER_X + 1, CENTER_Y, needleX, needleY, GC9A01A_RED);

  tft.fillCircle(CENTER_X, CENTER_Y, 8, GC9A01A_WHITE);
  tft.fillCircle(CENTER_X, CENTER_Y, 5, GC9A01A_RED);

  previousNeedleX = needleX;
  previousNeedleY = needleY;
  previousValue = value;
}

void drawMovingDot(uint16_t frame)
{
  static int oldX = -1;
  static int oldY = -1;

  if (oldX >= 0 && oldY >= 0) {
    tft.fillCircle(oldX, oldY, 5, GC9A01A_BLACK);
  }

  float angle = frame * 0.08;

  int x = CENTER_X + cos(angle) * 111;
  int y = CENTER_Y + sin(angle) * 111;

  tft.fillCircle(x, y, 5, GC9A01A_GREEN);

  oldX = x;
  oldY = y;
}

void setup()
{
  Serial.begin(115200);
  delay(500);

  Serial.println();
  Serial.println("Starting GC9A01 Hardware SPI demo...");

  setupBacklight();

  SPI.begin(TFT_SCLK, -1, TFT_MOSI, TFT_CS);

  tft.begin();
  tft.setRotation(0);

  displayOk = true;

  Serial.println("Display initialized.");
  Serial.println("Using Hardware SPI with custom ESP32 pins.");

  drawStaticInterface();
}

void loop()
{
  uint32_t now = millis();

  if (!displayOk) {
    if (now - lastSerialMs >= 1000) {
      lastSerialMs = now;
      Serial.println("Display is not available.");
    }
  }

  if (displayOk && now - lastFrameMs >= 33) {
    lastFrameMs = now;
    frameCounter++;

    int value = 50 + sin(frameCounter * 0.06) * 45;

    drawMovingDot(frameCounter);
    drawNeedle(value);

    if (value != previousValue) {
      drawValueText(value);
    }
  }

  if (now - lastSerialMs >= 3000) {
    lastSerialMs = now;

    Serial.print("Display OK: ");
    Serial.print(displayOk ? "yes" : "no");

    Serial.print(" | Backlight OK: ");
    Serial.print(backlightOk ? "yes" : "no");

    Serial.print(" | Frame: ");
    Serial.println(frameCounter);
  }
}

/*
  Por Alejandro Rebolledo
  arebolledo@udd.cl

  Animacion de prueba de concepto.
*/

#include <Arduino.h>
#include <SPI.h>
#include <LittleFS.h>
#include <Adafruit_GFX.h>
#include <Adafruit_GC9A01A.h>
#include <math.h>

// =========================
// Pin configuration
// =========================
#define TFT_SCLK 12
#define TFT_MOSI 11
#define TFT_CS   8
#define TFT_DC   10
#define TFT_RST  9
#define TFT_BL   13

// =========================
// Display configuration
// =========================
#define SCREEN_W 240
#define SCREEN_H 240
#define CENTER_X 120
#define CENTER_Y 120

// =========================
// Sprite configuration
// =========================
#define SPRITE_W 120
#define SPRITE_H 130
#define SPRITE_X 60
#define SPRITE_Y 56

#define FRAME_COUNT 4
#define FRAME_INTERVAL_MS 180

// RGB565 magenta transparent key
#define TRANSPARENT_COLOR 0xF81F

Adafruit_GC9A01A tft(TFT_CS, TFT_DC, TFT_RST);

// =========================
// Global flags
// =========================
bool displayOk = false;
bool backlightOk = false;
bool littleFsOk = false;
bool spriteSystemOk = false;
bool spritesLoadedOk = false;

// =========================
// Timers
// =========================
uint32_t lastFrameMs = 0;
uint32_t lastStatusMs = 0;
uint32_t lastBarsMs = 0;

// =========================
// Animation state
// =========================
uint16_t frameCounter = 0;
uint8_t currentFrame = 0;

// =========================
// Sprite memory
// =========================
uint16_t *spriteFrames[FRAME_COUNT];

// =========================
// Colors
// =========================
uint16_t colorBg;
uint16_t colorPanel;
uint16_t colorPanelDark;
uint16_t colorCyan;
uint16_t colorBlue;
uint16_t colorPink;
uint16_t colorPurple;
uint16_t colorYellow;
uint16_t colorOrange;
uint16_t colorGreen;
uint16_t colorWhite;
uint16_t colorBlack;
uint16_t colorGrey;

// =========================
// Color helper
// =========================
uint16_t rgb(uint8_t r, uint8_t g, uint8_t b)
{
  return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

void setupColors()
{
  colorBg = rgb(5, 8, 22);
  colorPanel = rgb(16, 28, 58);
  colorPanelDark = rgb(6, 14, 34);
  colorCyan = rgb(0, 220, 255);
  colorBlue = rgb(40, 100, 255);
  colorPink = rgb(255, 90, 180);
  colorPurple = rgb(135, 75, 230);
  colorYellow = rgb(255, 220, 70);
  colorOrange = rgb(255, 150, 40);
  colorGreen = rgb(70, 255, 130);
  colorWhite = rgb(255, 255, 255);
  colorBlack = rgb(0, 0, 0);
  colorGrey = rgb(120, 130, 150);
}

// =========================
// Backlight
// =========================
void setupBacklight()
{
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);

  backlightOk = true;
  Serial.println("Backlight enabled.");
}

// =========================
// LittleFS
// =========================
void setupLittleFS()
{
  if (LittleFS.begin(true)) {
    littleFsOk = true;
    Serial.println("LittleFS mounted.");
  } else {
    littleFsOk = false;
    Serial.println("LittleFS mount failed.");
  }
}

// =========================
// UI helpers
// =========================
void drawCenteredText(const char *text, int16_t y, uint8_t textSize, uint16_t color, uint16_t bg)
{
  int16_t x1;
  int16_t y1;
  uint16_t w;
  uint16_t h;

  tft.setTextSize(textSize);
  tft.setTextColor(color, bg);
  tft.getTextBounds(text, 0, y, &x1, &y1, &w, &h);

  int16_t x = (SCREEN_W - w) / 2;

  tft.setCursor(x, y);
  tft.print(text);
}

void drawPanel(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t fillColor, uint16_t borderColor)
{
  tft.fillRoundRect(x, y, w, h, 10, fillColor);
  tft.drawRoundRect(x, y, w, h, 10, borderColor);
}

void drawSparkle(int16_t x, int16_t y, uint16_t color)
{
  tft.drawLine(x - 4, y, x + 4, y, color);
  tft.drawLine(x, y - 4, x, y + 4, color);
  tft.drawPixel(x - 2, y - 2, color);
  tft.drawPixel(x + 2, y + 2, color);
}

void drawHeartIcon(int16_t x, int16_t y, uint16_t color)
{
  tft.fillCircle(x - 3, y - 2, 3, color);
  tft.fillCircle(x + 3, y - 2, 3, color);
  tft.fillTriangle(x - 7, y - 1, x + 7, y - 1, x, y + 8, color);
}

void drawCoinIcon(int16_t x, int16_t y)
{
  tft.fillCircle(x, y, 8, colorYellow);
  tft.drawCircle(x, y, 8, colorOrange);

  tft.setTextSize(1);
  tft.setTextColor(colorOrange, colorYellow);
  tft.setCursor(x - 3, y - 3);
  tft.print("$");
}

void drawProgressBar(int16_t x, int16_t y, int16_t w, int16_t h, uint8_t percent, uint16_t fillColor)
{
  tft.drawRoundRect(x, y, w, h, 5, colorWhite);
  tft.fillRoundRect(x + 2, y + 2, w - 4, h - 4, 4, colorPanelDark);

  int fillW = map(percent, 0, 100, 0, w - 4);

  if (fillW > 0) {
    tft.fillRoundRect(x + 2, y + 2, fillW, h - 4, 4, fillColor);
  }
}

void drawSpeechBubble(const char *text)
{
  drawPanel(150, 54, 70, 34, colorWhite, colorPink);

  tft.fillTriangle(164, 88, 174, 88, 168, 96, colorWhite);
  tft.drawLine(164, 88, 168, 96, colorPink);
  tft.drawLine(174, 88, 168, 96, colorPink);

  tft.setTextSize(1);
  tft.setTextColor(colorPanelDark, colorWhite);
  tft.setCursor(164, 66);
  tft.print(text);
}

void drawStaticUI()
{
  tft.fillScreen(colorBg);

  tft.fillCircle(CENTER_X, CENTER_Y, 118, colorPanelDark);
  tft.drawCircle(CENTER_X, CENTER_Y, 118, colorBlue);
  tft.drawCircle(CENTER_X, CENTER_Y, 116, colorCyan);

  drawCenteredText("CYBER", 16, 2, colorCyan, colorBg);

  tft.setTextSize(2);
  tft.setTextColor(colorPink, colorBg);
  tft.setCursor(130, 16);
  tft.print("PET");

  drawHeartIcon(120, 10, colorPink);

  drawSparkle(28, 50, colorCyan);
  drawSparkle(214, 52, colorCyan);
  drawSparkle(36, 118, colorPink);
  drawSparkle(202, 120, colorYellow);
  drawSparkle(60, 154, colorCyan);
  drawSparkle(184, 154, colorPink);

  drawPanel(18, 48, 62, 34, colorPanel, colorPink);

  tft.setTextSize(1);
  tft.setTextColor(colorPink, colorPanel);
  tft.setCursor(28, 56);
  tft.print("NAME");

  tft.setTextColor(colorCyan, colorPanel);
  tft.setCursor(27, 67);
  tft.print("NIKO");

  drawPanel(18, 92, 62, 40, colorPanel, colorCyan);

  tft.setTextColor(colorCyan, colorPanel);
  tft.setCursor(24, 100);
  tft.print("LEVEL");

  tft.setTextSize(2);
  tft.setTextColor(colorWhite, colorPanel);
  tft.setCursor(28, 112);
  tft.print("05");

  tft.fillCircle(64, 118, 4, colorYellow);

  drawPanel(18, 142, 62, 34, colorPanel, colorPurple);

  tft.setTextSize(1);
  tft.setTextColor(colorPurple, colorPanel);
  tft.setCursor(27, 150);
  tft.print("MOOD");

  drawHeartIcon(49, 165, colorPink);

  drawPanel(160, 100, 62, 40, colorPanel, colorYellow);

  tft.setTextColor(colorYellow, colorPanel);
  tft.setCursor(173, 108);
  tft.print("COINS");

  drawCoinIcon(176, 124);

  tft.setTextSize(2);
  tft.setTextColor(colorWhite, colorPanel);
  tft.setCursor(188, 116);
  tft.print("128");

  drawPanel(160, 148, 62, 36, colorPanel, colorCyan);

  tft.setTextSize(1);
  tft.setTextColor(colorCyan, colorPanel);
  tft.setCursor(176, 156);
  tft.print("TIME");

  tft.setTextColor(colorWhite, colorPanel);
  tft.setCursor(178, 170);
  tft.print("12:34");

  drawSpeechBubble("Meow!");

  drawPanel(26, 188, 188, 38, colorPanelDark, colorPurple);

  // Sprite background area.
  tft.fillRoundRect(SPRITE_X - 4, SPRITE_Y - 4, SPRITE_W + 8, SPRITE_H + 8, 18, colorPanelDark);
  tft.drawRoundRect(SPRITE_X - 4, SPRITE_Y - 4, SPRITE_W + 8, SPRITE_H + 8, 18, colorPurple);
}

// =========================
// Dynamic bars
// =========================
void drawBottomBars(uint16_t frame)
{
  uint8_t hunger = 60 + (int)(sin(frame * 0.05f) * 12.0f);
  uint8_t energy = 80 + (int)(cos(frame * 0.04f) * 8.0f);

  if (hunger > 100) hunger = 100;
  if (energy > 100) energy = 100;

  tft.fillRect(34, 196, 175, 24, colorPanelDark);

  tft.setTextSize(1);

  tft.setTextColor(colorPink, colorPanelDark);
  tft.setCursor(34, 196);
  tft.print("HUNGER");

  drawProgressBar(96, 194, 78, 10, hunger, colorPink);

  tft.setCursor(180, 196);
  tft.print(hunger);
  tft.print("%");

  tft.setTextColor(colorCyan, colorPanelDark);
  tft.setCursor(34, 210);
  tft.print("ENERGY");

  drawProgressBar(96, 208, 78, 10, energy, colorCyan);

  tft.setCursor(180, 210);
  tft.print(energy);
  tft.print("%");
}

// =========================
// Sprite loading to RAM
// =========================
void clearSpriteMemory()
{
  for (uint8_t i = 0; i < FRAME_COUNT; i++) {
    spriteFrames[i] = nullptr;
  }
}

bool loadRawFrameToRam(uint8_t index)
{
  if (!littleFsOk) {
    Serial.println("Cannot load frame: LittleFS is not mounted.");
    return false;
  }

  if (index >= FRAME_COUNT) {
    Serial.println("Invalid frame index.");
    return false;
  }

  char path[32];
  snprintf(path, sizeof(path), "/cat/cat%02u.raw", index);

  File file = LittleFS.open(path, "r");

  if (!file) {
    Serial.print("Cannot open frame: ");
    Serial.println(path);
    return false;
  }

  const size_t expectedBytes = (size_t)SPRITE_W * (size_t)SPRITE_H * 2;

  if (file.size() != expectedBytes) {
    Serial.print("Invalid frame size: ");
    Serial.print(path);
    Serial.print(" | expected=");
    Serial.print(expectedBytes);
    Serial.print(" | actual=");
    Serial.println(file.size());

    file.close();
    return false;
  }

  spriteFrames[index] = (uint16_t *)malloc(expectedBytes);

  if (spriteFrames[index] == nullptr) {
    Serial.print("RAM allocation failed for frame: ");
    Serial.println(index);

    file.close();
    return false;
  }

  uint8_t *rawBuffer = (uint8_t *)spriteFrames[index];

  size_t bytesRead = file.read(rawBuffer, expectedBytes);
  file.close();

  if (bytesRead != expectedBytes) {
    Serial.print("Incomplete frame read: ");
    Serial.println(path);

    free(spriteFrames[index]);
    spriteFrames[index] = nullptr;
    return false;
  }

  // Convert little-endian file into native uint16_t values and replace magenta
  // with local background color so the frame can be drawn in one fast block.
  for (size_t pixel = 0; pixel < (size_t)SPRITE_W * (size_t)SPRITE_H; pixel++) {
    uint8_t lowByte = rawBuffer[pixel * 2];
    uint8_t highByte = rawBuffer[pixel * 2 + 1];

    uint16_t color = (uint16_t)((highByte << 8) | lowByte);

    if (color == TRANSPARENT_COLOR) {
      color = colorPanelDark;
    }

    spriteFrames[index][pixel] = color;
  }

  Serial.print("Loaded frame to RAM: ");
  Serial.println(path);

  return true;
}

void loadAllSpritesToRam()
{
  spritesLoadedOk = true;

  for (uint8_t i = 0; i < FRAME_COUNT; i++) {
    bool ok = loadRawFrameToRam(i);

    if (!ok) {
      spritesLoadedOk = false;
    }
  }

  spriteSystemOk = spritesLoadedOk;

  if (spritesLoadedOk) {
    Serial.println("All frames loaded to RAM.");
  } else {
    Serial.println("Some frames failed to load. Fallback will be used.");
  }
}

// =========================
// Fast sprite drawing
// =========================
void drawSpriteFrame(uint8_t index)
{
  if (!spriteSystemOk) {
    return;
  }

  if (index >= FRAME_COUNT) {
    return;
  }

  if (spriteFrames[index] == nullptr) {
    return;
  }

  tft.drawRGBBitmap(SPRITE_X, SPRITE_Y, spriteFrames[index], SPRITE_W, SPRITE_H);
}

// =========================
// Fallback
// =========================
void drawFallbackPet(uint16_t frame)
{
  int bob = (int)(sin(frame * 0.12f) * 3.0f);
  int cx = CENTER_X;
  int cy = 118 + bob;

  tft.fillRoundRect(SPRITE_X, SPRITE_Y, SPRITE_W, SPRITE_H, 16, colorPanelDark);

  tft.fillTriangle(cx - 32, cy - 48, cx - 12, cy - 18, cx - 38, cy - 12, colorGrey);
  tft.fillTriangle(cx + 32, cy - 48, cx + 12, cy - 18, cx + 38, cy - 12, colorGrey);

  tft.fillCircle(cx, cy - 8, 36, colorGrey);
  tft.fillCircle(cx, cy, 30, rgb(255, 210, 180));

  tft.fillCircle(cx - 13, cy - 5, 6, colorCyan);
  tft.fillCircle(cx + 13, cy - 5, 6, colorCyan);

  tft.fillCircle(cx - 13, cy - 5, 3, colorBlack);
  tft.fillCircle(cx + 13, cy - 5, 3, colorBlack);

  tft.drawLine(cx - 5, cy + 12, cx, cy + 17, colorPink);
  tft.drawLine(cx, cy + 17, cx + 5, cy + 12, colorPink);

  tft.fillRoundRect(cx - 28, cy + 28, 56, 36, 12, colorBlue);

  tft.setTextSize(1);
  tft.setTextColor(colorYellow, colorPanelDark);
  tft.setCursor(80, 174);
  tft.print("Missing frames");
}

// =========================
// Animation update
// =========================
void updateSpriteAnimation(uint32_t now)
{
  if (!displayOk) {
    return;
  }

  if (now - lastFrameMs < FRAME_INTERVAL_MS) {
    return;
  }

  lastFrameMs = now;
  frameCounter++;

  currentFrame++;
  if (currentFrame >= FRAME_COUNT) {
    currentFrame = 0;
  }

  if (spriteSystemOk) {
    drawSpriteFrame(currentFrame);
  } else {
    drawFallbackPet(frameCounter);
  }
}

// =========================
// Setup
// =========================
void setup()
{
  Serial.begin(115200);
  delay(500);

  Serial.println();
  Serial.println("Starting GC9A01 RAM sprite demo...");

  clearSpriteMemory();
  setupBacklight();

  SPI.begin(TFT_SCLK, -1, TFT_MOSI, TFT_CS);

  tft.begin();
  tft.setRotation(0);

  // Try higher SPI speed. If your display gets artifacts, lower to 27000000.
  tft.setSPISpeed(40000000);

  setupColors();

  displayOk = true;
  Serial.println("Display initialized.");
  Serial.println("Hardware SPI enabled.");

  setupLittleFS();
  loadAllSpritesToRam();

  drawStaticUI();
  drawBottomBars(0);

  if (spriteSystemOk) {
    drawSpriteFrame(0);
  } else {
    drawFallbackPet(0);
  }
}

// =========================
// Main loop
// =========================
void loop()
{
  uint32_t now = millis();

  if (!displayOk) {
    if (now - lastStatusMs >= 1000) {
      lastStatusMs = now;
      Serial.println("Display not available.");
    }
  }

  updateSpriteAnimation(now);

  if (displayOk && now - lastBarsMs >= 500) {
    lastBarsMs = now;
    drawBottomBars(frameCounter);
  }

  if (now - lastStatusMs >= 3000) {
    lastStatusMs = now;

    Serial.print("Display OK: ");
    Serial.print(displayOk ? "yes" : "no");

    Serial.print(" | Backlight OK: ");
    Serial.print(backlightOk ? "yes" : "no");

    Serial.print(" | LittleFS OK: ");
    Serial.print(littleFsOk ? "yes" : "no");

    Serial.print(" | Sprites loaded OK: ");
    Serial.print(spritesLoadedOk ? "yes" : "no");

    Serial.print(" | Current frame: ");
    Serial.println(currentFrame);
  }
}

// TML_capacitive_screen.cpp

#include "TML_capacitive_screen.h"

#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_HX8357.h>
#include <Adafruit_FT5336.h>

#define FT5336_MAXTOUCHES 1 // maximum 1 touch simultaneously

#define TFT_IRQ 3 // DISPLAY_INT
#define TFT_SDA 46 // DISPLAY_SDA
#define TFT_SCL 9 // DISPLAY_SCL
#define TFT_CS 10 // DISPLAY_TCS
#define TFT_MOSI 11 // DISPLAY_MOSI
#define TFT_SCLK 12 // DISPLAY_SCK
#define TFT_RST 13 // DISPLAY_RST
#define TFT_DC 14 // DISPLAY_DC
#define TFT_MISO -1 // -1 if unused

Adafruit_HX8357 tft = Adafruit_HX8357(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST, TFT_MISO);

Adafruit_FT5336 ctp = Adafruit_FT5336();

unsigned int lastDebounceTime = 0;
unsigned int debounceDelay = 500;

char screen_main_commands[] = "nopqr^&+";
char screen_aux_commands[] = "s@#$%^&+";

// void IRAM_ATTR ISR(void){


//   return;
// }

void displayWireSetup(void){
  Wire1.begin(TFT_SDA, TFT_SCL);

  return;
}

void displaySetup(void){
  tft.begin();

  char16_t x = tft.readcommand8(HX8357_RDPOWMODE);
  Serial.print("Display Power Mode: 0x"); Serial.println(x, HEX);
  x = tft.readcommand8(HX8357_RDMADCTL);
  Serial.print("MADCTL Mode: 0x"); Serial.println(x, HEX);
  x = tft.readcommand8(HX8357_RDCOLMOD);
  Serial.print("Pixel Format: 0x"); Serial.println(x, HEX);
  x = tft.readcommand8(HX8357_RDDIM);
  Serial.print("Image Format: 0x"); Serial.println(x, HEX);
  x = tft.readcommand8(HX8357_RDDSDR);
  Serial.print("Self Diagnostic: 0x"); Serial.println(x, HEX);

  Serial.println("Display initialized!");

  tft.setRotation(2);
  tft.fillScreen(HX8357_BLACK);

  int w = tft.width();
  int h = tft.height();

  displayDrawLine((w - 1) / 2, 0, (w - 1) / 2, (h - 1), HX8357_WHITE);
  displayDrawText(0, 0, HX8357_WHITE, 4, "MAIN");
  displayDrawText(174, 0, HX8357_WHITE, 4, "AUX");

  displayDrawText(0, 50, HX8357_WHITE, 2, "FORE: ");
  displayDrawForce(65, 50, HX8357_WHITE, 2, 0000);
  displayDrawText(0, 80, HX8357_WHITE, 2, "HEEL: ");
  displayDrawForce(65, 80, HX8357_WHITE, 2, 0000);
  displayDrawText(0, 110, HX8357_WHITE, 2, "ANGLE: ");
  displayDrawAngle(75, 110, HX8357_WHITE, 2, 0000);

  displayDrawText(174, 50, HX8357_WHITE, 2, "FORE: ");
  displayDrawForce(239, 50, HX8357_WHITE, 2, 0000);
  displayDrawText(174, 80, HX8357_WHITE, 2, "HEEL: ");
  displayDrawForce(239, 80, HX8357_WHITE, 2, 0000);
  displayDrawText(174, 110, HX8357_WHITE, 2, "ANGLE: ");
  displayDrawAngle(249, 110, HX8357_WHITE, 2, 0000);

  displayDrawText(0, 150, HX8357_WHITE, 2, "UP");
  displayDrawBox(0, 170, 65, 65, HX8357_BLUE, 5);
  displayDrawText(0, 265, HX8357_WHITE, 2, "STOP");
  displayDrawBox(0, 285, 65, 65, HX8357_BLUE, 5);
  displayDrawText(0, 380, HX8357_WHITE, 2, "DOWN");
  displayDrawBox(0, 400, 65, 65, HX8357_BLUE, 5);

  displayDrawText(80, 150, HX8357_WHITE, 2, "AUTO");
  displayDrawBox(80, 170, 65, 65, HX8357_BLUE, 5);
  displayDrawText(80, 265, HX8357_WHITE, 2, "TARE");
  displayDrawBox(80, 285, 65, 65, HX8357_BLUE, 5);
  displayDrawText(80, 380, HX8357_WHITE, 2, "HOME");
  displayDrawBox(80, 400, 65, 65, HX8357_BLUE, 5);

  displayDrawText(254, 150, HX8357_WHITE, 2, "UP");
  displayDrawBox(254, 170, 65, 65, HX8357_MAGENTA, 5);
  displayDrawText(254, 265, HX8357_WHITE, 2, "STOP");
  displayDrawBox(254, 285, 65, 65, HX8357_MAGENTA, 5);
  displayDrawText(254, 380, HX8357_WHITE, 2, "DOWN");
  displayDrawBox(254, 400, 65, 65, HX8357_MAGENTA, 5);

  displayDrawText(174, 150, HX8357_WHITE, 2, "AUTO");
  displayDrawBox(174, 170, 65, 65, HX8357_MAGENTA, 5);
  displayDrawText(174, 265, HX8357_WHITE, 2, "TARE");
  displayDrawBox(174, 285, 65, 65, HX8357_MAGENTA, 5);
  displayDrawText(174, 380, HX8357_WHITE, 2, "HOME");
  displayDrawBox(174, 400, 65, 65, HX8357_MAGENTA, 5);

  displayWireSetup();

  if (!ctp.begin(FT53XX_DEFAULT_ADDR, &Wire1)){
    Serial.println("Error: could not start FT5336 touchscreen controller!");

  }
  else{
    Serial.println("Capacitive touchscreen started!");

    // attachInterrupt(TFT_IRQ, ISR, FALLING);
  }

  return;
}

char displayTouchChecker(void){
  if (!((millis() - lastDebounceTime) > debounceDelay)){
    return '\0';
  }

  uint8_t touches = ctp.touched();
  // Wait for a touch
  if (!touches){
    return '\0';
  }

  TS_Point ps[FT5336_MAXTOUCHES];
  ctp.getPoints(ps, FT5336_MAXTOUCHES);

  // MAIN UP
  if (ps[0].z != 0 && ps[0].x > 263 && ps[0].x < 323 && ps[0].y > 243 && ps[0].y < 303){
    return S2M_MAIN_UP;
  }
  // MAIN STOP
  else if (ps[0].z != 0 && ps[0].x > 263 && ps[0].x < 323 && ps[0].y > 137 && ps[0].y < 197){
    return S2M_MAIN_STOP;
  }
  // MAIN DOWN
  else if (ps[0].z != 0 && ps[0].x > 263 && ps[0].x < 323 && ps[0].y > 17 && ps[0].y < 77){
    return S2M_MAIN_DOWN;
  }
  // MAIN AUTO
  else if (ps[0].z != 0 && ps[0].x > 183 && ps[0].x < 243 && ps[0].y > 243 && ps[0].y < 303){
    return S2M_MAIN_AUTO;
  }
  // MAIN TARE
  else if (ps[0].z != 0 && ps[0].x > 183 && ps[0].x < 243 && ps[0].y > 137 && ps[0].y < 197){
    return S2M_MAIN_TARE;
  }
  // MAIN HOME
  else if (ps[0].z != 0 && ps[0].x > 183 && ps[0].x < 243 && ps[0].y > 17 && ps[0].y < 77){
    return S2M_MAIN_HOME;
  }
  // AUX UP
  else if (ps[0].z != 0 && ps[0].x > 0 && ps[0].x < 66 && ps[0].y > 243 && ps[0].y < 303){
    return S2M_AUX_UP;
  }
  // AUX STOP
  else if (ps[0].z != 0 && ps[0].x > 0 && ps[0].x < 66 && ps[0].y > 137 && ps[0].y < 197){
    return S2M_AUX_STOP;
  }
  // AUX DOWN
  else if (ps[0].z != 0 && ps[0].x > 0 && ps[0].x < 66 && ps[0].y > 17 && ps[0].y < 77){
    return S2M_AUX_DOWN;
  }
  // AUX AUTO
  else if (ps[0].z != 0 && ps[0].x > 90 && ps[0].x < 150 && ps[0].y > 243 && ps[0].y < 303){
    return S2M_AUX_AUTO;
  }
  // AUX TARE
  else if (ps[0].z != 0 && ps[0].x > 90 && ps[0].x < 150 && ps[0].y > 137 && ps[0].y < 197){
    return S2M_AUX_TARE;
  }
  // AUX HOME
  else if (ps[0].z != 0 && ps[0].x > 90 && ps[0].x < 150 && ps[0].y > 17 && ps[0].y < 77){
    return S2M_AUX_HOME;
  }

  lastDebounceTime = millis();

  return '\0';
}

void displayUpdate(int main_fore, int main_heel, int main_angle, int aux_fore, int aux_heel, int aux_angle){
  displayClearBox(65, 50, 60, 14, HX8357_BLACK);
  displayDrawForce(65, 50, HX8357_WHITE, 2, main_fore);

  displayClearBox(65, 80, 60, 14, HX8357_BLACK);
  displayDrawForce(65, 80, HX8357_WHITE, 2, main_heel);

  displayClearBox(75, 110, 60, 14, HX8357_BLACK);
  displayDrawAngle(75, 110, HX8357_WHITE, 2, main_angle);

  displayClearBox(239, 50, 60, 14, HX8357_BLACK);
  displayDrawForce(239, 50, HX8357_WHITE, 2, aux_fore);

  displayClearBox(239, 80, 60, 14, HX8357_BLACK);
  displayDrawForce(239, 80, HX8357_WHITE, 2, aux_heel);

  displayClearBox(249, 110, 60, 14, HX8357_BLACK);
  displayDrawAngle(249, 110, HX8357_WHITE, 2, aux_angle);

  return;
}

void displayDrawText(char16_t x, char16_t y, char16_t color, char16_t size, char* text){
  tft.setCursor(x, y);
  tft.setTextColor(color);
  tft.setTextSize(size);
  tft.println(text);

  return;
}

void displayDrawForce(char16_t x, char16_t y, char16_t color, char16_t size, int force){
  if (force >= -25 && force <= 2500){
    tft.setCursor(x, y);
    tft.setTextColor(color);
    tft.setTextSize(size);
    tft.println(force);
  }
  else{
    tft.setCursor(x, y);
    tft.setTextColor(HX8357_RED);
    tft.setTextSize(size);
    tft.println("ERROR");
  }

  return;
}

void displayDrawAngle(char16_t x, char16_t y, char16_t color, char16_t size, int angle){
  if (angle >= -1 && angle <= 50){
    tft.setCursor(x, y);
    tft.setTextColor(color);
    tft.setTextSize(size);
    tft.println(angle);
  }
  else{
    tft.setCursor(x, y);
    tft.setTextColor(HX8357_RED);
    tft.setTextSize(size);
    tft.println("ERROR");
  }

  return;
}

void displayDrawLine(char16_t x1, char16_t y1, char16_t x2, char16_t y2, char16_t color){
  tft.drawLine(x1, y1, x2, y2, color);

  return;
}

void displayDrawBox(char16_t x1, char16_t y1, char16_t x2, char16_t y2, char16_t color, char16_t radius){
  tft.fillRoundRect(x1, y1, x2, y2, radius, color);

  return;
}

void displayClearBox(char16_t x1, char16_t y1, char16_t x2, char16_t y2, char16_t color){
  tft.fillRect(x1, y1, x2, y2, color);
  
  return;
}

void displayTestSetup(void) {
  tft.begin();

  // read diagnostics (optional but can help debug problems)
  char16_t x = tft.readcommand8(HX8357_RDPOWMODE);
  Serial.print("Display Power Mode: 0x"); Serial.println(x, HEX);
  x = tft.readcommand8(HX8357_RDMADCTL);
  Serial.print("MADCTL Mode: 0x"); Serial.println(x, HEX);
  x = tft.readcommand8(HX8357_RDCOLMOD);
  Serial.print("Pixel Format: 0x"); Serial.println(x, HEX);
  x = tft.readcommand8(HX8357_RDDIM);
  Serial.print("Image Format: 0x"); Serial.println(x, HEX);
  x = tft.readcommand8(HX8357_RDDSDR);
  Serial.print("Self Diagnostic: 0x"); Serial.println(x, HEX); 
  
  Serial.println(F("Benchmark                Time (microseconds)"));

  tft.setRotation(1);

  Serial.print(F("Text                     "));
  Serial.println(testText());
  delay(500);

  Serial.print(F("Lines                    "));
  Serial.println(testLines(HX8357_CYAN));
  delay(500);

  Serial.print(F("Rectangles (outline)     "));
  Serial.println(testRects(HX8357_GREEN));
  delay(500);

  tft.fillScreen(HX8357_BLACK);
  Serial.print(F("Circles (outline)        "));
  Serial.println(testCircles(10, HX8357_RED));
  delay(500);

  Serial.print(F("Triangles (outline)      "));
  Serial.println(testTriangles());
  delay(500);

  Serial.print(F("Triangles (filled)       "));
  Serial.println(testFilledTriangles());
  delay(500);

  Serial.print(F("Rounded rects (outline)  "));
  Serial.println(testRoundRects());
  delay(500);

  Serial.print(F("Rounded rects (filled)   "));
  Serial.println(testFilledRoundRects());
  delay(500);

  Serial.println(F("Done!"));
}

void displayTestLoop(void) {
  for(char16_t rotation=0; rotation<4; rotation++) {
    tft.setRotation(rotation);
    testText();
    delay(1000);
  }
}

unsigned long testFillScreen(void) {
  unsigned long start = micros();
  tft.fillScreen(HX8357_RED);
  tft.fillScreen(HX8357_GREEN);
  tft.fillScreen(HX8357_BLUE);
  tft.fillScreen(HX8357_WHITE);
  return micros() - start;
}

unsigned long testText(void) {
  tft.fillScreen(HX8357_BLACK);
  unsigned long start = micros();
  tft.setCursor(0, 0);
  tft.setTextColor(HX8357_WHITE);  tft.setTextSize(1);
  tft.println("Hello World!");
  tft.setTextColor(HX8357_YELLOW); tft.setTextSize(2);
  tft.println(1234.56);
  tft.setTextColor(HX8357_RED);    tft.setTextSize(3);
  tft.println(0xDEADBEEF, HEX);
  tft.println();
  tft.setTextColor(HX8357_GREEN);
  tft.setTextSize(5);
  tft.println("Groop");
  tft.setTextSize(2);
  tft.println("I implore thee,");
  tft.setTextSize(1);
  tft.println("my foonting turlingdromes.");
  tft.println("And hooptiously drangle me");
  tft.println("with crinkly bindlewurdles,");
  tft.println("Or I will rend thee");
  tft.println("in the gobberwarts");
  tft.println("with my blurglecruncheon,");
  tft.println("see if I don't!");
  
  tft.setTextColor(HX8357_WHITE);
  tft.println(F("Alice was beginning to get very tired of sitting by her sister on the bank, and of having nothing to do: once or twice she had peeped into the book her sister was reading, but it had no pictures or conversations in it, 'and what is the use of a book,' thought Alice 'without pictures or conversations?'"));

  tft.println(F("So she was considering in her own mind (as well as she could, for the hot day made her feel very sleepy and stupid), whether the pleasure of making a daisy-chain would be worth the trouble of getting up and picking the daisies, when suddenly a White Rabbit with pink eyes ran close by her."));

  tft.println(F("There was nothing so very remarkable in that; nor did Alice think it so very much out of the way to hear the Rabbit say to itself, 'Oh dear! Oh dear! I shall be late!' (when she thought it over afterwards, it occurred to her that she ought to have wondered at this, but at the time it all seemed quite natural); but when the Rabbit actually took a watch out of its waistcoat-pocket, and looked at it, and then hurried on, Alice started to her feet, for it flashed across her mind that she had never before seen a rabbit with either a waistcoat-pocket, or a watch to take out of it, and burning with curiosity, she ran across the field after it, and fortunately was just in time to see it pop down a large rabbit-hole under the hedge."));

  tft.println(F("In another moment down went Alice after it, never once considering how in the world she was to get out again."));

  tft.println(F("The rabbit-hole went straight on like a tunnel for some way, and then dipped suddenly down, so suddenly that Alice had not a moment to think about stopping herself before she found herself falling down a very deep well."));

  tft.println(F("Either the well was very deep, or she fell very slowly, for she had plenty of time as she went down to look about her and to wonder what was going to happen next. First, she tried to look down and make out what she was coming to, but it was too dark to see anything; then she looked at the sides of the well, and noticed that they were filled with cupboards and book-shelves; here and there she saw maps and pictures hung upon pegs. She took down a jar from one of the shelves as she passed; it was labelled 'ORANGE MARMALADE', but to her great disappointment it was empty: she did not like to drop the jar for fear of killing somebody, so managed to put it into one of the cupboards as she fell past it."));
  
  return micros() - start;
}

unsigned long testLines(char16_t color) {
  unsigned long start;
  int           x1, y1, x2, y2,
                w = tft.width(),
                h = tft.height();

  tft.fillScreen(HX8357_BLACK);

  x1 = y1 = 0;
  y2    = h - 1;
  start = micros();
  for(x2=0; x2<w; x2+=6) tft.drawLine(x1, y1, x2, y2, color);
  x2    = w - 1;
  for(y2=0; y2<h; y2+=6) tft.drawLine(x1, y1, x2, y2, color);

  return micros() - start;
}

unsigned long testFastLines(char16_t color1, char16_t color2) {
  unsigned long start;
  int           x, y, w = tft.width(), h = tft.height();

  tft.fillScreen(HX8357_BLACK);
  start = micros();
  for(y=0; y<h; y+=5) tft.drawFastHLine(0, y, w, color1);
  for(x=0; x<w; x+=5) tft.drawFastVLine(x, 0, h, color2);

  return micros() - start;
}

unsigned long testRects(char16_t color) {
  unsigned long start;
  int           n, i, i2,
                cx = tft.width()  / 2,
                cy = tft.height() / 2;

  tft.fillScreen(HX8357_BLACK);
  n     = min(tft.width(), tft.height());
  start = micros();
  for(i=2; i<n; i+=6) {
    i2 = i / 2;
    tft.drawRect(cx-i2, cy-i2, i, i, color);
  }

  return micros() - start;
}

unsigned long testFilledRects(char16_t color1, char16_t color2) {
  unsigned long start, t = 0;
  int           n, i, i2,
                cx = tft.width()  / 2 - 1,
                cy = tft.height() / 2 - 1;

  tft.fillScreen(HX8357_BLACK);
  n = min(tft.width(), tft.height());
  for(i=n; i>0; i-=6) {
    i2    = i / 2;
    start = micros();
    tft.fillRect(cx-i2, cy-i2, i, i, color1);
    t    += micros() - start;
    // Outlines are not included in timing results
    tft.drawRect(cx-i2, cy-i2, i, i, color2);
  }

  return t;
}

unsigned long testFilledCircles(char16_t radius, char16_t color) {
  unsigned long start;
  int x, y, w = tft.width(), h = tft.height(), r2 = radius * 2;

  tft.fillScreen(HX8357_BLACK);
  start = micros();
  for(x=radius; x<w; x+=r2) {
    for(y=radius; y<h; y+=r2) {
      tft.fillCircle(x, y, radius, color);
    }
  }

  return micros() - start;
}

unsigned long testCircles(char16_t radius, char16_t color) {
  unsigned long start;
  int           x, y, r2 = radius * 2,
                w = tft.width()  + radius,
                h = tft.height() + radius;

  // Screen is not cleared for this one -- this is
  // intentional and does not affect the reported time.
  start = micros();
  for(x=0; x<w; x+=r2) {
    for(y=0; y<h; y+=r2) {
      tft.drawCircle(x, y, radius, color);
    }
  }

  return micros() - start;
}

unsigned long testTriangles(void) {
  unsigned long start;
  int           n, i, cx = tft.width()  / 2 - 1,
                      cy = tft.height() / 2 - 1;

  tft.fillScreen(HX8357_BLACK);
  n     = min(cx, cy);
  start = micros();
  for(i=0; i<n; i+=5) {
    tft.drawTriangle(
      cx    , cy - i, // peak
      cx - i, cy + i, // bottom left
      cx + i, cy + i, // bottom right
      tft.color565(200, 20, i));
  }

  return micros() - start;
}

unsigned long testFilledTriangles(void) {
  unsigned long start, t = 0;
  int           i, cx = tft.width()  / 2 - 1,
                   cy = tft.height() / 2 - 1;

  tft.fillScreen(HX8357_BLACK);
  start = micros();
  for(i=min(cx,cy); i>10; i-=5) {
    start = micros();
    tft.fillTriangle(cx, cy - i, cx - i, cy + i, cx + i, cy + i,
      tft.color565(0, i, i));
    t += micros() - start;
    tft.drawTriangle(cx, cy - i, cx - i, cy + i, cx + i, cy + i,
      tft.color565(i, i, 0));
  }

  return t;
}

unsigned long testRoundRects(void) {
  unsigned long start;
  int           w, i, i2,
                cx = tft.width()  / 2 ,
                cy = tft.height() / 2 ;

  tft.fillScreen(HX8357_BLACK);
  w     = min(tft.width(), tft.height());
  start = micros();
  for(i=0; i<w; i+=8) {
    i2 = i / 2 - 2;
    tft.drawRoundRect(cx-i2, cy-i2, i, i, i/8, tft.color565(i, 100, 100));
  }

  return micros() - start;
}

unsigned long testFilledRoundRects(void) {
  unsigned long start;
  int           i, i2,
                cx = tft.width()  / 2 + 10,
                cy = tft.height() / 2 + 10;

  tft.fillScreen(HX8357_BLACK);
  start = micros();
  for(i=min(tft.width(), tft.height()) - 20; i>25; i-=6) {
    i2 = i / 2;
    tft.fillRoundRect(cx-i2, cy-i2, i-20, i-20, i/8, tft.color565(100, i/2, 100));
  }

  return micros() - start;
}

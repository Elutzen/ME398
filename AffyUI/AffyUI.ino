// Touch screen library with X Y and Z (pressure) readings as well
// as oversampling to avoid 'bouncing'
// This demo code returns raw readings, public domain

#include <stdint.h>
#include <SPI.h>
#include "TouchScreen.h"

#define YP A6  // must be an analog pin, use "An" notation!
#define XM A9  // must be an analog pin, use "An" notation!
#define YM 22   // can be a digital pin
#define XP 21   // can be a digital pin

#define TS_MINX 147
#define TS_MINY 83
#define TS_MAXX 925
#define TS_MAXY 925

#define MINPRESSURE 10
#define MAXPRESSURE 1000

#include <Adafruit_GFX.h>
#include <Adafruit_HX8357.h>
#include <Fonts/FreeSansBold24pt7b.h>

#define _CS 10
#define _DC 9
Adafruit_HX8357 tft = Adafruit_HX8357(_CS,_DC);

// For better pressure precision, we need to know the resistance
// between X+ and X- Use any multimeter to read it
// For the one we're using, its 300 ohms across the X plate
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);
TSPoint p;
bool isTouching;

/////// Define buttons and their actions  ////////
struct Button {
  int16_t x,y,w,h;
  char* label;
  uint16_t color;
  void draw() {
    tft.fillRect(x,y,w,h,color);
    tft.setFont();
    tft.setCursor(x+5,y+5);
    tft.setTextColor(HX8357_BLACK);
    tft.setTextSize(3);
    tft.print(label);
  }
  bool pressed() { return (isTouching) && (p.x > x && p.y > y && p.x < x+w && p.y < y+h); };
  Button(int16_t _x, int16_t _y, int16_t _w, int16_t _h, char* _label, uint16_t _color) : x(_x), y(_y), w(_w), h(_h), label(_label), color(_color) {};
};

Button BtnDispense(50, 50, 150, 50, "Dispense",HX8357_GREEN);
Button BtnCancel  (50, 110, 150, 50, "Cancel", HX8357_RED);


////////// MAIN CODE /////////////
void setup(void) {
  Serial.begin(115200);
  tft.begin(HX8357D);
  tft.fillScreen(HX8357_BLACK);
  BtnDispense.draw();
  BtnCancel.draw();
}

void loop(void) {
  handleTouch();
  
  // we have some minimum pressure we consider 'valid'
  // pressure of 0 means no pressing!
  if (isTouching) {
     Serial.print("X = "); Serial.print(p.x);
     Serial.print("\tY = "); Serial.print(p.y);
     Serial.print("\tPressure = "); Serial.println(p.z);
     drawReadout();
  }

  if (BtnDispense.pressed()) {
    BtnDispense.color = HX8357_BLUE;
    BtnDispense.draw();
  }

  if (BtnCancel.pressed()) {
    BtnCancel.color = HX8357_BLUE;
    BtnCancel.draw();
  }
  
  delay(20);
}

void handleTouch() {
  // a point object holds x y and z coordinates
  p = ts.getPoint();
  p.x = map(p.x,TS_MINX,TS_MAXX,0,HX8357_TFTWIDTH);
  p.y = map(p.y,TS_MINY,TS_MAXY,0,HX8357_TFTHEIGHT);
  p.z = (p.z == 0 ? 0 : map(p.z,MINPRESSURE,MAXPRESSURE,0,100));  

  isTouching = (p.z > 0 && p.z < 100);
}

void drawReadout() {
  tft.setFont(&FreeSansBold24pt7b);
  tft.setCursor(50,300);
  tft.setTextColor(HX8357_WHITE);
  tft.setTextSize(2);
  tft.fillRect(50,200,200,106,HX8357_BLACK);
  tft.print(p.x/100.0);
}


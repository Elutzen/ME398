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

#include <Adafruit_GFX.h>
#include <Adafruit_HX8357.h>

#define _CS 10
#define _DC 9
Adafruit_HX8357 tft = Adafruit_HX8357(_CS,_DC);

// For better pressure precision, we need to know the resistance
// between X+ and X- Use any multimeter to read it
// For the one we're using, its 300 ohms across the X plate
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);

void setup(void) {
  Serial.begin(9600);
  tft.begin(HX8357D);
  tft.fillScreen(HX8357_BLACK);
}

void loop(void) {
  // a point object holds x y and z coordinates
  TSPoint p = ts.getPoint();
  
  // we have some minimum pressure we consider 'valid'
  // pressure of 0 means no pressing!
  if (p.z > ts.pressureThreshhold) {
     Serial.print("X = "); Serial.print(p.x);
     Serial.print("\tY = "); Serial.print(p.y);
     Serial.print("\tPressure = "); Serial.println(p.z);
     tft.fillCircle(map(p.x,167,900,0,320),map(p.y,100,900,0,480),5,HX8357_RED);
  }

  delay(20);
}

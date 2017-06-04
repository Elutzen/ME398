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
#define _BRIGHT 7
Adafruit_HX8357 tft = Adafruit_HX8357(_CS,_DC);
short TFTbrightness = 127;
#define BRIGHTNESS_PWM_FREQ 7324

// For better pressure precision, we need to know the resistance
// between X+ and X- Use any multimeter to read it
// For the one we're using, its 300 ohms across the X plate
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);
TSPoint p;
bool isTouching;

/////// Define buttons and their actions  ////////
enum BtnType     {SINGLE,TOGGLE,ONPRESS,ONRELEASE}; 
enum BtnState    {RELEASED,PRESSED};
struct Button {
  int16_t x,y,w,h;
  char* label;
  uint16_t color;
  BtnType type;
  BtnState state = RELEASED, prevState = RELEASED;
  bool togState = 0;
  long millisLastChanged;

  // State Functions
  const inline bool pressed()      { return state==PRESSED; }
  const inline bool justPressed()  { return state==PRESSED && prevState == RELEASED; }
  const inline bool justReleased() { return state==RELEASED && prevState == PRESSED; }
  const inline bool released()     { return state==RELEASED; }
  const inline bool tapped()       { return (state==RELEASED) && (prevState==PRESSED) && (millis() - millisLastChanged < 500);}
  const inline bool toggledOn()    { return togState; }
  const inline bool stateChanged() { return state != prevState; }

  // Routines
  void update();
  const void draw();

  // CTOR
  Button(BtnType _type, int16_t _x, int16_t _y, int16_t _w, int16_t _h, char* _label, uint16_t _color) : 
    x(_x), 
    y(_y), 
    w(_w), 
    h(_h), 
    label(_label), 
    color(_color), 
    type(_type)
    {};
};

// Create buttons here
Button BtnDispense(BtnType::SINGLE, 80, 50, 150, 50, "Dispense",HX8357_GREEN);
Button BtnCancel  (BtnType::TOGGLE, 80, 110, 150, 50, "Cancel", HX8357_RED);
Button BtnRapid   (BtnType::SINGLE, 80, 170, 150, 50, "Rapid", HX8357_YELLOW);

/////////// HELPERS //////////////
void handleTouch() {
  // a point object holds x y and z coordinates
  p = ts.getPoint();
  p.x = map(p.x,TS_MINX,TS_MAXX,0,HX8357_TFTWIDTH);
  p.y = map(p.y,TS_MINY,TS_MAXY,0,HX8357_TFTHEIGHT);
  p.z = (p.z == 0 ? 0 : map(p.z,MINPRESSURE,MAXPRESSURE,0,100));  

  isTouching = (p.z > 0 && p.z < 100);
}

void updateButtons() {
  BtnDispense.update();
  BtnCancel.update();
  BtnRapid.update();
}

void drawAllButtons(bool force = false) {
  /* DISPENSE */
  if (BtnDispense.millisLastChanged > millis() - 300 || force) {
    BtnDispense.color = BtnDispense.justPressed() ? HX8357_BLUE: HX8357_GREEN;
    BtnDispense.draw();
  }

  /* CANCEL */
  if (BtnCancel.stateChanged() || force) {
    BtnCancel.color = BtnCancel.toggledOn() ? HX8357_BLUE : HX8357_RED;
    BtnCancel.draw();
  }

  /* RAPID */ 
  if (BtnRapid.stateChanged() || force) {
    BtnRapid.color = BtnRapid.pressed() ? HX8357_BLUE : HX8357_YELLOW;
    BtnRapid.draw();
  }
}

void drawReadout() {
  tft.setFont(&FreeSansBold24pt7b);
  tft.setCursor(20,350);
  tft.setTextColor(HX8357_WHITE);
  tft.setTextSize(2);
  tft.fillRect(0,275,320,85,HX8357_BLACK);
  tft.print(p.x/100.0,4);
  tft.drawCircle(p.x,p.y,3,HX8357_RED);
}

////////// MAIN CODE /////////////
void setup(void) {
  Serial.begin(115200);
  pinMode(_BRIGHT,OUTPUT); // PWM Brightness pin
  analogWriteFrequency(_BRIGHT,BRIGHTNESS_PWM_FREQ);
  tft.begin(HX8357D);
  tft.fillScreen(HX8357_BLACK);
  Serial.print(BtnDispense.justPressed());
  drawAllButtons(/*FORCE UPDATE*/ true);
  drawReadout();
}

void loop(void) {
  handleTouch();
  updateButtons();
  drawAllButtons();
  TFTbrightness = BtnCancel.toggledOn() ? 20 : 255;
  analogWrite(_BRIGHT,TFTbrightness); // Controll LCD brightness
  
  // we have some minimum pressure we consider 'valid'
  // pressure of 0 means no pressing!
  if (isTouching) {
     Serial.print("X = "); Serial.print(p.x);
     Serial.print("\tY = "); Serial.print(p.y);
     Serial.print("\tPressure = "); Serial.println(p.z);
     drawReadout();
  }
     
  delay(10);
}

///////// Button functions ////////////
void Button::update() {
  // Sync current state
  prevState = state;

  // Update Current State
  if ((isTouching) && (p.x > x && p.y > y && p.x < x+w && p.y < y+h))
  {
    state = PRESSED;
  } else {
    state = RELEASED;
  }

  if (prevState != state) {millisLastChanged = millis();}

  // Update toggle state
  if (type == TOGGLE)
  {
    if (justPressed())
    {
      togState = !togState;
    }
  }
}

const void Button::draw() {
  tft.fillRect(x,y,w,h,color);
  tft.setFont();
  tft.setCursor(x+5,y+5);
  tft.setTextColor(HX8357_BLACK);
  tft.setTextSize(3);
  tft.print(label);
}


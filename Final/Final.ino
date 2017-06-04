#include <avr/io.h>
#include <BasicStepperDriver.h>
#include <DRV8825.h>
#include <DRV8834.h>
#include <avr/wdt.h>
#include <Arduino.h>
#include <SoftwareSerial.h>
#include <SPI.h>

//Overall Control
char action = 'a';


//Spi Control
#define HWSERIAL Serial1


//Position Control Variables
#define anPos A20 // analog position
#define posUpper 255
#define posLower 252
#define sumNumber 10
#define DAC A22
#define dacStep 1
#define sec 500
int dacVal = 50000000;



//auger
// using a 200-step motor (most common)
// pins used are DIR, STEP, MS1, MS2, MS3 in that order
DRV8825 stepper(200, 8, 9);
#define ENBL 7
#define enable() digitalWrite(ENBL,LOW)
#define disable() digitalWrite(ENBL,HIGH);

//Voice coil setup
#define PWMstep 1
int PWMval = 360;
#define PWMpin 4

//Vibration Setup
#define vibeHz 50



void setup() {
  // serial set up
  Serial.begin(9600);
HWSERIAL.begin(9600);


  //Dac set up
  analogWriteResolution(10);
  analogWriteFrequency(4, 46875);
  Serial.println("START");



}

void loop() {
  //float val = getPosition();
  //Serial.println(val);
  //readCurrent();
  correctPosition();
  //analogWrite(PWMpin,400);
  switch (action) {
    case 'a' :
      //Serial.println("Current Action: Stopped\n" );
      disable();
      action = 'a';
      break;
  }
  //menu();
}



/*Position control
   Main Function correctPostion() returns the voice coil to the measuring position



*/
void correctPosition() {
  // moves voice coil into to measuring position
  float pos = getPosition();
  while (pos < posLower || pos > posUpper) {
    if (pos < posLower) {
      voltagePositionControl(-1);
      Serial.println("High");
    }
    if (pos > posUpper) {
      voltagePositionControl(1);
      Serial.println("Low");
      Serial.println(getPosition());
    }
    pos = getPosition();
    delay(sec);
  }
}

void voltagePositionControl(int dir) {
  switch (dir) {
    case 1:
      PWMval += PWMstep;
      analogWrite(PWMpin, PWMval);
      Serial.println("increase");
      Serial.println(PWMval);
      break;
    case -1:
      PWMval -= PWMstep;
      analogWrite(PWMpin, PWMval);
      Serial.println("decrease");
      Serial.println(PWMval);
      break;
  }
}

int getPosition() {
  //   returns the position of voice coil
  float loc = 0;
  int total = 0;
  for (int i = 0; i < sumNumber; i++) {
    int val =  analogRead(anPos);
    total += val;
  }
  loc = total / sumNumber;
  return loc;
}


/* End of Position Control



*/


/*Start of Auger Control



*/
void augerFast() {

}

void augerSlow() {

}

/* End of Auger



*/


/* Start of Vibration control




*/

void plateVibe() {
  float place  = 0.0;
  float twopi = 3.14159 * 2;
  while (1) {
    float val = sin(place) * 2000.0 + 2050.0;
    Serial.println(val);
    analogWrite(DAC, (int)val);
    place = place + 0.02;
    if (place >= twopi) {
      place = 0;
    }
    delayMicroseconds(vibeHz);
  }
}

/* End of Vibration



*/


/* Current Measurement and Mass calc



*/
float readCurrent() {
 HWSERIAL.flush();
 int incomingByte;
  if (HWSERIAL.available() > 0) {
    incomingByte = HWSERIAL.read();
    Serial.print("UART received: ");
    Serial.println(incomingByte, DEC);
  }
}

float calcMass() {

}



/* End of Current Measurement




*/

void serialRead() {
  if (Serial.available()) {
    //Serial.flush();
    char c = Serial.read();
    switch (c) {
      case 'a' :
        action = 'a';
        break;
      case 'b':
        action = 'b';
        break;
      case 'c':
        action = 'c';
        break;
      case'd':
        action = 'd';
        break;
    }
    menu();
  }
}


void menu() {
  Serial.println("a:stop ");
  Serial.println("b:auger");
  Serial.println("c:plate");
  Serial.println("d:measure");

}

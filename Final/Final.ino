#include <avr/io.h>
#include <BasicStepperDriver.h>
#include <DRV8825.h>
#include <DRV8834.h>
#include <avr/wdt.h>
#include <Arduino.h>
//#include <SoftwareSerial.h>
#include <SPI.h>

//Overall Control
//#define SUPRESS_OUTPUT

enum ACTION {IDLE, AUGERSLOW, AUGERFAST, MEASURE, DISPENSE} action = IDLE;

//Spi Control
#define HWSERIAL Serial1

//Position Control Variables
#define anPos A20 // analog position
#define posUpper 530
#define posLower 504
#define sumNumber 10
#define DAC A22
#define dacStep 1
#define sec 500
int dacVal = 50000000;

//auger
// using a 200-step motor (most common)
// pins used are DIR, STEP, MS1, MS2, MS3 in that order
DRV8825 stepper(200, 30, 29, 28);
#define AUGERFAST_RPM 300
#define AUGERSLOW_RPM 10
long rpm = 60;
int tempRpm = 0;
int numSteps = 0;

//Voice coil setup
#define PWMstep 5
int PWMval = 360;
#define PWMpin 4

//Vibration Setup
#define vibeHz 50

void setup() {
  // serial set up
  Serial.begin(9600);
HWSERIAL.begin(9600);

  //Auger setup
  stepper.disable();
  stepper.setRPM(rpm);
  stepper.setMicrostep(1);

  //Dac set up
  analogWriteResolution(10);
  analogWriteFrequency(4, 46875);

  //Wait for connection
  while (!Serial) {}
  Serial.println("START");
  menu();
}

void loop() {
  float val;
  readCommand();
  switch (action) {
    case IDLE :
      break;
    case AUGERSLOW:
      stepper.enable();
      augerSlow();
      break;
    case AUGERFAST:
      stepper.enable();
      augerFast();
      break;
    case MEASURE:
      val = getPosition();
#ifndef SUPRESS_OUTPUT
        Serial.println(val);
#endif
      //readCurrent();
      correctPosition();
      //analogWrite(PWMpin,400); 
      if (action == MEASURE) {action = DISPENSE;}
      break;
    case DISPENSE:
      plateVibe();
      break;
  }
}



/*Position control
   Main Function correctPostion() returns the voice coil to the measuring position

*/
void correctPosition() {
  // moves voice coil into to measuring position
  float pos = getPosition();
  int count = 0;
  while ((pos < posLower || pos > posUpper) && count < 30) {
    if (pos < posLower) {
      voltagePositionControl(-1);
#ifndef SUPRESS_OUTPUT
      Serial.println("High");
#endif
    }
    if (pos > posUpper) {
      voltagePositionControl(1);
#ifndef SUPRESS_OUTPUT
      Serial.println("Low");
      Serial.println(getPosition());
#endif
    }
    pos = getPosition();
    delay(sec);
    count++;

    // allow user to break out of loop
    if (readCommand()){break;}
  }
  // Turn it off
  analogWrite(PWMpin,0);
}

void voltagePositionControl(int dir) {
  switch (dir) {
    case 1:
      PWMval += PWMstep;
      analogWrite(PWMpin, PWMval);
#ifndef SUPRESS_OUTPUT
      Serial.println("increase");
      Serial.println(PWMval);
#endif
      break;
    case -1:
      PWMval -= PWMstep;
      analogWrite(PWMpin, PWMval);
#ifndef SUPPRESS_OUTPUT
      Serial.println("decrease");
      Serial.println(PWMval);
#endif
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
//auger
void takeSteps(int s) {
  stepper.setRPM(rpm);
  stepper.move(s);
}

void augerFast() {
  stepper.setRPM(AUGERFAST_RPM);
  stepper.move(195);
  //stepper.setRPM(AUGERSLOW_RPM);
  //stepper.move(5);
}

void augerSlow() {
  stepper.setRPM(AUGERSLOW_RPM);
  stepper.move(10);
}

/* End of Auger



*/


/* Start of Vibration control




*/

void plateVibe() {
    delay(20);
    analogWrite(PWMpin,100);
    delay(20);
    analogWrite(PWMpin,700);
}

/* End of Vibration



*/


/* Current Measurement and Mass calc



*/
/*float readCurrent() {
 HWSERIAL.flush();
 int incomingByte;
  if (HWSERIAL.available() > 0) {
    incomingByte = HWSERIAL.read();
    Serial.print("UART received: ");
    Serial.println(incomingByte, DEC);
  }
}*/

/*float calcMass() {

}*/



/* End of Current Measurement




*/

bool readCommand() {
  if (Serial.available()) {
    //Serial.flush();
    char c = Serial.read();
    switch (c) {
      case 'a' :
        Serial.println(F("Current Action: Stopped"));
        action = IDLE;
        break;
      case 'b' :
        Serial.println(F("Current Action: Auger (slow)"));
        action = AUGERSLOW;
        break;
      case 'c' :
        Serial.println(F("Current Action: Auger (fast)"));
        action = AUGERFAST;
        break;
      case 'd' :
        Serial.println(F("Current Action: Measuring"));
        Serial.println(F("Press q to quit"));
        action = MEASURE;
        break;
      case 'e' :
        Serial.println("Current Action: Dispensing ");
        action = DISPENSE;
        break;
      case 'f' :
        Serial.println("Enter Number of steps (int) [200 steps = 1 rev]:");
        while (!Serial.available()) {}
        numSteps = Serial.parseInt();
        stepper.enable();
        takeSteps(numSteps);
        stepper.disable();
        break;
      case 'h' :
        /*Serial.print("tare ");
        tare();*/
        break;
      case 'r' :
        Serial.println("Enter rpm (int) [0 - 300]:");
        while (!Serial.available()) {}
        rpm = Serial.parseInt();
        break;
    }
    analogWrite(PWMpin,0);
    stepper.disable();
    menu();
    return true;
  }
  return false;
}


void menu() {
  Serial.println();
  Serial.println(F("a: Stop"));
  Serial.println(F("b: Auger (slow)"));
  Serial.println(F("c: Auger (fast)"));
  Serial.println(F("d: Measure"));
  Serial.println(F("e: Dispense"));
}

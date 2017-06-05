#include <avr/io.h>
#include <BasicStepperDriver.h>
#include <DRV8825.h>
#include <DRV8834.h>
#include <avr/wdt.h>
#include <Arduino.h>
//#include <SoftwareSerial.h>
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
#define AUG_FASTRPM 300
#define AUG_SLOWRPM 10
long rpm = 60;
int tempRpm = 0;
int numSteps = 0;
// using a 200-step motor (most common)
// pins used are DIR, STEP, MS1, MS2, MS3 in that order
DRV8825 stepper(200, 30, 29);
#define ENBL 28
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

  //Auger setup
  pinMode(ENBL, OUTPUT);
  disable();
  stepper.setRPM(rpm);
  stepper.setMicrostep(1);

  //Dac set up
  analogWriteResolution(10);
  analogWriteFrequency(4, 46875);
  Serial.println("START");

}

void loop() {
  float val = getPosition();
  //Serial.println(val);
  //readCurrent();
  //correctPosition();
  //analogWrite(PWMpin,400);

  readCommand();
  switch (action) {
    case 'a' :
      disable();
      action = 'a';
      break;
    case 'k':
      augerSlow();
      break;
    case 'p':
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
//auger
void takeSteps(int s) {
  stepper.setRPM(rpm);
  stepper.move(s);
}

void augerFast() {
  stepper.setRPM(AUG_FASTRPM);
  stepper.move(10);
}

void augerSlow() {
  stepper.setRPM(AUG_SLOWRPM);
  stepper.move(10);
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

void readCommand() {
  if (Serial.available()) {
    //Serial.flush();
    char c = Serial.read();
    switch (c) {
      case 'a' :
        Serial.println("Current Action: Stopped\n" );
        /*setPWM(255);*/
        action = 'a';
        break;
      case 'b' :
        /*Serial.println("Set PWm (int)\n" );
        while (!Serial.available()) {}
        tempPWM = Serial.parseInt();
        if (tempPWM < 256) {
          setPWM(tempPWM);
        }
        else {
          Serial.println("Invalid int\n" );
        }
        break;*/
      case 'c' :
        /*Serial.print("current is ");
        Serial.println(getCurrent());*/
        break;
      case 'd' :
        /*Serial.print("position is ");
        Serial.println(getPosition());*/
        break;
      case 'e' :
        Serial.print("correcting position ");
        correctPosition();
        break;
      case 'f' :
        Serial.print("measure mass (in current/ not tared) ");
        /*correctPosition();
        temp = String(calcMass());
        Serial.println(temp);*/
        break;
      case 'g' :
        Serial.println("Number of steps (int)  200 steps = 1 rev\n");
        while (!Serial.available()) {}
        numSteps = Serial.parseInt();
        enable();
        takeSteps(numSteps);
        disable();
        break;
      case 'h' :
        /*Serial.print("tare ");
        tare();*/
        break;
      case 'i' :
        Serial.print("enable ");
        enable();
        break;
      case 'j' :
        Serial.print("disable ");
        disable();
        break;
      case 'k' :
        Serial.print("auger");
        enable();
        action = 'k';
        break;
      case 'l' :
        Serial.println("rpm (int)  ");
        while (!Serial.available()) {}
        rpm = Serial.parseInt();
        break;
      case 'm':
        enable();
        action = 'm';
        break;
    }
    menu();
  }
}


void menu() {
  Serial.println("a:stop ");
  Serial.println("k:auger");
  Serial.println("p:plate");
  Serial.println("m:measure");

}

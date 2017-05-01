#include <avr/io.h>
#include <A4988.h>
#include <BasicStepperDriver.h>
#include <DRV8825.h>
#include <DRV8834.h>
#include <avr/wdt.h>
#include <Arduino.h>
#include <SoftwareSerial.h>

int analogPin = 0;
int pwm = 0;
int abovePin = 13;
int belowPin = 12;
int tempPWM;
String temp;
int PWMtare;


//auger
char action = 'a';
long rpm = 10;
int tempRpm = 0;
int numSteps = 0;
void vibrationPlate();
void takeSteps(int);
// using a 200-step motor (most common)
// pins used are DIR, STEP, MS1, MS2, MS3 in that order
DRV8825 stepper(200, 8, 9);
#define ENBL 7
#define enable() digitalWrite(ENBL,LOW)
#define disable() digitalWrite(ENBL,HIGH);

void setup() {
  pinMode(4, OUTPUT);//voice coil direction
  pinMode(abovePin, INPUT); //phototransistor
  pinMode(belowPin, INPUT); //phototransistor
  Serial.begin(9600);
  digitalWrite(4, HIGH);

  pinMode(ENBL, OUTPUT);
  disable();
  Serial.begin(9600);
  stepper.setRPM(10);
  stepper.setMicrostep(1);
  //PWM Set Up (8kHz)
  DDRD |= (1 << DDD6);
  // PD6 is now an output
  OCR0A = 255;
  pwm = 255;
  // set PWM for 0% duty cycle (0 = 100 duty cycle, 255 = 0 duty);
  TCCR0A |= (1 << COM0A1);
  // set none-inverting mode
  TCCR0A |= (1 << WGM01) | (1 << WGM00);
  // set fast PWM Mode
  TCCR0B |= (1 << CS01) | (1 << CS00);
  // set prescaler to 8 and starts PWM
  menu();
}

void loop() {
  serialRead();
  switch (action) {
    case 'b':
      auger();
      break;
    case 'c':
      vibrationPlate();
      break;
  }
}

void setPWM(int val) {
  pwm = val;
  OCR0A = pwm;
}


void voicePWM(char dir) {
  //increase or decrease pwm by 1 step (increase dir = 1, decrease dir = 0, set to zero if dir = 0)
  if (dir == 1) {
    pwm --;
    OCR0A = pwm;
  }
  if (dir == 0) {
    pwm ++;
    OCR0A = pwm;
  }
  if (dir == 2) {
    pwm = 0;
    OCR0A = pwm;
  }
}

float getCurrent() {
  //reads the current from the voice coil, averages current for 20 readings and returns float of current
  float current = 0;
  int total = 0;
  int count = 0;
  for (int i = 0; i < 100; i++)
  {
    int val = analogRead(analogPin);
    total += val;
    count ++;
  }
  current = ((float)total) / ((float)count);
  return current;
}

void correctPosition() {
  //moves voice coil into reading position
  char done = 0;
  while (done == 0) {
    if (getPosition() == 1) {
      //exit loop at correct position
      delay(100);
      if (getPosition() == 1) {
        done = 1;
      }
    }
    if (getPosition() == 0) {
      // increase voltage
      voicePWM(1);
    }
    if (getPosition() == 2) {
      // decrease voltage
      voicePWM(0);
    }
    delay(100);
  }
  Serial.println("Done");
}

int getPosition() {
  //returns the position of the voice coil (0 = below, 1= measuring position, 2 = above)
  int location = 7;
  int above = digitalRead(abovePin);
  int below = digitalRead(belowPin);
  if (!above) {
    location = 2;
  }
  if (below && above) {
    location = 1;
  }
  if (!below) {
    location = 0;
  }
  Serial.println(location);
  return location;
}

void changeCurrent(int sign, float amount) {
  //changes the current in the direction of the sign(sign 1 or -1) and by the amount (amount)
}

void tare() {
  //corrects position and sets the tare current
  correctPosition();
  PWMtare = pwm;
  setPWM(255);
}

void measure() {
  //corrects position and measures current to find mass
  pwm = PWMtare;
  correctPosition();

}

float calcMass() {
  //takes in a current and returns a mass

  float curInt = getCurrent();
  float curV = (5 * (float)curInt) / 1024;
  float curr = curV / 1.604;
  return curr;
}



void dump() {

}

void dispense() {

}

void serialRead() {
  if (Serial.available()) {
    //Serial.flush();
    char c = Serial.read();
    switch (c) {
      case 'a' :
        Serial.println("Current Action: Stopped\n" );
        setPWM(255);
        disable();
        action = 'a';
        break;
      case 'b' :
        Serial.println("Set PWm (int)\n" );
        while (!Serial.available()) {}
        tempPWM = Serial.parseInt();
        if (tempPWM < 256) {
          setPWM(tempPWM);
        }
        else {
          Serial.println("Invalid int\n" );
        }
        break;
        while (Serial.available()) {
          Serial.read();
        }
      case 'c' :
        Serial.print("current is ");
        Serial.println(getCurrent());
        break;
      case 'd' :
        Serial.print("position is ");
        Serial.println(getPosition());
        break;
      case 'e' :
        Serial.print("correcting position ");
        correctPosition();
        break;
      case 'f' :
        Serial.print("measure mass (in current/ not tared) ");
        correctPosition();
        temp = String(calcMass());
        Serial.println(temp);
        break;
      case 'g' :
        Serial.println("Number of steps (int)  200 steps = 1 rev\n");
        while (!Serial.available()) {}
        numSteps = Serial.parseInt();
        enable();
        takeSteps(numSteps);
        break;
      case 'h' :
        Serial.print("tare ");
        tare();
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
        action = 'b';
        break;
      case 'l' :
        Serial.println("rpm (int)  ");
        while (!Serial.available()) {}
        rpm = Serial.parseInt();
        break;
      case 'm':
        enable();
        action = 'c';
        break;
    }
    menu();
  }
}

void menu() {
  Serial.println("a:stop ");
  Serial.println("b:set pwm ");
  Serial.println("c:get current ");
  Serial.println("d: get position ");
  Serial.println("e: correct position ");
  Serial.println("f: mass ");
  Serial.println("g: take steps ");
  Serial.println("h: tare");
  Serial.println("i:enable");
  Serial.println("j: disable");
  Serial.println("k: auger");
  Serial.println("l: change rpm");
  Serial.println("m: plate");


}


//auger
void takeSteps(int s) {
  stepper.setRPM(rpm);
  stepper.move(s);
  Serial.println("done");
}

void auger() {

  stepper.move(200);
  Serial.println("done");
}

//vibratory plate

void vibrationPlate() {
  unsigned rpm = 20;
  stepper.setRPM(rpm);
  rpm = 40;
  stepper.setRPM(rpm);
  long amp = 1;
  int t = 0;

  while (t < 10) {
    stepper.move(amp);
    stepper.move(-amp);
    t++;
  }
  rpm = 40;
  stepper.setRPM(rpm);
  amp = 1;
  stepper.move(amp);
  stepper.move(-amp);
}






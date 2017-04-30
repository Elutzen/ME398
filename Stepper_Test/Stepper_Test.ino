#include <A4988.h>
#include <BasicStepperDriver.h>
#include <DRV8825.h>
#include <DRV8834.h>
#include <avr/wdt.h>
#include <Arduino.h>
#include <SoftwareSerial.h>

#define SHOWMENU

// using a 200-step motor (most common)
// pins used are DIR, STEP, MS1, MS2, MS3 in that order
DRV8825 stepper(200, 8, 9);
#define ENBL 7
#define enable() digitalWrite(ENBL,LOW)
#define disable() digitalWrite(ENBL,HIGH);

char action = 'a';
long rpm = 10;
int tempRpm=0;
int numSteps = 0;
void auger();
void serialRead();
void vibrationAuger();
void vibrationPlate();
void printMenu();
void takeSteps(int);

void setup() {
  pinMode(ENBL,OUTPUT);
  disable();
  Serial.begin(9600);
  // Set target motor RPM to 10RPM
  stepper.setRPM(10);
  // Set full speed mode (microstepping also works for smoother hand movement
  stepper.setMicrostep(1);
  printMenu();
}

void loop() {
  serialRead();
  switch (action) {
    case 'a':
      disable();
      break;
    case 'b' :
      enable();
      auger();
      break;
    case 'c' :
      enable();
      vibrationAuger();
      break;
    case 'd' :
      enable();
      vibrationPlateSetup();
      break;
    case 'z' :
      enable();
      vibrationPlate();
      break;
    case 'f' :
      enable();
      takeSteps(numSteps);
      break;
    case 'h':
      enable();
      break;
  }
}

void auger() {
  stepper.setRPM(rpm);
  stepper.move(50);
}

void vibrationPlateSetup() {
  unsigned rpm = 20;
  stepper.setRPM(rpm);
  rpm = 80;
  stepper.setRPM(rpm);
  long amp = 1;
  int t = 0;

  while(t< 10){
     stepper.move(amp);
    stepper.move(-amp);
    t++;
  }
  action = 'z';
}

void vibrationPlate() {
  unsigned rpm = 40;
  stepper.setRPM(rpm);
  long amp = 1;
  stepper.move(amp);
  stepper.move(-amp);
}

void takeSteps(int s) {
  stepper.setRPM(rpm);
  stepper.move(s);
  action = 'a';
}

void vibrationAuger() {
  // 3 forward 2 back

  unsigned rpm = 20;
  stepper.setRPM(rpm);
  stepper.move(10);
  stepper.move(-2);
}

void serialRead() {
  if (Serial.available()) {
    //Serial.flush();
    char c = Serial.read();
    switch (c) {
      case 'a' :
        Serial.println("Current Action: Stopped\n" );
        action = 'a';
        break;
      case 'b' :
        Serial.println("Current Action: Auger\n" );
        action = 'b';
        break;
      case 'c' :
        Serial.println("Current Action: Vibrating Auger\n" );
        action = 'c';
        break;
      case 'd' :
        Serial.println("Current Action: Plate\n" );
        action = 'd';
        break;
       case 'e' :
        Serial.println("Give Rpm (int)\n" );
        while(!Serial.available()){}
        tempRpm = Serial.parseInt();
        if(tempRpm < 350){
          rpm = tempRpm;
        }
        else{
           Serial.println("Invalid int\n" );
        }
        break;
       case 'f' :
        Serial.println("Number of steps (int)  200 steps = 1 rev\n");
        while(!Serial.available()){}
        numSteps = Serial.parseInt();
        action = 'f';
        break;
       case 'g' : 
        Serial.println(rpm);
       break;
       case 'h':
         Serial.println("Current Action: Braked\n" );
         action = 'h';
         break;
      default :
        Serial.println("Invalid entry\n" );
    }
    printMenu();
    while(Serial.available()){Serial.read();}
  }
}

void printMenu() {
#ifdef SHOWMENU
  Serial.println("a) Stop\n");
  Serial.println("b) Auger-no rpm control\n");
  Serial.println("c) Vibration Axel-no rpm control\n");
  Serial.println("d) Plate-no rpm control\n");
  Serial.println("e) Set Rpm\n");
  Serial.println("f) Take Steps\n");
  Serial.println("g) Return Rpm\n");
  Serial.println("h) Brake\n");
#endif
}



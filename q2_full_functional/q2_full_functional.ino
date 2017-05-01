#include <avr/io.h>

int analogPin = 0;
int pwm = 0;
int abovePin = 12;
int belowPin = 13;
int tempPWM;

void setup() {
  pinMode(4, OUTPUT);//voice coil direction
  pinMode(abovePin, INPUT); //phototransistor
  pinMode(belowPin, INPUT); //phototransistor
  Serial.begin(9600);
  digitalWrite(4, HIGH);

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
  return location;
}

void changeCurrent(int sign, float amount) {
  //changes the current in the direction of the sign(sign 1 or -1) and by the amount (amount)
}

void tare() {
  //corrects position and sets the tare current
}

float measure() {
  //corrects position and measures current to find mass
  correctPosition();
  float curInt = getCurrent();
  float curV = ((float)curInt) / 1024;
  float curr = curV / 1.604;
  return curr;
  Serial.println(curr);
}

float calcMass(float I) {
  //takes in a current and returns a mass
  float massMeasured;
  return massMeasured;
}

void auger() {

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


}




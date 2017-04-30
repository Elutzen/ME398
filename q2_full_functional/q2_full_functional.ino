int analogPin = 0;

void setup() {
  pinMode(5, OUTPUT);//voice coil pwm
  pinMode(4, OUTPUT);//voice coil enable
  Serial.begin(9600);
  digitalWrite(4,HIGH);
}

void loop() {
  getCurrent();
  voicePWM();
}

void voicePWM() {
  //drives voice coil with pdm signal
    digitalWrite(5, HIGH);
    delayMicroseconds(1); // Approximately 50% duty cycle @ 5KHz
    digitalWrite(5, LOW);
    delayMicroseconds(10 - 5);
}

void getCurrent() {
  //reads the current from the voice coil
  int val = analogRead(analogPin);
  Serial.println(val);
}

void correctPosition() {
  //moves voice coil into reading position
}

void changeCurrent(int sign, float amount) {
  //changes the current in the direction of the sign(sign 1 or -1) and by the amount (amount)
}

void tare() {
  //corrects position and sets the tare current
}

void measure() {
  //corrects position and measures current to find mass
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




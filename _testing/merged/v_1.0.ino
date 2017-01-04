#include <Wire.h>
#include <LiquidCrystal.h>
#define set 1
#define reset 0

// initialise library variables.
// LiquidCrystal lcd(RS, EN, D4, D5, D6, D7);
LiquidCrystal lcd(49, 48, 53, 52, 51, 50);

int rpm = 0;                                        // holds the average rpm
byte pwm_in = A0;                                   // holds the pin for pwm input
byte old_col = 0, old_row = 0;                      // data persistance for lcd functions

struct typeMotor {
  byte pin;
  unsigned long oldTime;
  int rpmBuffer[10];
  byte pwm;
  byte last_pointer;
  byte bufferSize;
};
struct typeISR {
  byte pin;
  volatile byte flag;
  volatile byte counter;
};

typeISR isr = {2, 0, 0};
typeMotor motor = {3, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 0, 0, 0};

// start rpmBuffer logistics
void enqueue(struct typeMotor *motor, int value) {
  motor->rpmBuffer[motor->last_pointer] = value;
  motor->last_pointer = (motor->last_pointer >= 9) ? 0 : (motor->last_pointer + 1);
  motor->bufferSize = (motor->bufferSize < 9) ? (motor->bufferSize + 1) : 9;
}

int getAverageRPM(struct typeMotor *motor) {
  int sum = 0;
  byte start = (motor->last_pointer == 0) ? 9 : motor->last_pointer - 1;
  for (int i = 0; i < motor->bufferSize; i++) {
    sum += motor->rpmBuffer[start];
    start = (start == 0) ? 9 : (start - 1);
  }
  return (sum / (motor->bufferSize + 1));
}

int getBufferAverage(struct typeMotor *motor, byte checkback) {
  int average = 0;
  byte start = motor->last_pointer - checkback;
  start = (start < 0) ? (10 - start) : start;
  for (int i = start; i <= motor->last_pointer; i++) {
    average += motor->rpmBuffer[i];
    i = (i == 10) ? 0 : i;
  }
  return average / (motor->bufferSize + 1);
}

int stdDev(struct typeMotor *motor, byte checkback, int average) {
  int sum = 0;
  byte start = motor->last_pointer - checkback;
  start = (start < 0) ? (10 - start) : start;
  for (int i = start; i <= motor->last_pointer; i++) {
    sum += int(pow(motor->rpmBuffer[i] - average, 2));
    i = (i == 10) ? 0 : i;
  }
  return (sum / (motor->bufferSize + 1));
}

int getBufferSize(struct typeMotor *motor) {
  int deviation = 0;
  int checkback = 9;
  int average = 0;
  while (true) {
    average = getBufferAverage(motor, checkback);
    deviation = stdDev(motor, checkback, average);
    if (deviation <= (0.05 * average) || checkback == 0) return checkback;
    checkback -= 1;
  }
}
// end rpmBuffer logistics

// function to read valus from adxl
byte getAccelValues(byte address) {
  Wire.beginTransmission(0x68);
  Wire.write(address);
  Wire.endTransmission(false);
  Wire.requestFrom(0x68, 1, true);
  return Wire.read();
}

// clear garbage values w/o updating the entire display
void cleanLCD(byte end_col, byte end_row) {
  while (true) {
    old_col = (old_col >= 14) ? 0 : (old_col + 1);
    if (old_col == 0) {
      old_row = (old_row >= 3) ? 0 : (old_row + 1);
      lcd.setCursor(old_col, old_row);
    }
    if ((old_col == end_col) && (old_row == end_row)) return;
    lcd.print(" ");
  }
}

// function to display numbers on the lcdisplay.
// arguments: the number, the column of it's unit digit, the row,
// and the max possible no. of digits the value can have.
void displayLCD(int value, byte unit_col, byte unit_row, byte valueLen) {
  cleanLCD((unit_col - valueLen) + 1, unit_row);
  for (int i = valueLen - 1; i > 0; i--)  ((value / int(pow(10, i))) > 0) ? : lcd.print(" ");
  lcd.print(value);
}

// function to display string on the screen
void displayLCD(String s, byte start_col, byte start_row) {
  cleanLCD(start_col, start_row);
  old_col += s.length();
  lcd.print(s);
}

// interrupt service routine for pin 18
void interruptRoutine() {
  isr.counter++;
  isr.flag = set;
}

void setup() {
  Wire.begin();
  lcd.begin(16, 4);
  pinMode(pwm_in, INPUT);
  pinMode(motor.pin, OUTPUT);
  pinMode(isr.pin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(2), interruptRoutine, RISING);
}

void loop() {
  motor.pwm = analogRead(pwm_in) / 4;
  analogWrite(motor.pin, motor.pwm);
  if (isr.flag) {
    enqueue(&motor, ((60000) / (millis() - motor.oldTime)));
    motor.oldTime = millis();
    isr.counter = reset;
    isr.flag = reset;
    motor.bufferSize = getBufferSize(&motor);
    rpm = getAverageRPM(&motor);
  }
  //  noInterrupts();
  displayLCD("Motor RPM:", 0, 0);
  displayLCD(rpm, 14, 0, 3);
  displayLCD("Motor PWM:", 0, 1);
  displayLCD(motor.pwm, 14, 1, 3);
  displayLCD("X-Angle:", 0, 2);
  displayLCD(int(getAccelValues(0x3B)), 12, 2, 3);
  displayLCD("Y-Angle:", 0, 3);
  displayLCD(int(getAccelValues(0x3D)), 12, 3, 3);
  if ((millis() - motor.oldTime) > 3000)  {
    for (int i = 0; i < 10; i++) motor.rpmBuffer[i];
    motor.bufferSize = reset;
    rpm = 0;
  }
  //  interrupts();
}

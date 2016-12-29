#include<Wire.h>
#include <LiquidCrystal.h>
#define set 1
#define reset 0

// initialise library variables.
// LiquidCrystal lcd(RS, EN, D4, D5, D6, D7);
LiquidCrystal lcd(49, 48, 53, 52, 51, 50);

int avg_rpm = 0;
byte potPin = A0;
int potValue = 0;
int rpm_iterate = 0;
const int MPU_addr = 0x68; // I2C address
byte Xh = 0, Yh = 0, Xl = 0;

struct typeMotor {
  byte pin;
  unsigned long oldTime;
  int rpm[10];
  int timer[10];
};
struct typeISR {
  byte pin;
  volatile byte flag;
  volatile byte counter;
};

typeISR isr = {2, 0, 0};
typeMotor motor = {3, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}};

void getAccelValues() {
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x3B);  // starting with register 0x3B (ACCEL_XOUT_H)
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_addr, 3, true); // request a total of 3 registers
  Xh = Wire.read();
  Xl = Wire.read();
  Yh = Wire.read();
}

// function to display stuff on the lcdisplay
void displayLCD() {
  lcd.setCursor(2, 1);
  ((motor.rpm[0] / 100) > 0) ? : lcd.print(" ");
  ((motor.rpm[0] / 10) > 0) ? : lcd.print(" ");
  lcd.print(motor.rpm[0]);
  lcd.print(" ");
  lcd.print(" ");
  lcd.print(" ");
  lcd.print(" ");
  lcd.print(" ");
  lcd.print(" ");
  lcd.setCursor(11, 1);
  ((potValue / 100) > 0) ? : lcd.print(" ");
  ((potValue / 10) > 0) ? : lcd.print(" ");
  lcd.print(potValue);
  lcd.setCursor(1, 2);
  ((Xh / 100) > 0) ? : lcd.print(" ");
  ((Xh / 10) > 0) ? : lcd.print(" ");
  lcd.print(Xh);
  lcd.setCursor(6, 2);
  ((Yh / 100) > 0) ? : lcd.print(" ");
  ((Yh / 10) > 0) ? : lcd.print(" ");
  lcd.print(Yh);
}

// interrupt service routine for pin 18
void interruptRoutine() {
  isr.counter++;
  isr.flag = set;
}

void setup() {
  Wire.begin();
  lcd.begin(16, 4);
  pinMode(potPin, INPUT);
  pinMode(motor.pin, OUTPUT);
  pinMode(isr.pin, INPUT_PULLUP);
  getAccelValues();
  for (int i = 0; i < 10; i++) {
    motor.rpm[i] = 0;
  }
  attachInterrupt(digitalPinToInterrupt(2), interruptRoutine, RISING);
}

void loop() {
  getAccelValues();
  potValue = analogRead(potPin) / 4;
  analogWrite(motor.pin, potValue);
  if (isr.flag) {
    motor.rpm[0] = (60000) / (millis() - motor.oldTime);
    motor.oldTime = millis();
    isr.counter = reset;
    isr.flag = reset;
  }
  //  noInterrupts();
  displayLCD();
  if ((millis() - motor.oldTime) > 3000) for (int i = 0; i < 10; i++) motor.rpm[i];
  //  interrupts();
}

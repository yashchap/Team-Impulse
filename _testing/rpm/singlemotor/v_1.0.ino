#include <LiquidCrystal.h>
#define set 1
#define reset 0

// initialise library variables.
// LiquidCrystal lcd(RS, EN, D4, D5, D6, D7);
LiquidCrystal lcd(12, 11, 7, 6, 5, 4);           // uno

byte potPin = A0;
int potValue = 0;
int sum = 0;
int rpm_iterate = 0;
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

typeMotor motor = {3, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}};
typeISR isr = {2, 0, 0};

void setup() {
  lcd.begin(16, 4);
  //  Serial.begin(9600);
  pinMode(potPin, INPUT);
  pinMode(8, OUTPUT);
  digitalWrite(8, HIGH);
  pinMode(motor.pin, OUTPUT);
  pinMode(isr.pin, INPUT_PULLUP);
  for (int i = 0; i < 10; i++) {
    motor.rpm[i] = 0;
  }
  attachInterrupt(digitalPinToInterrupt(2), interruptRoutine, RISING);
}

void loop() {
  potValue = analogRead(potPin) / 4;
  // Serial.print(potValue);
  analogWrite(motor.pin, potValue);
  if (isr.flag) {
    motor.rpm[rpm_iterate] = (60000) / (millis() - motor.oldTime);
    motor.oldTime = millis();
    isr.counter = reset;
    isr.flag = reset;
    (rpm_iterate == 10) ? (rpm_iterate = 0) : (rpm_iterate++);
  }
  sum = 0;
  for (int i = 0; i < 10; i++) {
    sum += motor.rpm[i];
  }
  sum /= 10;
  noInterrupts();
  lcd.setCursor(2, 1);
  ((sum / 100) > 0) ? : lcd.print(" ");
  ((sum / 10) > 0) ? : lcd.print(" ");
  lcd.print(sum);
  lcd.setCursor(11, 1);
  ((potValue / 100) > 0) ? : lcd.print(" ");
  ((potValue / 10) > 0) ? : lcd.print(" ");
  lcd.print(potValue);
  for (int i = 0; i < 10; i++) if ((millis() - motor.oldTime) > 3000) motor.rpm[i];
  interrupts();
}

// interrupt service routine for pin 18
void interruptRoutine() {
  isr.counter++;
  isr.flag = set;
}


#include <LiquidCrystal.h>
#define reset 0
#define set 1

// initialise library variables.
// LiquidCrystal lcd(RS, EN, D4, D5, D6, D7);
// LiquidCrystal lcd(12, 11, 5, 4, 3, 2);
LiquidCrystal lcd(49, 48, 53, 52, 51, 50);

byte lcdRow = 0;                              // used to switch rows on the lcdisplay.
byte potPin = A0;                             // holds the pin that is attached to potentiometer data line.
int potValue = 0;                             // holds value read from potentiometer.
struct typeMotor {
  byte pin;
  unsigned long oldTime;
  int rpm;
};
struct typeISR {
  byte pin;
  volatile byte flag;
  volatile byte counter;
};

typeMotor motor[] = {{2, 0, 0}, {3, 0, 0}, {4, 0, 0}, {5, 0, 0}};
typeISR isr[] = {{18, 0, 0}, {19, 0, 0}, {20, 0, 0}, {21, 0, 0}};

void setup() {
  lcd.begin(16, 4);
  pinMode(potPin, INPUT);
  for (int i = 0; i < 4; i++) {
    pinMode(motor[i].pin, OUTPUT);
  }
  for (int i = 0; i < 4; i++) {
    pinMode(isr[i].pin, INPUT_PULLUP);
  }
  attachInterrupt(digitalPinToInterrupt(18), interruptRoutine18, RISING);
  attachInterrupt(digitalPinToInterrupt(19), interruptRoutine19, RISING);
  attachInterrupt(digitalPinToInterrupt(20), interruptRoutine20, RISING);
  attachInterrupt(digitalPinToInterrupt(21), interruptRoutine21, RISING);
}

void loop() {
  potValue = analogRead(potPin) / 4;
  for (int i = 0; i < 4; i++) {
    if (potValue > 200) analogWrite(motor[i].pin, 200);
    else analogWrite(motor[i].pin, potValue);
  }
  for (int i = 0; i < 4; i++) {
    if (isr[i].flag) {
      motor[i].rpm = (60000) / (millis() - motor[i].oldTime);
      motor[i].oldTime = millis();
      isr[i].counter = reset;
      isr[i].flag = reset;
    }
  }
  // print on the lcd screen. testing not done.
  if ((millis() % 150) == 0) {
    if (potValue < 200) {
      lcd.setCursor(1, lcdRow);
      lcd.print(" ");
      ((motor[lcdRow].rpm / 100) > 0) ? : lcd.print(" ");
      ((motor[lcdRow].rpm / 10) > 0) ? : lcd.print(" ");
      lcd.print(motor[lcdRow].rpm);
    }
    else {
      lcd.setCursor(1, lcdRow);
      lcd.print("700+");
    }
    (lcdRow == 3) ? (lcdRow = 0) : (lcdRow++);
  }
  lcd.setCursor(11, 0);
  ((potValue / 100) > 0) ? : lcd.print(" ");
  ((potValue / 10) > 0) ? : lcd.print(" ");
  lcd.print(potValue);
  for (int i = 0; i < 4; i++) if ((millis() - motor[i].oldTime) > 3000) motor[i].rpm = 0;
}

// interrupt service routine for pin 18
void interruptRoutine18() {
  isr[0].counter++;
  isr[0].flag = set;
}

// interrupt service routine for pin 19
void interruptRoutine19() {
  isr[1].counter++;
  isr[1].flag = set;
}

// interrupt service routine for pin 20
void interruptRoutine20() {
  isr[2].counter++;
  isr[2].flag = set;
}

// interrupt service routine for pin 21
void interruptRoutine21() {
  isr[3].counter++;
  isr[3].flag = set;
}

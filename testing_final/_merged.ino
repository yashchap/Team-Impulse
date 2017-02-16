// ps2x lib
#include <PS2X_lib.h>
PS2X ps2x;

// lcd lib
#include <LiquidCrystal.h>
LiquidCrystal lcd(34, 30, 28, 26, 24, 22);


#define PS2_DAT        13
#define PS2_CMD        12
#define PS2_SEL        11
#define PS2_CLK        10
#define pressures   false
#define rumble      false

// data structure for a motor controlled by one of our H-bridges.
struct typeMotor {
  byte pwmPin;
  byte dirPin;
};

// all 4 motors of the bot's base.
typeMotor base_motor[4];

int pneumatic_pin = 49;                 // R5

byte old_col = 0, old_row = 0;          // data persistance for lcd functions

byte minutes = 3, seconds = 0;
int last_time = 1000;
int current_time = 0;

const int act_pwm = 5;                  // H4
const int act_dir = 40;

const int upper_pwm = 4;                // H3
int upSpeed = 50;                       // pwm for upper motor is written through this variable

const int xpin = A3;                    // x-axis of the accelerometer
const int ypin = A2;                    // y-axis of the accelerometer
const int zpin = A1;                    // z-axis of the accelerometer

void setup() {
  // begin lcd communications
  lcd.begin(16, 4);

  Serial.begin(57600);

  // set pin modes
  pinMode(upper_pwm, OUTPUT);
  pinMode(pneumatic_pin, OUTPUT);
  pinMode(xpin, INPUT);
  pinMode(ypin, INPUT);
  pinMode(zpin, INPUT);
  pinMode(act_pwm, OUTPUT);
  pinMode(act_dir, OUTPUT);
  digitalWrite(act_pwm, LOW);
  digitalWrite(act_dir, HIGH);

  // initialize base motors. pwm pins = 6, 7, 8, 9. dir pins = 42, 44, 46, 48.
  for (int i = 0; i < 4; i++) {
    base_motor[i].pwmPin = 6 + i;
    pinMode(base_motor[i].pwmPin, OUTPUT);
    base_motor[i].dirPin = 42 + (i * 2);
    pinMode(base_motor[i].dirPin, OUTPUT);
  }

  // wait for ps2 to boot
  delay(300);
  configurePS2X();
}

void loop() {
  // read gamepad. call atleast once a second
  ps2x.read_gamepad();

  // update lcd
  updateLCD();

  if (ps2x.Button(PSB_PAD_UP)) {                                            // up - move actuator up
    moveActuator(HIGH, HIGH);
  }
  if (ps2x.Button(PSB_PAD_RIGHT)) {                                         // right - increase upper pwm
    upSpeed++;
    setUpperPwm();
    delay(15);
  }
  if (ps2x.Button(PSB_PAD_LEFT)) {                                          // left - decrease upper pwm
    upSpeed--;
    setUpperPwm();
    delay(15);
  }
  if (ps2x.Button(PSB_PAD_DOWN)) {                                          // down - move actuator down
    moveActuator(HIGH, LOW);
  }
  if (!ps2x.Button(PSB_PAD_UP) && !ps2x.Button(PSB_PAD_DOWN)) {
    moveActuator(LOW, LOW);
  }

  if (ps2x.ButtonPressed(PSB_TRIANGLE)) {
    calibrationDance();
  }
  if (ps2x.ButtonPressed(PSB_CIRCLE)) {                                     // o - reset upper pwm
    upSpeed = 30;
    setUpperPwm();
  }
  if (ps2x.NewButtonState(PSB_CROSS)) {                                     // x - pneumatic control
    //will be TRUE if button was JUST pressed OR released
    fireDisc();
  }

  if (ps2x.Button(PSB_R1)) {
    int RX = ps2x.Analog(PSS_RX);
    Serial.println(RX);
    if (RX == 123) {
      Serial.print("Stopped at ");
      Serial.println(123 - RX);
      stopBot();
    }
    else if (RX > 123) {
      Serial.print("Right at ");
      walkRight((RX - 123) * (100.000 / 123.000));
    }
    else if (RX < 123) {
      Serial.print("Left at ");
      walkLeft((123 - RX) * (100.000 / 123.000));
    }
  }

  if (minutes != 0 || seconds != 0) keepTime();
  delay(50);
}

/*-------| PS2X |--------------------------------------------------------------------------*/
// function stolen from setup of PS2X_Example.
void configurePS2X() {
  int error = 0;
  byte type = 0;
  byte vibrate = 0;

  error = ps2x.config_gamepad(PS2_CLK, PS2_CMD, PS2_SEL, PS2_DAT, pressures, rumble);
  if (error == 0) {
    Serial.println("Found Controller, configured successfully.");
    Serial.print("pressures = ");
    if (pressures)
      Serial.println("true ");
    else
      Serial.println("false");
    Serial.print("rumble = ");
    if (rumble)
      Serial.println("true)");
    else
      Serial.println("false");
  }
  else if (error == 1)
    Serial.println("No controller found, check wiring.");
  else if (error == 2)
    Serial.println("Controller found but not accepting commands.");
  else if (error == 3)
    Serial.println("Controller refusing to enter Pressures mode, may not support it. ");

  type = ps2x.readType();
  switch (type) {
    case 0:
      Serial.println("Unknown Controller type found ");
      break;
    case 1:
      Serial.println("DualShock Controller found ");
      break;
    case 2:
      Serial.println("GuitarHero Controller found ");
      break;
    case 3:
      Serial.println("Wireless Sony DualShock Controller found ");
      break;
  }
  if (error == 1) //skip loop if no controller found
    return;
}


/*-------| Pneumatic Valve |---------------------------------------------------------------*/
int pneumatic_out;
void fireDisc() {
  pneumatic_out = (pneumatic_out == LOW) ? HIGH : LOW;
  digitalWrite(pneumatic_pin, pneumatic_out);
}

/*-------| Upper PWM |---------------------------------------------------------------------*/
void setUpperPwm() {
  analogWrite(upper_pwm, upSpeed);
}

/*-------| LCD |---------------------------------------------------------------------------*/

/*
   |0|0|0|0|0|0|0|0|0|0|1|1|1|1|1|1|
  c|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|
  r  ---------------------------------
  0|M|o|t|o|r| |P|W|M|:| | |#|#|#| |
  1| | | |X| | | | |Y| | | | |Z| | |
  2| |#|#|#|#| |#|#|#|#| |#|#|#|#| |
  3| | |T|i|m|e|r|:| | |#|:|#|#| | |
*/

void keepTime() {
  current_time = millis() % 1000;
  if ((current_time) < last_time) {
    seconds = (seconds != 0) ? (seconds - 1) : 59;
    if (seconds == 59) minutes = (minutes != 0) ? (minutes - 1) : 59;
  }
  last_time = current_time;
}

void updateLCD() {
  int x_val = analogRead(xpin), y_val = analogRead(ypin), z_val = analogRead(zpin);
  displayLCD("Motor PWM:", 0, 0);
  displayLCD(upSpeed, 14, 0, 3);
  displayLCD("X", 3, 1);
  displayLCD("Y", 8, 1);
  displayLCD("Z", 13, 1);
  displayLCD(x_val, 4, 2, 4);
  displayLCD(y_val, 9, 2, 4);
  displayLCD(z_val, 14, 2, 4);
  displayLCD("Timer:", 2, 3);
  displayLCD(minutes, seconds, 9, 3, "t");
}

void serialPrint() {
  Serial.print("Motor PWM:  ");
  Serial.print(upSpeed);
  Serial.print(" | X: ");
  Serial.print(analogRead(xpin));
  Serial.print(" | Y: ");
  Serial.print(analogRead(ypin));
  Serial.print(" | Z: ");
  Serial.print(analogRead(zpin));
  Serial.print(" | Timer- ");
  Serial.println("#:##");                                        // Timer dummy
}

// clear garbage values w/o updating the entire display
void cleanLCD(byte end_col, byte end_row) {
  while (true) {
    old_col = (old_col + 1) % 16;
    if (old_col == 0) {
      old_row = (old_row + 1) % 4;
      lcd.setCursor(old_col, old_row);
    }
    if (old_col != 0) lcd.print(" ");
    if ((old_col == end_col) && (old_row == end_row)) {
      return;
    }
    //    delay(250);
  }
}

// function to display numbers on the lcdisplay.
// arguments: the number, the column of it's unit digit, the row,
// and the max possible no. of digits the value can have.
void displayLCD(int value, byte unit_col, byte unit_row, byte valueLen) {
  cleanLCD((unit_col - valueLen) + 1, unit_row);
  for (int i = valueLen - 1; i > 0; i--)  ((value / int(pow(10, i))) > 0) ? : lcd.print(" ");
  old_col = unit_col + 1;
  lcd.print(value);
}

// function to display string on the screen
void displayLCD(String s, byte start_col, byte start_row) {
  cleanLCD(start_col, start_row);
  old_col += s.length();
  lcd.print(s);
}

// function to display some timer on the screen
void displayLCD(int minutes, int seconds, byte start_col, byte start_row, String t) {
  cleanLCD(start_col, start_row);
  old_col += 5;
  (minutes < 10) ? lcd.print("0") : NULL;
  lcd.print(minutes);
  lcd.print(":");
  (seconds < 10) ? lcd.print("0") : NULL;
  lcd.print(seconds);
}

/*-------| Linear Actuator |---------------------------------------------------------------*/
void moveActuator(int pwm, int dir) {
  digitalWrite(act_pwm, pwm);
  digitalWrite(act_dir, dir);
}

/*-------| Bot Base. |---------------------------------------------------------------------*/
// stop the bot.
void stopBot() {
  for (int i = 0; i < 4; i++) {
    digitalWrite(base_motor[i].dirPin, HIGH);
    analogWrite(base_motor[i].pwmPin, 0);
  }
}

// move the bot in the right direction.
void walkRight(int pwm) {
  Serial.println(pwm);
  for (int i = 0; i < 4; i++) {
    digitalWrite(base_motor[i].dirPin, HIGH);
    analogWrite(base_motor[i].pwmPin, pwm);
  }
}

// move the bot in left direction.
void walkLeft(int pwm) {
  Serial.println(pwm);
  for (int i = 0; i < 4; i++) {
    digitalWrite(base_motor[i].dirPin, LOW);
    analogWrite(base_motor[i].pwmPin, pwm);
  }
}

void calibrationDance() {
  for (int j = 0; j < 3; j++) {
    digitalWrite(base_motor[0].dirPin, HIGH);
    digitalWrite(base_motor[1].dirPin, HIGH);
    digitalWrite(base_motor[2].dirPin, LOW);
    digitalWrite(base_motor[3].dirPin, LOW);
    for (int i = 0; i < 4; i++) analogWrite(base_motor[i].pwmPin, 150);
    delay(1000);
    digitalWrite(base_motor[0].dirPin, LOW);
    digitalWrite(base_motor[1].dirPin, LOW);
    digitalWrite(base_motor[2].dirPin, HIGH);
    digitalWrite(base_motor[3].dirPin, HIGH);
    for (int i = 0; i < 4; i++) analogWrite(base_motor[i].pwmPin, 150);
    delay(1000);
  }
  stopBot();
}

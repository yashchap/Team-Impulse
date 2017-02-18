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

// data structure for anything controlled by one of our H-bridges.
struct hBridge {
  byte pwmPin;
  byte dirPin;
};

// pnuematic output
int pneumatic_out;

// all 4 motors of the bot's base.
hBridge base_motor[4];                  // 6, 7 - L - H2; 8, 9 - R - H1;

// adxl variables.
int z_sum = 0, z_quant = 0;             // for average.
int z_val = 0;                          // actual value

// pid stuff
int walk_direction = HIGH;
byte walk_pwm = 0;
byte zero_pos = 123;
const float Kp = 5.0;   // Kp value that you have to change
const float Kd = 3.0;  // Kd value that you have to change
const int setPoint = 35;    // Middle point of sensor array
const int baseSpeed = 120;    // Base speed for your motors
const int maxSpeed = 220;   // Maximum speed for your motors
int positionVal = 0;
int rightMotorSpeed = 0;
int leftMotorSpeed = 0;

const byte rx = 0;    // Defining pin 0 as Rx
const byte tx = 1;    // Defining pin 1 as Tx
const byte serialEn1 = 14;
const byte serialEn2 = 15;
const byte jPulse1 = 31;
const byte jPulse2 = 37;

int lastError = 0;    // Declare a variable to store previous error


int pneumatic_pin = 49;                 // R5

byte old_col = 0, old_row = 0;          // data persistance for lcd functions

byte minutes = 3, seconds = 0;
int last_time = 1000;
int current_time = 0;

// define hBridge for actuator movement.
const int act_pwm = 5;                  // H4
const int act_dir = 40;

const int upper_pwm = 4;                // H3
int upSpeed = 50;                       // pwm for upper motor is written through this variable

const int xpin = A3;                    // x-axis of the accelerometer
const int ypin = A2;                    // y-axis of the accelerometer
const int zpin = A1;                    // z-axis of the accelerometer

void setup() {
  // lsa stuff
  pinMode(serialEn1, OUTPUT);  // Setting serialEn as digital output pin
  pinMode(serialEn2, OUTPUT);  // Setting serialEn as digital output pin
  pinMode(jPulse1, INPUT);  // Setting junctionPulse as digital input pin
  pinMode(jPulse2, INPUT);  // Setting junctionPulse as digital input pin

  // Setting initial condition of serialEn pin to HIGH
  digitalWrite(serialEn1, HIGH);
  digitalWrite(serialEn2, HIGH);

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

  // initialize base motors. pwm pins = 6, 7, 8, 9. dir pins = 22, 23, 24, 25.
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
  // update lcd
  updateLCD();

  // check ps2x state, and control via it.
  doPS2XStuff();

  // walk with pid
  pidWalk();

  // read adxl values.
  readADXL();

  // do stuff that depends on some internal clocktime.
  doTimedStuff(checkTime());
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

void doPS2XStuff() {
  // read gamepad. call atleast once a second
  ps2x.read_gamepad();
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

  if (ps2x.ButtonPressed(PSB_TRIANGLE)) {                                     // o - reset upper pwm
    stopBot();
  }
  if (ps2x.ButtonPressed(PSB_CIRCLE)) {                                     // o - reset upper pwm
    upSpeed = 30;
    setUpperPwm();
  }
  if (ps2x.NewButtonState(PSB_CROSS)) {                                     // x - pneumatic control
    //will be TRUE if button was JUST pressed OR released
    fireDisc();
  }

  // when L1 is pressed, enable left analog stick values to control the base motors..
  if (ps2x.Button(PSB_R1)) {
    int RX = ps2x.Analog(PSS_RX);
    if (RX == zero_pos) {
      Serial.print("Stopped at ");
      Serial.println(zero_pos - RX);
      stopBot();
    }
    else if (RX > zero_pos) {
      Serial.print("Right :: ");
      Serial.print(RX);
      Serial.print(" :: ");
      walkRight((RX - zero_pos) * (120.000 / (255.000 - (float)zero_pos)));
    }
    else if (RX < zero_pos) {
      Serial.print("Left :: ");
      Serial.print(RX);
      Serial.print(" :: ");
      walkLeft((zero_pos - RX) * (120.000 / (float)zero_pos));
    }
  }
}

/*-------| ADXL |--------------------------------------------------------------------------*/
void readADXL() {
  z_sum += analogRead(zpin);
  z_quant++;
}

void setADXLValue() {
  if (z_quant == 0) return;
  z_val = z_sum / z_quant;
  z_sum = 0;
  z_quant = 0;
}

/*-------| Pneumatic Valve |---------------------------------------------------------------*/
void fireDisc() {
  pneumatic_out = (pneumatic_out == LOW) ? HIGH : LOW;
  digitalWrite(pneumatic_pin, pneumatic_out);
}

/*-------| Upper PWM |---------------------------------------------------------------------*/
void setUpperPwm() {
  analogWrite(upper_pwm, upSpeed);
}

/*-------| Timer |-------------------------------------------------------------------------*/
void doTimedStuff(boolean isTime) {
  if (!isTime) return;
  updateTimer();
  setADXLValue();
}

boolean checkTime() {
  current_time = millis() % 1000;
  boolean timePulse = (current_time < last_time) ? true : false;
  last_time = current_time;
  return timePulse;
}

void updateTimer() {
  if (minutes == 0 && seconds == 0) return;
  seconds = (seconds != 0) ? (seconds - 1) : 59;
  if (seconds == 59) minutes = (minutes != 0) ? (minutes - 1) : 59;
}

/*-------| LCD |---------------------------------------------------------------------------*/

void updateLCD() {
/*
   |0|0|0|0|0|0|0|0|0|0|1|1|1|1|1|1|
  c|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|
  r  ---------------------------------
  0|M|o|t|o|r| |P|W|M|:| | |#|#|#| |
  1| | | |X| | | | |Y| | | | |Z| | |
  2| |#|#|#|#| |#|#|#|#| |#|#|#|#| |
  3| | |T|i|m|e|r|:| | |#|:|#|#| | |
*/
  displayLCD("Motor PWM:", 0, 0);
  displayLCD(upSpeed, 14, 0, 3);
  displayLCD("X", 3, 1);
  displayLCD("Y", 8, 1);
  displayLCD("Z", 13, 1);
  displayLCD(z_val, 14, 2, 4);
  displayLCD("Timer:", 2, 3);
  displayLCD(minutes, seconds, 9, 3, "t");
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
  walk_pwm = 0;
  walk_direction = LOW;
}

// move the bot in the right direction.
void walkRight(int pwm) {
  Serial.println(pwm);
  walk_pwm = pwm;
  walk_direction = HIGH;
}

// move the bot in left direction.
void walkLeft(int pwm) {
  Serial.println(pwm);
  walk_pwm = pwm;
  walk_direction = LOW;
}

/*-------| PSD |--------------------------------------------------------------------------*/
void pidWalk() {
  if (walk_pwm == 0) {
    for (int i = 0; i < 4; i++) analogWrite(base_motor[i].pwmPin, walk_pwm);
    return;
  }
  switch (walk_direction) {
    case LOW:
      digitalWrite(serialEn1, LOW);
      while (Serial.available() <= 0);
      Serial.print("x: ");
      positionVal = Serial.read();
      Serial.println(positionVal);
      digitalWrite(serialEn1, HIGH);
      break;
    case HIGH:
      digitalWrite(serialEn2, LOW);
      //      Serial.println("-");
      while (Serial.available() <= 0);
      Serial.print("x: ");
      positionVal = Serial.read();
      Serial.println(positionVal);
      digitalWrite(serialEn2, HIGH);
      break;
  }

  // If no line is detected, stay at the position
  if (positionVal == 255) walk_pwm = 0;

  // Else if line detected, calculate the motor speed and apply
  else {
    int error = positionVal - setPoint;   // Calculate the deviation from position to the set point
    int motorSpeed = Kp * (float)error + Kd * (float)(error - lastError);   // Applying formula of PID
    lastError = error;    // Store current error as previous error for next iteration use

    // Adjust the motor speed based on calculated value
    // You might need to interchange the + and - sign if your robot move in opposite direction
    rightMotorSpeed = walk_pwm - motorSpeed;
    leftMotorSpeed = walk_pwm + motorSpeed;

    // limit the values to a specific range (0 <= rms/lms <= maxSpeed)
    rightMotorSpeed = (rightMotorSpeed > maxSpeed) ? maxSpeed : (rightMotorSpeed < 0 ? 0 : rightMotorSpeed);
    leftMotorSpeed = (leftMotorSpeed > maxSpeed) ? maxSpeed : (leftMotorSpeed < 0 ? 0 : leftMotorSpeed);

    // Writing the motor speed value as output to hardware motor
    switch (walk_direction) {
      case HIGH:
        for (int i = 0; i < 2; i++) {
          digitalWrite(base_motor[i].dirPin, walk_direction);
          analogWrite(base_motor[i].pwmPin, leftMotorSpeed);
        }
        for (int i = 2; i < 4; i++) {
          digitalWrite(base_motor[i].dirPin, walk_direction);
          analogWrite(base_motor[i].pwmPin, rightMotorSpeed);
        }
        break;
      case LOW:
        for (int i = 0; i < 2; i++) {
          digitalWrite(base_motor[i].dirPin, walk_direction);
          analogWrite(base_motor[i].pwmPin, rightMotorSpeed);
        }
        for (int i = 2; i < 4; i++) {
          digitalWrite(base_motor[i].dirPin, walk_direction);
          analogWrite(base_motor[i].pwmPin, leftMotorSpeed);
        }
        break;
    }
  }
}

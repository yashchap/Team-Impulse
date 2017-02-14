#include <PS2X_lib.h>  //for v1.6
PS2X ps2x;

#define PS2_DAT        13
#define PS2_CMD        11
#define PS2_SEL        10
#define PS2_CLK        12
#define pressures   false
#define rumble      false

int walk_direction = HIGH;
byte walk_pwm = 0;

// pid stuff.
const float Kp = 3.67;   // Kp value that you have to change
const float Kd = 1.3;  // Kd value that you have to change
const int setPoint = 35;    // Middle point of sensor array
const int baseSpeed = 60;    // Base speed for your motors
const int maxSpeed = 150;   // Maximum speed for your motors

const byte rx = 0;    // Defining pin 0 as Rx
const byte tx = 1;    // Defining pin 1 as Tx
const byte serialEn = 2;    // Connect UART output enable of LSA08 to pin 2
const byte junctionPulse = 4;   // Connect JPULSE of LSA08 to pin 4

int lastError = 0;    // Declare a variable to store previous error

// data structure for a motor controlled by one of our H-bridges.
struct typeMotor {
  byte pwmPin;
  byte dirPin;
};

// all 4 motors of the bot's base.
typeMotor base_motor[4];

void setup() {
  // lsa stuff
  pinMode(serialEn, OUTPUT);  // Setting serialEn as digital output pin
  pinMode(junctionPulse, INPUT);  // Setting junctionPulse as digital input pin

  // Setting initial condition of serialEn pin to HIGH
  digitalWrite(serialEn, HIGH);

  Serial.begin(57600);

  // initialize base motors. pwm pins = 6, 7, 8, 9. dir pins = 22, 23, 24, 25.
  for (int i = 0; i < 4; i++) {
    base_motor[i].pwmPin = 6 + i;
    pinMode(base_motor[i].pwmPin, OUTPUT);
    base_motor[i].dirPin = 22 + i;
    pinMode(base_motor[i].dirPin, OUTPUT);
  }
  delay(300);  //added delay to give wireless ps2 module some time to startup, before configuring it
  configurePS2X();
}

void loop() {
  ps2x.read_gamepad(); // read controller.
  // when L1 is pressed, enable left analog stick values to control the base motors..
  if (ps2x.Button(PSB_R1)) {
    int RX = ps2x.Analog(PSS_RX);
    if (RX == 123) {
      Serial.print("Stopped at ");
      Serial.println(123 - RX);
      stopBot();
    }
    else if (RX > 123) {
      Serial.print("Right :: ");
      Serial.print(RX);
      Serial.print(" :: ");
      walkRight((RX - 123) * (60.000 / 132.000));
      //      pidWalk();
    }
    else if (RX < 123) {
      Serial.print("Left :: ");
      Serial.print(RX);
      Serial.print(" :: ");
      walkLeft((123 - RX) * (60.000 / 123.000));
    }
  }
  if (walk_direction == HIGH) pidWalk();
  delay(50);
}
/*--------------------------------------------------------| PID |--------------------------------------------------------*/

void pidWalk() {
  digitalWrite(serialEn, LOW);  // Set serialEN to LOW to request UART data
  while (Serial.available() <= 0);  // Wait for data to be available
  int positionVal = Serial.read();    // Read incoming data and store in variable positionVal
  digitalWrite(serialEn, HIGH);   // Stop requesting for UART data

  // If no line is detected, stay at the position
  if (positionVal == 255) walk_pwm = 0;

  // Else if line detected, calculate the motor speed and apply
  else {
    int error = positionVal - setPoint;   // Calculate the deviation from position to the set point
    int motorSpeed = Kp * (float)error + Kd * (float)(error - lastError);   // Applying formula of PID
    lastError = error;    // Store current error as previous error for next iteration use

    // Adjust the motor speed based on calculated value
    // You might need to interchange the + and - sign if your robot move in opposite direction
    int rightMotorSpeed = walk_pwm - motorSpeed;
    int leftMotorSpeed = walk_pwm + motorSpeed;

    // If the speed of motor exceed max speed, set the speed to max speed
    if (rightMotorSpeed > maxSpeed) rightMotorSpeed = maxSpeed;
    if (leftMotorSpeed > maxSpeed) leftMotorSpeed = maxSpeed;

    // If the speed of motor is negative, set it to 0
    if (rightMotorSpeed < 0) rightMotorSpeed = 0;
    if (leftMotorSpeed < 0) leftMotorSpeed = 0;

    // Writing the motor speed value as output to hardware motor
    //    Serial.print(rightMotorSpeed);
    //    Serial.print("  |  lms  ");
    //    Serial.println(leftMotorSpeed);
    if (walk_pwm == 0) {
      for (int i = 0; i < 4; i++) analogWrite(base_motor[i].pwmPin, walk_pwm);
    }
    else {
      Serial.print("  pv  ");
      Serial.print(positionVal);
      Serial.print("  ||  ");
      Serial.print("  |  ms  ");
      Serial.print(motorSpeed);
      Serial.print("  |  rms  ");
      Serial.print(rightMotorSpeed);
      Serial.print("  |  lms  ");
      Serial.println(leftMotorSpeed);
      for (int i = 1; i < 4; i = i + 2) {
        digitalWrite(base_motor[i].dirPin, walk_direction);
        analogWrite(base_motor[i].pwmPin, leftMotorSpeed);
      }
      for (int i = 0; i < 4; i = i + 2) {
        digitalWrite(base_motor[i].dirPin, walk_direction);
        analogWrite(base_motor[i].pwmPin, rightMotorSpeed);
      }
    }
  }
}

/*--------------------------------------------------------| Bot Base. |--------------------------------------------------------*/
// stop the bot.
void stopBot() {
  walk_pwm = 0;
  walk_direction = HIGH;
}

// move the bot in the right direction.
void walkRight(int pwm) {
  Serial.print(pwm);
  walk_pwm = pwm;
  walk_direction = HIGH;
}

// move the bot in left direction.
void walkLeft(int pwm) {
  Serial.println(pwm);
  walk_pwm = pwm;
  walk_direction = LOW;
  for (int i = 0; i < 4; i++) {
    digitalWrite(base_motor[i].dirPin, LOW);
    analogWrite(base_motor[i].pwmPin, pwm);
  }
}

/*--------------------------------------------------------| PS2X |--------------------------------------------------------*/
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


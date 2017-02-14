#include <PS2X_lib.h>  //for v1.6
PS2X ps2x;

#define PS2_DAT        13
#define PS2_CMD        11
#define PS2_SEL        10
#define PS2_CLK        12
#define pressures   false
#define rumble      false

// data structure for a motor controlled by one of our H-bridges.
struct typeMotor {
  byte pwmPin;
  byte dirPin;
};

// all 4 motors of the bot's base.
typeMotor base_motor[4];

void setup() {
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
  if (ps2x.Button(PSB_L1)) {
    int LX = ps2x.Analog(PSS_LX);
    if (LX == 123) {
      Serial.print("Stopped at ");
      Serial.println(123 - LX);
      stopBot();
    }
    else if (LX > 123) {
      Serial.print("Right at ");
      walkRight((LX - 123) * (100.000 / 132.000));
    }
    else if (LX < 123) {
      Serial.print("Left at ");
      walkLeft((123 - LX) * (100.000 / 123.000));
    }
  }
  delay(50);
}
/*--------------------------------------------------------| Bot Base. |--------------------------------------------------------*/
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


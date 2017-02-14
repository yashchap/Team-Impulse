#include <PS2X_lib.h>  //for v1.6
PS2X ps2x;

#define PS2_DAT        13
#define PS2_CMD        12
#define PS2_SEL        11
#define PS2_CLK        10
#define pressures   false
#define rumble      false

const int act_out_one = 37;         // r3
const int act_out_two = 43;         // r4

void setup() {
  Serial.begin(57600);


  pinMode(act_out_one, OUTPUT);
  pinMode(act_out_two, OUTPUT);
  digitalWrite(act_out_one, LOW);
  digitalWrite(act_out_two, HIGH);



  delay(300);  //added delay to give wireless ps2 module some time to startup, before configuring it
  configurePS2X();
}

void loop() {
  ps2x.read_gamepad(); // read controller.

// actuator up
  if (ps2x.Button(PSB_PAD_UP)) {     //will be TRUE as long as button is pressed
    Serial.println("Up held.");
    moveActuator(HIGH, HIGH);
  }
// actuator doen  
  if (ps2x.Button(PSB_PAD_DOWN)) {
    Serial.println("DOWN held.");
    moveActuator(LOW, LOW);
  }
// actuator stop
  if (!ps2x.Button(PSB_PAD_UP) && !ps2x.Button(PSB_PAD_DOWN)) {
    moveActuator(HIGH, LOW);
  }

  delay(50);
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

/*--------------| Actuator |-------------------------------------------------------*/
//  pwm1      pwm2        actuator               (0 = low, 1 = high)
//   0         0           goes down
//   0         1            no move
//   1         0            no move
//   1         1           goes up
void moveActuator(int pwm1, int pwm2) {
  digitalWrite(act_out_one, pwm1);
  digitalWrite(act_out_two, pwm2);
}

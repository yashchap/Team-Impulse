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

int pneumatic_pin = 49;                 // R5


const int act_out_one = 37;             // R3
const int act_out_two = 43;             // R4

const int upper_pwm = 4;                // H3
int upSpeed = 50;                       // pwm for upper motor is written through this variable

void setup() {
  // begin lcd communications
  lcd.begin(16, 4);

  Serial.begin(57600);

  // set pin modes
  pinMode(upper_pwm, OUTPUT);
  pinMode(pneumatic_pin, OUTPUT);
  pinMode(act_out_one, OUTPUT);
  pinMode(act_out_two, OUTPUT);
  digitalWrite(act_out_one, LOW);
  digitalWrite(act_out_two, HIGH);

  
  // wait for ps2 to boot
  delay(300);
  configurePS2X();
}

void loop() {
  // read gamepad. call atleast once a second
  ps2x.read_gamepad();

  // update lcd
  updateLCD();

  if (ps2x.Button(PSB_START))                                               // start - not used
    Serial.println("Start is being held");
  if (ps2x.Button(PSB_SELECT))                                              // select - not used
    Serial.println("Select is being held");

  if (ps2x.Button(PSB_PAD_UP)) {                                            // up - move actuator up
    Serial.println("Up held.");
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
    Serial.println("DOWN held.");
    moveActuator(LOW, LOW);
  }
  if (!ps2x.Button(PSB_PAD_UP) && !ps2x.Button(PSB_PAD_DOWN)) {
    moveActuator(HIGH, LOW);
  }

  if (ps2x.NewButtonState()) {
    if (ps2x.Button(PSB_L3))                                                // l3 - not used
      Serial.println("L3 pressed");
    if (ps2x.Button(PSB_R3))                                                // r3 - not used
      Serial.println("R3 pressed");
    if (ps2x.Button(PSB_L2))                                                // l2 - not used
      Serial.println("L2 pressed");
    if (ps2x.Button(PSB_R2))                                                // r2 - not used
      Serial.println("R2 pressed");
    if (ps2x.Button(PSB_TRIANGLE))                                          // ^ - not used
      Serial.println("Triangle pressed");
  }

  if (ps2x.ButtonPressed(PSB_CIRCLE)) {                                     // o - reset upper pwm
    upSpeed = 0;
    setUpperPwm();
  }
  if (ps2x.NewButtonState(PSB_CROSS)) {                                     // x - pneumatic control
    //will be TRUE if button was JUST pressed OR released
    fireDisc();
  }
  if (ps2x.ButtonReleased(PSB_SQUARE))                                      // # - not used
    Serial.println("Square just released");

  if (ps2x.Button(PSB_L1) || ps2x.Button(PSB_R1)) {                         // triggers and analog - not set - base control 
    Serial.print("Stick Values:");
    Serial.print(ps2x.Analog(PSS_LY), DEC);
    Serial.print(",");
    Serial.print(ps2x.Analog(PSS_LX), DEC);
    Serial.print(",");
    Serial.print(ps2x.Analog(PSS_RY), DEC);
    Serial.print(",");
    Serial.println(ps2x.Analog(PSS_RX), DEC);
  }
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
void updateLCD() {
  lcd.setCursor(0,1);
  lcd.print("PWM  ");
  (upSpeed >= 100) ? NULL : lcd.print(" ");
  (upSpeed >= 10) ? NULL : lcd.print(" ");
  lcd.print(upSpeed);
}

/*-------| Linear Actuator |---------------------------------------------------------------*/
void moveActuator(int pwm1, int pwm2) {
  digitalWrite(act_out_one, pwm1);
  digitalWrite(act_out_two, pwm2);
}

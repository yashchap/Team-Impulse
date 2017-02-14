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

byte old_col = 0, old_row = 0;          // data persistance for lcd functions

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
  displayLCD("Motor PWM:", 0, 0);
  displayLCD(upSpeed, 14, 0, 3);
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

/*-------| Linear Actuator |---------------------------------------------------------------*/
void moveActuator(int pwm1, int pwm2) {
  digitalWrite(act_out_one, pwm1);
  digitalWrite(act_out_two, pwm2);
}

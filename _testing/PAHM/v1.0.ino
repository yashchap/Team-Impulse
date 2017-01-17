//___ROBOCON_17___
//_PNEUMATIC__ACT_
//__ACTUATOR_ACT__
//PWM_VALUE = 000_

/* MPU PIN Config
  Vcc - 3.3v
  SDA - 20
  SCL - 21
  AD0 - GND
 */

#include <LiquidCrystal.h>
#include <Wire.h>

//LCD_INITIALIZE_PIN
//ROBO(RS, E, D4, D5, D6, D7);

LiquidCrystal lcd(27, 26, 25, 24, 23, 22);
int pin_contrast = A1;
int pin_input = A0; //FOR VARYING SPEED
int pwm_output = 9; //PWM INPUT TO H-BRIDGE
int direction_pin = 8;  //FOR DIRECTION
int direction_val = LOW;  //keep low ;not working in other direction
int relay_pneumatic_out = 10;
int relay_actuator_out = 5;
int relay_act_pin_in = 7;
int relay_pneu_in = 40;
int read_button_pneu = 0;
int read_button_ACT = 0;
const int MPU_addr = 0x68; // I2C address

void setup() {

  //pneumatic_pin_configuration
  pinMode(relay_pneu_in,INPUT);
  pinMode(relay_pneumatic_out,OUTPUT); //connected to NO

  //acctuator_pin_configuration
  pinMode(relay_act_pin_in,INPUT);
  pinMode(relay_actuator_out,OUTPUT);//connected to NC
  digitalWrite(relay_actuator_out,LOW);

  //H-BRIDGE_pin_config
  pinMode(direction_pin,OUTPUT);
  pinMode(pwm_output,OUTPUT);
  pinMode(pin_input,INPUT);
  digitalWrite(direction_pin,direction_val);

  //Serial initialization
  Serial.begin(9600);
  Wire.begin();

  //LCD_INITIALIZE
  //lcd.clear();
  lcd.begin(16,4);

  //CONTRAST
  pinMode(pin_contrast,OUTPUT);
  analogWrite(pin_contrast,255);

  //MPU_Wakeup-call
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x6B);  // PWR_MGMT_1 register
  Wire.write(0);     // set to zero (wakes up the MPU)
  Wire.endTransmission(true);
}

void loop() {
  //lcd.setCursor(0,0);
  //lcd.print("___ROBOCON_17___");
  h_bridge();
  pneumatic();
  actuator();
  MPU();
}

void pneumatic()
{
  read_button_pneu = digitalRead(relay_pneu_in);
  if(read_button_pneu)
  {
    Serial.println("_PNEUMATIC__ACT_");
    lcd.setCursor(0,0);
    lcd.println("_PNEUMATIC__ACT_");
    digitalWrite(relay_pneumatic_out,HIGH);
  }
  else
  {
    digitalWrite(relay_pneumatic_out,LOW);
    lcd.setCursor(0,0);
    Serial.println("PNEUMATIC_DEACT_");
  }
}

void actuator()
{
  read_button_ACT = digitalRead(relay_act_pin_in);
  if(read_button_ACT)
  {
    digitalWrite(relay_actuator_out,LOW);
    //Serial.print("ACTUATOR_relay_released");
    lcd.setCursor(0,1);
    lcd.print("__ACTUATOR_ACT__");   
  }
  else
  {
    digitalWrite(relay_actuator_out,HIGH);
    lcd.setCursor(0,1);
    lcd.print("_ACTUATOR_DEACT_");  
    //Serial.print("_ACTUATOR_DEACT_");  
  }
}

void h_bridge()
{
  int value_analog = analogRead(pin_input);
  value_analog = map(value_analog, 0, 1023, 0, 255);
//  Serial.print("direction = ");
//  Serial.println(value_analog);
//  Serial.print("Value = ");
//  Serial.println(value_analog);
  analogWrite(pwm_output,value_analog);
   lcd.setCursor(11,2);
  ((value_analog/100 > 0)) ? : lcd.print(" ")  ;
  ((value_analog/10 > 0)) ? : lcd.print(" ")  ;
  lcd.setCursor(11,2);
  lcd.print(value_analog);
  delay(260);
}

void MPU() {
  int Hx = (int)getAccelValues(0x3B);
  int Hy = (int)getAccelValues(0x3D);
  ((Hx/100 > 0)) ? : lcd.print(" ")  ;
  ((Hx/10 > 0)) ? : lcd.print(" ")  ;
  lcd.setCursor(4,3);
  lcd.print(Hx);
  ((Hy/100 > 0)) ? : lcd.print(" ")  ;
  ((Hy/10 > 0)) ? : lcd.print(" ")  ;
  lcd.setCursor(11,3);
  lcd.print(Hy);
}

byte getAccelValues(byte address) {
  Wire.beginTransmission(0x68);
  Wire.write(address);
  Wire.endTransmission(false);
  Wire.requestFrom(0x68, 1, true);
  return Wire.read();
}

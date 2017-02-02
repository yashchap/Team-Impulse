//___ROBOCON_17___
//DELAY(3000)
//_PNEUMATIC__ACT_
//__ACTUATOR_ACT__
//PWM_VALUE = 000_
// X = 000 Y =000 


#include <LiquidCrystal.h>
#include <SD.h>
#include <Wire.h>

//LCD_INITIALIZE_PIN
//ROBO(RS, E, D4, D5, D6, D7);

// SD Card Declaration
File myFile;

LiquidCrystal lcd(27, 26, 25, 24, 23, 22);
int counter = 0;
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
int sdButton_pin = x;
const int chipSelect = 53;
int read_sdButton = 0;

int Hx;
int Hy;
void setup() {

  //---------------------------------------------------SD Card setup----------------------------------------------------

  
  Serial.print("Initializing SD card...");
  if (!SD.begin(chipSelect)) 
  {
    Serial.println("Initialization FAILED!!!!");
    return;
  }
  Serial.println("Initialization DONE.");
  pinMode(sdButton_pin,INPUT);


 //----------------------------------------------pneumatic_pin_configuration-----------------------------------------------
 pinMode(relay_pneu_in,INPUT);
 pinMode(relay_pneumatic_out,OUTPUT); //connected to NO

 
 //------------------------------------------acctuator_pin_configuration---------------------------------------------------------
 pinMode(relay_act_pin_in,INPUT);
 pinMode(relay_actuator_out,OUTPUT);//connected to NC
 digitalWrite(relay_actuator_out,LOW);

 
 //-----------------------------------------H-BRIDGE_pin_config-------------------------------------------------------------
 pinMode(direction_pin,OUTPUT);
 pinMode(pwm_output,OUTPUT);
 pinMode(pin_input,INPUT);
 digitalWrite(direction_pin,direction_val);

 
 //-----------------------------------------------Serial Initialization---------------------------------------------------------
 Serial.begin(9600);
 Wire.begin();

 
//-------------------------------------------------------LCD Initialization--------------------------------------------------------------
 lcd.clear();
 lcd.begin(16,4);


//------------------------------------------------------CONTRAST Setting----------------------------------------------------------
 pinMode(pin_contrast,OUTPUT);
 analogWrite(pin_contrast,200);


 lcd.setCursor(0,0);
 lcd.print("___ROBOCON_17___");
 delay(3000);
}

//----------------------------------------------------------------main loop()-----------------------------------------------------

void loop() {
 
 h_bridge();
 pneumatic();
 actuator();
 MPU();
 sd();
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
    Serial.println("PNEUMATIC_DEACT_");
    lcd.setCursor(0,0);
    lcd.println("PNEUMATIC_DEACT_");
  }
}

void sd()
{
  read_sdButton = digitalRead(sdButton_pin);
  if(read_sdButton)
  {
    myFile = SD.open("fileName.txt", FILE_WRITE | 0_APPEND);

    if(myFile) 
    {
      myFile.print(Counter);
      myFile.print(". \tPWM: ");
      myFile.print(pwm_output);
      myFile.print("\t\t Hx: ");
      myFile.print(Hx);
      myFile.print("\t\t Hy: ");
      myFile.print(Hy);
      myFile.println("             -----------------------------------              ");
      counter++;
//---------------------------------Closing File-------------------------------
      myFile.close();
    }
    else
    {
      Serial.println("oops!Aliens took your File...can't open it up.");
    }
  }
}
void actuator()
{
 read_button_ACT = digitalRead(relay_act_pin_in);
  if(read_button_ACT)
  {
    digitalWrite(relay_actuator_out,LOW);
    Serial.println("ACTUATOR_relay_released");
    lcd.setCursor(0,1);
    lcd.print("__ACTUATOR_ACT__");   
  }
  else
  {
    digitalWrite(relay_actuator_out,HIGH);
    lcd.setCursor(0,1);
    lcd.print("_ACTUATOR_DEACT_");  
    Serial.println("_ACTUATOR_DEACT_");  
  }
}

void h_bridge()
{
 int value_analog = analogRead(pin_input);
 value_analog = map(value_analog, 0, 1023, 0, 255);
 Serial.print("direction = ");
 Serial.println(value_analog);
 Serial.print("Value = ");
 Serial.println(value_analog);
 analogWrite(pwm_output,value_analog);
 lcd.setCursor(0,2);
 lcd.print("PWM_VALUE = "); 
 if(value_analog < 100)
 {lcd.setCursor(11,2);
  lcd.print("0");
  }
  
 else if(value_analog < 10)
 {lcd.setCursor(11,2);
  lcd.print("00");
  }
  
 else {lcd.setCursor(11,2);}
 lcd.print(value_analog);  
//  ((value_analog/100 > 0)) ? : lcd.print(" ")  ;
//  ((value_analog/10 > 0)) ? : lcd.print(" ")  ;
//  lcd.setCursor(0,2);
//  lcd.print("PWM_OUTPUT = ");
//  lcd.print(value_analog);
 delay(260);
}
.

void MPU() {
  Hx = (int) getAccelValues(0x3B);
  Hy = (int) getAccelValues(0x3D);
  ((Hx/100 > 0)) ? : lcd.print(" ")  ;
  ((Hx/10 > 0)) ? : lcd.print(" ")  ;
  lcd.setCursor(0,3);
  lcd.print("X =");
  lcd.print(Hx);
  ((Hy/100 > 0)) ? : lcd.print(" ")  ;
  ((Hy/10 > 0)) ? : lcd.print(" ")  ;
  lcd.setCursor(7,3);
  lcd.print("Y =");
  lcd.print(Hy);
  Serial.println("X: ");
  Serial.println(Hx);
  Serial.println("Y: ");
  Serial.println(Hy);
}

byte getAccelValues(byte address) {
  Wire.beginTransmission(0x68);
  Wire.write(address);
  Wire.endTransmission(false);
  Wire.requestFrom(0x68, 1, true);
  return Wire.read();
}



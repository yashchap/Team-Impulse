#include<stdio.h>
#include "PAHM.h"
#include "../Wire/Wire.h"

/*default variables */
int PAHM::MPU_wake_up_address = 0x68;
int PAHM::disk_Hbridge_throw_direction_val = 0;

/* default defination of variables*/
int PAHM::pwm_input_pin = 0;
int PAHM::pwm_output_pin = 0;
int PAHM::direction_pin = 0;
int PAHM::relay_pneumatic_out_pin = 0;
int PAHM::relay_actuator_out_pin = 0;
int PAHM::relay_actuator_in_pin = 0;
int PAHM::relay_pneumatic_in_pin = 0;

PAHM::PAHM(int pwm_input_pin,int pwm_output_pin,int direction_pin,int relay_pneumatic_out_pin,int relay_actuator_out_pin,int relay_actuator_in_pin,int relay_pneumatic_in_pin){
  PAHM::pwm_input_pin = pwm_input_pin;
  PAHM::pwm_output_pin = pwm_output_pin;
  PAHM::direction_pin = direction_pin;
  PAHM::relay_pneumatic_out_pin = relay_pneumatic_out_pin;
  PAHM::relay_pneumatic_in_pin = relay_pneumatic_in_pin;
  PAHM::relay_actuator_in_pin = relay_actuator_in_pin;
  PAHM::relay_actuator_out_pin = relay_actuator_out_pin;
}

int PAHM::pneumatic(){
  int read_button_pneu = digitalRead(relay_pneumatic_in_pin);
  if(read_button_pneu){
    digitalWrite(relay_pneumatic_out_pin,HIGH);
    return 1;
  }
  else{
    digitalWrite(relay_pneumatic_out_pin,LOW);
    return 0;
  }
}

void PAHM::Hbridge(int pwm){
  int value_analog = analogRead(pwm_input_pin);
  analogWrite(pwm_output_pin,pwm);
}

int PAHM::actuator(int new_state){
  int read_button_ACT = digitalRead(relay_actuator_in_pin);
   if(read_button_ACT){
     digitalWrite(relay_actuator_out_pin,LOW);
     return 0;
   }
   else{
     digitalWrite(relay_actuator_out_pin,HIGH);
     return 1;
   }
}

byte PAHM::getAccelValue(byte address){
  Wire.beginTransmission(0x68);
  Wire.write(address);
  Wire.endTransmission(false);
  Wire.requestFrom(0x68, 1, true);
  return Wire.read();
}

void PAHM::wakeMPU(){
  Wire.beginTransmission(MPU_wake_up_address);
  Wire.write(0x6B);  // PWR_MGMT_1 register
  Wire.write(0);     // set to zero (wakes up the MPU)
  Wire.endTransmission(true);
}

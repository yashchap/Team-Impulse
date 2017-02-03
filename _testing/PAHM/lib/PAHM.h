#ifndef PAHM_h
#define PAHM_h

#if ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
  #include "pins_arduino.h"
  #include "WConstants.h"
  #include <Wire.h>
#endif

class PAHM{
  public:
    PAHM();
    /*Pin Variales*/
    static int pwm_input_pin; /*pwm input for varying speed*/
    static int pwm_output_pin; /*pwm output for H-Bridge*/
    static int direction_pin; /* For direction */
    static int relay_pneumatic_out_pin; /*pneumatic out pin*/
    static int relay_actuator_out_pin ;  /*relay actuator out pin */
    static int relay_actuator_in_pin ;   /*relay actuator in pin */
    static int relay_pneumatic_in_pin; /*pneumatic in pin*/

    /*Functions */
    int pneumatic();
    void Hbridge(int pwm);
    int actuator(int new_state);
    void wakeMPU();
    byte getAccelValue(byte register_address);

  private:
    /*Default Value variables*/
    static int disk_Hbridge_throw_direction_val;  /*Low: another direction des not work due to the blockage of mechanism */
    static int MPU_wake_up_address; //I2C wake_up_address
};

#endif

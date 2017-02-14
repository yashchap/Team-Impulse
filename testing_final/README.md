##Testing Order##

  1. __actuator.ino: 
     * may have to change the logic table of HIGH/LOW combinations
  2. __base_control_no_correction.ino
     * will have to change pin definitions
     * will have to change how right side and left side motors are selected
  3. __base_control_with_correction.ino
     * all the changes from above
     * will have to add logic for lsa selection
     * will have to add logic for jPulse
     * fine tuning of PSD constants

//#define address      0x01

char address = 0x01;
const byte rx = 0;    // Defining pin 0 as Rx
const byte tx = 1;    // Defining pin 1 as Tx
const byte serialEn = 2;
const byte jPulse = 3;

void setup() {
  Serial.begin(57600);
  Serial.flush();   // Clear serial buffer
}

void loop() {

  char command, data;

  // Clear internal junction count of LSA08
  command = 'X';
  data = 0x00;
  sendCommand(command,data);

  // Setting LCD contrast to 80
  command = 'S';
  data = 0x50;
  sendCommand(command,data);

  // Setting junction width to 6
  command = 'J';
  data = 0x08;
  sendCommand(command,data);

  // Setting threshold value to 5
  command = 'T';
  data = 0x06;
  sendCommand(command,data);

  // Setting line mode to Dark-On
  command = 'L';
  data = 0x00;
  sendCommand(command,data);

  // Start calibration
  command = 'C';
  data = 0x00;
  sendCommand(command,data);

  while(1);   // Stay here to prevent infinite loop
}


/*-----------------------------------Send Command to LSA---------------------------------------------------*/

void sendCommand(char command, char data) {

  char checksum = address + command + data;

  Serial.write(address);
  Serial.write(command);
  Serial.write(data);
  Serial.write(checksum);

}







#include <Servo.h>
#define reset 0                  // for setting up in absolute degrees
#define halfset 90               // for setting up in absolute degrees
#define set 180                  // for setting up in absolute degrees
#define smooth true              // for movement type 
#define staggered false          // for movement type

Servo myservo;                   // object that handles the servo motor
String inputString = "";         // a string to hold incoming data
boolean stringEnd = false;       // whether the string is complete
boolean inputEnd = false;        // whether the entire input is complete
boolean movementType = smooth;   // boolean variable that holds current movement type
byte servoPin = 9;               // pin to which servo data line is attached
byte currentPos = 0;             // holds current position of the servo in degrees
byte lastPos = 0;                // holds the last position of the servo in degrees
byte stepSize = 10;              // holds step size for staggered mode
byte state = 0;                  // holds the state of the DFA for switch-case statements
int stepTime = 250;              // holds step time for staggered mode

void setup() {
  // initialize serial:
  Serial.begin(9600);
  // reserve 75 bytes for the inputString
  inputString.reserve(75);
  // attach servo data line to pin 9
  myservo.attach(servoPin);
  // reset servo to 0 degrees
  myservo.write(currentPos);
}

// calculates the degree to which 
// the servo needs to rotate
void toDegree(int number, int argument) {
  currentPos = currentPos*argument + number;
  currentPos = abs(currentPos);
  if (currentPos>180) currentPos = 180;
  if (currentPos<0) currentPos = 0;
  moveServo();
}

// moves the servo to that position
void moveServo() {
  // for smooth movementType
  if (movementType) myservo.write(currentPos);
  // for staggered movementType
  else {
    while (currentPos != lastPos) {
      int n = currentPos - lastPos;
      n = abs(n);
      if (n<stepSize) {
        lastPos = currentPos;
        myservo.write(lastPos);
        delay(stepTime);
      }
      else {
        lastPos = lastPos + (((currentPos - lastPos)>0)-((currentPos - lastPos)<0))*stepSize;
        myservo.write(lastPos);
        delay(stepTime);
      }
    }
  }
  lastPos = currentPos;
}

// returns true if all chars in a 
// given string are digits.
boolean isInt(String strIn) {
  for (int i=0; i<strIn.length(); i++) {
    if (strIn.charAt(i)<'0' || strIn.charAt(i)>'9') return false;
  }
  return true;
}

// clear inputString
void clearString() {
  inputString = "";
  stringEnd = false;  
}

void loop() {
  // work on the string when stringEnd is true
  if (stringEnd || inputEnd) {
    switch (state) {
      // determine actor
      case 0:
        if (inputString == "Servo") {
          Serial.println("Servo selected."); 
          state = 1;
        }
        clearString();
        break;
      // actor: servo; case to determine action
      case 1:
        // move relatively anticlockwise
        if (inputString == "ac") {
          Serial.println("Moving in anti-clockwise");
          state = 2;
          clearString();
        }
        // move relatively clockwise
        else if (inputString == "c") {
          Serial.println("Moving in clock-wise direction");
          state = 3;
          clearString();
        }
        // move to absolute value
        else if (isInt(inputString) || inputString=="set" || inputString=="halfset" || inputString=="reset") {
          Serial.println("Using Absolute Value");
          state = 4;
        }
        // set movementMode
        else if (inputString == "mode") {
          Serial.println("Setting mode.");
          state = 5;
          clearString();
        }
        break;
      // actor: servo; case to determine movement
      // (anticlockwise) through relative degree
      case 2:
        if (isInt(inputString)) {
          Serial.println("Using relative input.");
          toDegree(inputString.toInt(), 1);
        }
        else; state = 255;
        clearString();
        break;
      // actor: servo; case to determine movement
      // (clockwise) through relative degree
      case 3:
        if (isInt(inputString)) {
          Serial.println("Using relative input.");
          toDegree(inputString.toInt(), -1);
        }
        else; state = 255;
        clearString();
        break;
      // actor: servo; case to determine movement
      // (absolute) through absolute degree
      case 4:
        if (isInt(inputString)) toDegree(inputString.toInt(), 0);
        else if (inputString=="set") toDegree(set, 0);
        else if (inputString=="halfset") toDegree(halfset, 0);
        else if (inputString=="reset") toDegree(reset, 0);
        else; state = 255;
        clearString();
        break;
      // actor: servo; case to set modes
      case 5:
        if (inputString == "smooth") {
          Serial.println("Smooth mode selected.");
          movementType = smooth;
          clearString();
          state = 255;
        }
        else if (inputString == "staggered") {
          Serial.println("Staggered Mode selected.");
          movementType = staggered;
          clearString();
          state = 6;
        }
        else;
        break;
      // actor: servo; set stepSize if
      // movement set to staggered
      case 6:
        if (isInt(inputString)) {
          Serial.println("Step size set.");
          int n = inputString.toInt();
          if (n<180 && n>0) stepSize = n;
          else Serial.println("stepSize not in range of movement."); 
        }
        clearString();
        state = 7;
        break;
      // actor: servo; set stepTime if
      // movement set to staggered
      case 7:
        if (isInt(inputString)) {
          Serial.println("Step time set.");
          int n = inputString.toInt();
          if (n>=250 && n<=2000) stepTime = n;
          else Serial.println("stepTime not in acceptable range (250 - 2000)."); 
        }
        clearString();
        state = 255;
        break;
      // state 255 is the reset input state
      case 255:
        Serial.println("Input Reset.");
        inputEnd = false;
        clearString();
        Serial.println();
        state = 0;
        break;
      // when a state arrives for which no case
      // has been determined.
      default:
        Serial.println("could not determine action");
        clearString();
        break;
    }
  }
}

void serialEvent() {
  char inChar;  
  while (Serial.available()) {
    // when the main loop is done
    // processing the current string
    if (!stringEnd) {
      inChar = (char)Serial.read();
    }
    // if the incoming character is a ' ', set a flag
    // so the main loop can know a word is complete:
    if (inChar == ' ' && inputString != "") {
      stringEnd = true;
    }
    // if the incoming character is a ';', set a flag
    // so the main loop can know input is complete:
    else if (inChar == ';') {
      inputEnd = true;
      stringEnd = true;
    }
    // else, append this byte to the string:
    else {
      inputString += inChar;
    }
  }
}

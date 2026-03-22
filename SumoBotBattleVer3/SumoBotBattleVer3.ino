#include <Wire.h>
#include <Zumo32U4.h>
#include <PololuOLED.h>
#include <avr/pgmspace.h>

Zumo32U4OLED display;
Zumo32U4Motors motors;
Zumo32U4Buzzer buzzer;

Zumo32U4ProximitySensors proxSensors;
Zumo32U4LineSensors lineSensors;

Zumo32U4ButtonA buttonA;
Zumo32U4ButtonB buttonB;
Zumo32U4ButtonC buttonC;

enum ButtonChoice { // Create datatype for the button label
  A,
  B,
  C
}; 

unsigned int lineSensorValues[3];

const uint16_t lineSensorThreshold = 400;
const uint16_t reverseSpeed = 450;
const uint16_t turnSpeed = 300;
const uint16_t forwardSpeed = 450;
const uint16_t veerSpeedLow = 225;
const uint16_t veerSpeedHigh = 450;
const uint16_t rammingSpeed = 450;
const uint16_t reverseTime = 250;
const uint16_t scanTimeMin = 200;
const uint16_t scanTimeMax = 50000;
const uint16_t waitTime = 1000;
const uint16_t stalemateTime = 4000;

uint8_t leftValue = proxSensors.countsFrontWithLeftLeds();
uint8_t rightValue = proxSensors.countsFrontWithRightLeds();

// This enum lists the top-level states that the robot can be in.
enum State {
  StatePausing,
  StateWaiting,
  StateScanning,
  StateDriving,
  StateBacking,
};

State state = StateDriving;
String currentState = "";

enum Direction {
  DirectionLeft,
  DirectionRight,
};


Direction scanDir = DirectionRight;

uint16_t stateStartTime;
uint16_t displayTime;
bool justChangedState;

// This gets set whenever we clear the display.
bool displayCleared;


void moveForwardForOneSecond() {
  unsigned long startTime = millis();

  while (millis() - startTime < 400) {
    motors.setSpeeds(forwardSpeed, forwardSpeed);   
  }
}


void setup() {
  bool buttonPressed = false;       // Boolean whether or not a button has been pressed.
  ButtonChoice buttonSelected;      // variable to store which button was pressed.
  
  motors.flipLeftMotor(true);
  motors.flipRightMotor(true);

  lineSensors.initThreeSensors();
  proxSensors.initThreeSensors();

  changeState(StatePausing);

  // Wait for the user to press A before driving the motors.
  buzzer.playNote(NOTE_B(5), 100, 15);
  delay(100);
  buzzer.playNote(NOTE_E(6), 200, 15);
  display.setLayout11x4();   
  display.gotoXY(0, 1);
  display.print(F("   Select"));
  display.gotoXY(0, 2);
  display.print(F("   A B C"));
  while (buttonPressed == false) {
    if (buttonA.isPressed()) {
      buttonSelected = A;
      //Serial.print("Button A Selected");
      buzzer.playFrequency(300, 200, 15);
      buttonPressed = true;
    }
    if (buttonB.isPressed()) {
      buttonSelected = B;
      //Serial.print("Button B Selected");
      buzzer.playFrequency(440, 200, 15);
      buttonPressed = true;
    }
    if (buttonC.isPressed()) {
      buttonSelected = C;
      //Serial.print("Button C Selected");
      buzzer.playFrequency(600, 200, 15);
      buttonPressed = true;
    }
  }
  delay(500);   
  display.clear();
  display.gotoXY(0, 1);
  display.print(F("   Press"));
  display.gotoXY(0, 2);
  display.print(F(" A To Start"));
  buttonA.waitForButton(); 

  if (buttonSelected == A) {
    display.clear();
    // Play audible countdown.
    for (int i = 0; i < 3; i++)
    {
      delay(1000);
      display.clear();
      display.gotoXY(0, 1);
      display.print("     ");
      display.print(3-i);
      buzzer.playNote(NOTE_G(3), 200, 15);
    }
    delay(1000);
    display.clear();
    display.gotoXY(0, 1);
    display.print("   GOOO!");
    buzzer.playNote(NOTE_G(4), 500, 15);
    delay(1000);
    display.clear();
    display.setLayout21x8();   
    changeState(StateScanning);
    currentState = "Scanning";

    // Run the function for exactly 1 second
    moveForwardForOneSecond();
  } 
  else if (buttonSelected == B) {
    display.clear();
    // Play audible countdown.
    for (int i = 0; i < 3; i++)
    {
      delay(1000);
      display.clear();
      display.gotoXY(0, 1);
      display.print("     ");
      display.print(3-i);
      buzzer.playNote(NOTE_G(3), 200, 15);
    }
    delay(1000);
    display.clear();
    display.gotoXY(0, 1);
    display.print("   GOOO!");
    buzzer.playNote(NOTE_G(4), 500, 15);
    delay(1000);
    display.clear();
    display.setLayout21x8();   
    changeState(StateScanning);
    currentState = "Scanning"; 
    changeState(StateBacking);
    currentState = "reversing";
  }
  else {
    delay(5000);
    display.setLayout21x8();
    changeState(StateBacking);  
    currentState = "Reversing"; 
  }
}

void loop()
{
  if (state == StateScanning) {
    // In this state the robot rotates in place and tries to find
    // its opponent.

    if (justChangedState) {
      justChangedState = false;
      currentState = "Scanning";
    }

    if (scanDir == DirectionRight) {
      motors.setSpeeds(turnSpeed, -turnSpeed);
    }
    else {
      motors.setSpeeds(-turnSpeed, turnSpeed);
    }

    uint16_t time = timeInThisState();

    if (time > scanTimeMax) {
      // We have not seen anything for a while, so start driving.
      changeState(StateDriving);
    }
    else if (time > scanTimeMin) {
      // Read the proximity sensors.  If we detect anything with
      // the front sensor, then start Driving.
      proxSensors.read();
      if (proxSensors.countsFrontWithLeftLeds() >= 2 || proxSensors.countsFrontWithRightLeds() >= 2) {
        changeState(StateDriving);
        currentState = "Driving";
      }
    }
  }
  if (state == StateBacking) {
    // In this state, the robot drives in reverse.

    if (justChangedState) {
      justChangedState = false;
      currentState = "Reversing";
    }

    motors.setSpeeds(-reverseSpeed, -reverseSpeed);

    // After backing up for a specific amount of time, start
    // scanning.
    if (timeInThisState() >= reverseTime) {
      changeState(StateScanning);
      currentState = "Scanning";
    }
  }
  else if (state == StateDriving) {
    // In this state we drive forward while also looking for the
    // opponent using the proximity sensors and checking for the
    // white border.

    if (justChangedState) {
      justChangedState = false;
      currentState = "Driving";
    }

    // Check for borders.
    lineSensors.read(lineSensorValues);
    if (lineSensorValues[0] < lineSensorThreshold) {
      scanDir = DirectionRight;
      changeState(StateBacking);
      currentState = "Reversing";

    }
    if (lineSensorValues[2] < lineSensorThreshold) {
      scanDir = DirectionLeft;
      changeState(StateBacking);
      currentState = "Driving Back";
    }

    // Read the proximity sensors to see if know where the
    // opponent is.
    proxSensors.read();
    leftValue = proxSensors.countsFrontWithLeftLeds();
    rightValue = proxSensors.countsFrontWithRightLeds();
    Serial.print(leftValue);
    Serial.print(rightValue);
    uint8_t sum = proxSensors.countsFrontWithRightLeds() + proxSensors.countsFrontWithLeftLeds();
    int8_t diff = proxSensors.countsFrontWithRightLeds() - proxSensors.countsFrontWithLeftLeds();

    if (sum >= 2 || timeInThisState() > stalemateTime) {
      // The front sensor is getting a strong signal, or we have
      // been driving forward for a while now without seeing the
      // border.  Either way, there is probably a robot in front
      // of us and we should switch to ramming speed to try to
      // push the robot out of the ring.
      motors.setSpeeds(rammingSpeed, rammingSpeed);

      // Turn on the red LED when ramming.
      ledRed(1);
    }
    else if (sum == 0) {
      // We don't see anything with the front sensor, so just
      // keep driving forward.  Also monitor the side sensors; if
      // they see an object then we want to go to the scanning
      // state and turn torwards that object.

      motors.setSpeeds(forwardSpeed, forwardSpeed);

      if (proxSensors.countsLeftWithLeftLeds() >= 1) {
        // Detected something to the left.
        scanDir = DirectionLeft;
        changeState(StateScanning);
        currentState = "Scanning";
      }

      if (proxSensors.countsRightWithRightLeds() >= 1) {
        // Detected something to the right.
        scanDir = DirectionRight;
        changeState(StateScanning);
        currentState = "Scanning";
      }

      ledRed(0);
    }
    else {
      // We see something with the front sensor but it is not a
      // strong reading.

      if (diff >= 1) {
        // The right-side reading is stronger, so veer to the right.
        motors.setSpeeds(veerSpeedHigh, veerSpeedLow);
      }
      else if (diff <= -1) {
        // The left-side reading is stronger, so veer to the left.
        motors.setSpeeds(veerSpeedLow, veerSpeedHigh);
      }
      else {
        // Both readings are equal, so just drive forward.
        motors.setSpeeds(forwardSpeed, forwardSpeed);
      }
      ledRed(0);
    }
  }

 /////////////////////////////////////////////////////////////OLED DISPLAY DATA/////////////////////////////////////////////////////////////

  //REMEMBER TO COMMENT OUT DURING COMPETITION!!!//

  
  //Display Front Sensor Data//
  display.gotoXY(0, 1);
  display.print("LS:");
  display.print(leftValue);
  display.print("  ");
  display.print("RS:");
  display.print(rightValue);
  display.gotoXY(0, 2);
  display.print("State: ");
  display.print(currentState);
  display.gotoXY(0, 3);
  display.print(' '); 
  display.print(' ');

  //Display Line Sensor Data//
  lineSensors.read(lineSensorValues);
  for(int i=5; i<=7; i++) {
    display.gotoXY(0, i);
    display.print(i-5);
    display.print(": ");
    display.print(lineSensorValues[i-5]);
  }
  

}

// Gets the amount of time we have been in this state, in
// milliseconds.  After 65535 milliseconds (65 seconds), this
// overflows to 0.
uint16_t timeInThisState()
{
  return (uint16_t)(millis() - stateStartTime);
}

// Changes to a new state.  It also clears the display and turns off
// the LEDs so that the things the previous state were doing do
// not affect the feedback the user sees in the new state.
void changeState(uint8_t newState) {
  state = (State)newState;
  justChangedState = true;
  stateStartTime = millis();
  ledRed(0);
  ledYellow(0);
  ledGreen(0);
  display.clear();
  displayCleared = true;
}

// Returns true if the display has been cleared or the contents
// on it have not been updated in a while.  The time limit used
// to decide if the contents are staled is specified in
// milliseconds by the staleTime parameter.
bool displayIsStale(uint16_t staleTime) {
  return displayCleared || (millis() - displayTime) > staleTime;
}

// Any part of the code that uses displayIsStale to decide when
// to update the display should call this function when it updates the
// display.
void displayUpdated() {
  displayTime = millis();
  displayCleared = false;
}

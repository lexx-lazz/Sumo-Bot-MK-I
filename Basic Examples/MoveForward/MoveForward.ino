/* This example uses the front proximity sensor on the Zumo 32U4
Front Sensor Array to locate an opponent robot or any other
reflective object. Using the motors to turn, it scans its
surroundings. If it senses an object, it turns on its yellow LED
and attempts to face towards that object. */

#include <Wire.h>
#include <Zumo32U4.h>
#include <PololuOLED.h>

/////////////////////////////////////////////////////////////ZUMO VARIABLES/////////////////////////////////////////////////////////////

Zumo32U4OLED display;

Zumo32U4Motors motors;
Zumo32U4ProximitySensors proxSensors;
Zumo32U4LineSensors lineSensors;
Zumo32U4ButtonA buttonA;
Zumo32U4Buzzer buzzer;

#define NUM_SENSORS 3
unsigned int lineSensorValues[NUM_SENSORS];

/////////////////////////////////////////////////////////////CONSTANT SPEEDS FOR FRONT IR SENSORS/////////////////////////////////////////////////////////////

// The maximum speed to drive the motors while moving forward.
const uint16_t moveSpeedMax = 1000;

/////////////////////////////////////////////////////////////BOOLEAN START CONDITIONS/////////////////////////////////////////////////////////////

// True if the robot is moving straight forward.
bool movingForward = false;

// If the robot is moving, this is the speed it will use.
uint16_t moveSpeed = moveSpeedMax;

/////////////////////////////////////////////////////////////SETUP/////////////////////////////////////////////////////////////

void setup() {
    
  // Wait for the user to press A before driving the motors.
  display.setLayout11x4();   
  display.gotoXY(0, 0);
  display.print(F("Press A or "));
  display.gotoXY(0, 1);
  display.print(F("give me 5"));
  display.gotoXY(0, 2);
  display.print(F("BIG BOOMS!"));
  buttonA.waitForButton();
  display.clear();
  display.print(F("  BOOM!"));

  // Play audible countdown.
  for (int i = 0; i < 3; i++)
  {
    delay(1000);
    display.gotoXY(0, i+1);
    display.print(F("  BOOM!"));
    buzzer.playNote(NOTE_G(3), 200, 15);
  }
  delay(1000);
  display.clear();
  display.gotoXY(0, 1);
  display.print(F("  BOOM!"));
  buzzer.playNote(NOTE_G(4), 500, 15);
  delay(1000);
  display.clear();
}

/////////////////////////////////////////////////////////////MOVEMENT FUNCTIONS/////////////////////////////////////////////////////////////

void moveForward() {
  motors.setSpeeds(-moveSpeed, -moveSpeed);
  movingForward = true;
}

/////////////////////////////////////////////////////////////MAIN LOOP////////////////////////////////////////////////////////////

void loop() {
 
  moveForward();
}
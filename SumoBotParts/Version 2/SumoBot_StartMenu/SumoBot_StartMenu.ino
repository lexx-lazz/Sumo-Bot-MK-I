/* This example uses the front proximity sensor on the Zumo 32U4
Front Sensor Array to locate an opponent robot or any other
reflective object. Using the motors to turn, it scans its
surroundings. If it senses an object, it turns on its yellow LED
and attempts to face towards that object. */

#include <Wire.h>
#include <Zumo32U4.h>
#include <PololuOLED.h>

/////////////////////////////////////////////////////////////CONSTANT SPEEDS FOR LINE SENSORS/////////////////////////////////////////////////////////////

// This might need to be tuned for different lighting conditions,
// surfaces, etc.
#define QTR_THRESHOLD     400  // microseconds                     //Threshhold for line sensors  //set to 400

// These might need to be tuned for different motor types.
#define REVERSE_SPEED     400  // 0 is stopped, 400 is full speed
#define TURN_SPEED        450
#define FORWARD_SPEED     450
#define REVERSE_DURATION  300  // ms
#define TURN_DURATION     575  // ms

/////////////////////////////////////////////////////////////ZUMO VARIABLES/////////////////////////////////////////////////////////////

Zumo32U4OLED display;
Zumo32U4Motors motors;
Zumo32U4Buzzer buzzer;

Zumo32U4ProximitySensors proxSensors;
Zumo32U4LineSensors lineSensors;

Zumo32U4ButtonA buttonA;
Zumo32U4ButtonB buttonB;
Zumo32U4ButtonC buttonC;


enum ButtonChoice       // Create datatype for the button label.
{
  A,
  B,
  C
}; 

#define NUM_SENSORS 3
unsigned int lineSensorValues[NUM_SENSORS];

/////////////////////////////////////////////////////////////CONSTANT SPEEDS FOR FRONT IR SENSORS/////////////////////////////////////////////////////////////

// A sensors reading must be greater than or equal to this
// threshold in order for the program to consider that sensor as

// seeing an object in front with IR sensors
const uint8_t frontSensorThreshold = 4;

// The maximum speed to drive the motors while turning.  400 is
// full speed.
const uint16_t turnSpeedMax = 450;

// The minimum speed to drive the motors while turning.  400 is
// full speed.
const uint16_t turnSpeedMin = 50;

// The maximum speed to drive the motors while moving forward.
const uint16_t moveSpeedMax = 1000;

// The amount to decrease the motor speed by during each cycle
// when an object is seen.
const uint16_t deceleration = 10;

// The amount to increase the speed by during each cycle when an
// object is not seen.
const uint16_t acceleration = 50;

#define LEFT 0
#define RIGHT 1

/////////////////////////////////////////////////////////////BOOLEAN START CONDITIONS/////////////////////////////////////////////////////////////

// Stores the last indication from the sensors about what
// direction to turn to face the object.  When no object is seen,
// this variable helps us make a good guess about which direction
// to turn.
bool senseDir = RIGHT;

// True if the robot is turning left (counter-clockwise).
bool turningLeft = false;

// True if the robot is turning right (clockwise).
bool turningRight = false;

// True if the robot is moving straight forward.
bool movingForward = false;

// True if the robot is moving straight backward.
bool movingBackward = false;

// If the robot is turning, this is the speed it will use.
uint16_t turnSpeed = turnSpeedMax;

// If the robot is moving, this is the speed it will use.
uint16_t moveSpeed = moveSpeedMax;

// The time, in milliseconds, when an object was last seen.u
uint16_t lastTimeObjectSeen = 0;

bool buttonPressed = false;       // Boolean whether or not a button has been pressed.

ButtonChoice buttonSelected;      // variable to store which button was pressed.

/////////////////////////////////////////////////////////////MUSIC STORAGE/////////////////////////////////////////////////////////////

const char fugue[] PROGMEM =
  "! O5 L16 agafaea dac+adaea fa<aa<bac#a dac#adaea f"
  "O6 dcd<b-d<ad<g d<f+d<gd<ad<b- d<dd<ed<f+d<g d<f+d<gd<ad"
  "L8 MS <b-d<b-d MLe-<ge-<g MSc<ac<a ML d<fd<f O5 MS b-gb-g"
  "ML >c#e>c#e MS afaf ML gc#gc# MS fdfd ML e<b-e<b-"
  "O6 L16ragafaea dac#adaea fa<aa<bac#a dac#adaea faeadaca"
  "<b-acadg<b-g egdgcg<b-g <ag<b-gcf<af dfcf<b-f<af"
  "<gf<af<b-e<ge c#e<b-e<ae<ge <fe<ge<ad<fd"
  "O5 e>ee>ef>df>d b->c#b->c#a>df>d e>ee>ef>df>d"
  "e>d>c#>db>d>c#b >c#agaegfe f O6 dc#dfdc#<b c#4";

/////////////////////////////////////////////////////////////SETUP/////////////////////////////////////////////////////////////

void setup() {
  bool buttonPressed = false;       // Boolean whether or not a button has been pressed.
  ButtonChoice buttonSelected;      // variable to store which button was pressed.

  proxSensors.initFrontSensor();
  lineSensors.initThreeSensors();

  // Wait for the user to press A before driving the motors.
  display.setLayout11x4();   
  display.gotoXY(0, 1);
  display.print(F("   Select"));
  display.gotoXY(0, 2);
  display.print(F("   A B C"));
  while (buttonPressed == false) {
    if (buttonA.isPressed()) {
      buttonSelected = A;
      Serial.print("Button A Selected");
      buzzer.playFrequency(300, 200, 15);
      buttonPressed = true;
    }
    if (buttonB.isPressed()) {
      buttonSelected = B;
      Serial.print("Button B Selected");
      buzzer.playFrequency(440, 200, 15);
      buttonPressed = true;
    }
    if (buttonC.isPressed()) {
      buttonSelected = C;
      Serial.print("Button C Selected");
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
      display.gotoXY(0, i+1);
      buzzer.playNote(NOTE_G(3), 200, 15);
    }
    delay(1000);
    display.clear();
    display.gotoXY(0, 1);
    buzzer.playNote(NOTE_G(4), 500, 15);
    delay(1000);
    display.clear();

    // Run the function for exactly 1 second
    moveForwardForOneSecond();
  } 
  else if (buttonSelected == B) {
    display.clear();
    display.gotoXY(0, 1);
    display.print(F("Now Playing"));
    display.gotoXY(0, 2);
    display.print(F("...Fugue"));
    // Start playing a tone with frequency 440 Hz at maximum
    // volume (15) for 200 milliseconds.
    buzzer.playFrequency(440, 200, 15);

    // Delay to give the tone time to finish.
    delay(1000);

    // Start playing note A in octave 4 at maximum volume
    // volume (15) for 2000 milliseconds.
    buzzer.playNote(NOTE_A(4), 2000, 15);

    // Wait for 200 ms and stop playing note.
    delay(200);
    buzzer.stopPlaying();

    delay(1000);

    // Start playing a fugue from program space.
    buzzer.playFromProgramSpace(fugue);

    // Wait until it is done playing.
    while(buzzer.isPlaying()){ }

    delay(1000);
  }
  else {
    //nothing
  }
}

/////////////////////////////////////////////////////////////MOVEMENT FUNCTIONS/////////////////////////////////////////////////////////////

void turnRight() {
  motors.setSpeeds(-turnSpeed, turnSpeed);
  turningLeft = false;
  turningRight = true;
}

void turnLeft() {
  motors.setSpeeds(turnSpeed, -turnSpeed);
  turningLeft = true;
  turningRight = false;
}

void moveForward() {
  motors.setSpeeds(-moveSpeed, -moveSpeed);
  movingForward = true;
  movingBackward = false;
}

void moveBackward() {
  motors.setSpeeds(moveSpeed, moveSpeed);
  movingForward = false;
  movingBackward = true;
}

void moveForwardForOneSecond() {
  unsigned long startTime = millis();

  while (millis() - startTime < 1000) {
    motors.setSpeeds(-moveSpeed, -moveSpeed);
    movingForward = true;    
  }
}

void stop() {
  motors.setSpeeds(0, 0);
  turningLeft = false;
  turningRight = false;
  movingForward = false;
}

/////////////////////////////////////////////////////////////MAIN LOOP////////////////////////////////////////////////////////////

void loop() {
 
  // Read the front proximity sensor and gets its left value (the amount of reflectance detected while using the left LEDs) and right value.
  proxSensors.read();
  uint8_t leftValue = proxSensors.countsFrontWithLeftLeds();
  uint8_t rightValue = proxSensors.countsFrontWithRightLeds();

  // Determine if an object is visible or not.
  bool objectSeen = leftValue >= frontSensorThreshold || rightValue >= frontSensorThreshold;

  if (objectSeen) {
    // An object is visible, so we will start decelerating in order to help the robot find the object without overshooting or oscillating.
    turnSpeed -= deceleration;
  }
  else {
    // An object is not visible, so we will accelerate in order to help find the object sooner.
    turnSpeed += acceleration;
  }

  // Constrain the turn speed so it is between turnSpeedMin and turnSpeedMax.
  turnSpeed = constrain(turnSpeed, turnSpeedMin, turnSpeedMax);

  if (objectSeen) {
      // An object seen.
      ledYellow(1);

      lastTimeObjectSeen = millis();

      if (leftValue < rightValue) { //Turn Right
        // The right value is greater, so the object is probably closer to the robot's right LEDs, which means the robot
        // is not facing it directly. Turn to the right to try to make it more even.
        turnRight();
        senseDir = RIGHT;
      }
      else if (leftValue > rightValue) { //Turn Left
        // The left value is greater, so turn to the left.
        turnLeft();
        senseDir = LEFT;
      }
      else {
        moveForward();
      }
    } 
    else {
      // No object is seen, so just keep turning in the direction that we last sensed the object.
      ledYellow(0);
      
      if (senseDir == RIGHT) {
        turnRight();
      }
      else {
        turnLeft();
      }
    }  

/////////////////////////////////////////////////////////////OLED DISPLAY DATA/////////////////////////////////////////////////////////////

  display.setLayout21x8();   

  //Display Front Sensor Data//
  display.gotoXY(0, 1);
  display.print("LS:");
  display.print(leftValue);
  display.print("  ");
  display.print("RS:");
  display.print(rightValue);
  display.gotoXY(0, 2);
  display.print(turningRight ? 'R' : (turningLeft ? 'L' : ' '));
  display.print(' ');
  display.print(turnSpeed);
  display.print(' ');
  display.gotoXY(0, 3);
  display.print(movingForward ? 'F' : (movingBackward ? 'B' : ' ')); 
  display.print(' '); 
  display.print(moveSpeed);
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
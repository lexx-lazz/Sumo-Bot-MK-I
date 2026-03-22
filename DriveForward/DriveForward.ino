//DRV8833 Pinout
#define IN1 5
#define IN2 4
#define IN3 7
#define IN4 6

// Ultrasonic sensor
const int trigPin = 10;
const int echoPin = 9;
float duration, distance;

//  turn left
void turnleft() {
  Serial.println("Turning Left");
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}

// turn right
void turnright() {
  Serial.println("Turning Right");
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
}

// go ahead
void go() {
  Serial.println("Moving Forwards");
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}

// go back
void back() {
  Serial.println("Moving Backwards");
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
}

// spinning in counter clockwise.
void spinCCW() {
  Serial.println("Spinning CCW");
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
}

// spinning in clockwise.
void spinCW() {
  Serial.println("Spinning CW");
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}

void initCar() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
}

// stop the car
void stopCar() {
  initCar();
}

float get_distance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  duration = pulseIn(echoPin, HIGH);
  distance = (duration * 0.034) / 2;
  Serial.println("Distance: ");
  Serial.print(distance);
  return distance;
}

void setup() {
  Serial.begin(9600);
  Serial.println("Init ultrasonic sensor...");
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  Serial.println("Init ultrasonic sensor...OK!");
  Serial.println("Init DRV8833 driver...");
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  Serial.println("Init DRV8833 driver...OK!");

  Serial.println("Init Car functions...");
  initCar();
  Serial.println("Car initializing finished!!!.");
}

 void loop() {
    distance = get_distance();
    
    if (distance <= 20) {
        // Turn left
        spinCCW();
        delay(500); // Adjust the delay according to your needs
        stopCar(); // Stop the car after turning
    } else {
        // Go forward
        go();
        delay(1000); // Adjust the delay according to your needs
        stopCar(); // Stop the car after moving forward
    }
}
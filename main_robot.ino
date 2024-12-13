#define FORWARD 1
#define BACKWARD 0

#define LEFT_FLAME_THRESHOLD 990
#define RIGHT_FLAME_THRESHOLD 990
#define HORIZ_FLAME_THRESHOLD 1018
#define WALL_THRESHOLD 375
#define WALL_THRESHOLD_DIAG 350
#define WALL_EMERGENCY 495

#define MOVE_SPEED 100
#define WALL_MOVE_SPEED 30
#define FIRE_MOVE_SPEED 20

// Digital Pins
int frontBumper = 11;
int sideBumper = 4;
int flameLight = 13;

// Analog Pins
int sharpHoriz = 5; 
int sharpDiag = 4; 
int flameSensorLeft = 1;
int flameSensorRight = 0;
int flameSensorHoriz = 3;

// Motor Pins
int rightMotorState = 4;
int rightMotorSpeed = 5;
int leftMotorState = 7;
int leftMotorSpeed = 6;
int fanPin = 0; 

// variables that change every cycle
int leftFlameLevel = 0;
int rightFlameLevel = 0;
int distanceHoriz = 0;
int distanceDiag = 0;
bool lastTurnRight = false;
int timeSinceLastTurn = 0;

void setup() {
  pinMode(frontBumper, INPUT); // initialize the touch sensor
  pinMode(sideBumper, INPUT); // initialize the touch sensor
  pinMode(flameLight, OUTPUT); // initialize the flame indicator led
  pinMode(fanPin, OUTPUT); // initialize the flame indicator led
  
  pinMode(rightMotorState, OUTPUT); // initalize the pin for settting the direction of the motor 1
  pinMode(leftMotorState, OUTPUT); // initalize the pin for settting the direction of the motor 2

  analogWrite(rightMotorSpeed, 0); // set the motor speed to 0 to make sure it isnt spinning
  digitalWrite(rightMotorState, FORWARD); // set the motor to spinning forward
  
  analogWrite(leftMotorSpeed, 0); // set the motor speed to 0 to make sure it isnt spinning
  digitalWrite(leftMotorState, FORWARD); // set the motor to spinning forward

  digitalWrite(flameLight, LOW);
  digitalWrite(fanPin, LOW);

  // read the values of the sharp sensors
  int time0 = analogRead(sharpDiag);
  delay(50);
  int time1 = analogRead(sharpDiag);

  // if the robot is facing the wrong way
  if((time0+time1)/2 > 290) {
    turnRobot(130, false); // turn the robot
  }
}

void loop() {
  // set the robot to moving forwards
  digitalWrite(leftMotorState, FORWARD);
  digitalWrite(rightMotorState, FORWARD);
  // update the state variables
  findWall();
  findFire();

  // check to see if there is a fire infront of the robot
  if(leftFlameLevel <= LEFT_FLAME_THRESHOLD || rightFlameLevel <= RIGHT_FLAME_THRESHOLD) {
    fire();
  }

  else if(analogRead(flameSensorHoriz) <= LEFT_FLAME_THRESHOLD) {
    turnRobot(130, true);
  }
  // check to see if the robot is stuck on a wall
  if(distanceHoriz >= WALL_EMERGENCY || distanceDiag >= WALL_EMERGENCY) {
    avoidWall();
  }
  // check to see if the robot is too close to a wall
  else if(distanceHoriz >= WALL_THRESHOLD || distanceDiag >= WALL_THRESHOLD_DIAG) {
    analogWrite(rightMotorSpeed, WALL_MOVE_SPEED);
    analogWrite(leftMotorSpeed, MOVE_SPEED);
  }
  // check to see if the robot is too far from a wall
  else if(distanceHoriz <= WALL_THRESHOLD || distanceDiag <= WALL_THRESHOLD_DIAG) {
    analogWrite(leftMotorSpeed, WALL_MOVE_SPEED);
    analogWrite(rightMotorSpeed, MOVE_SPEED);
  }

  // check to see if the robot hit a wall
  if(digitalRead(frontBumper) == LOW) {
    avoidWall();
  }

  if(digitalRead(sideBumper) == LOW) {
    avoidWall();
  }

  if(timeSinceLastTurn > 5000) {
    avoidWall();
  }
  
  delay(50);
  timeSinceLastTurn += 50;
}

void findWall() {
  // get the distances from the sharp sensors
  distanceHoriz = analogRead(sharpHoriz);
  distanceDiag = analogRead(sharpDiag);
}

void findFire() {
  // get the heat levels from the flame sensors
  leftFlameLevel = analogRead(flameSensorLeft);
  rightFlameLevel = analogRead(flameSensorRight);

}

int turnRobot(int degrees, bool isTurningRight) { 
  // set both wheels to max speed
  analogWrite(leftMotorSpeed, MOVE_SPEED);
  analogWrite(rightMotorSpeed, MOVE_SPEED);
  // set the states of the wheels depending in whether the robot is turning left or right
  if(isTurningRight) {
    digitalWrite(rightMotorState, BACKWARD);
    digitalWrite(leftMotorState, FORWARD);
  }
  else {
    digitalWrite(rightMotorState, FORWARD);
    digitalWrite(leftMotorState, BACKWARD);
  }
  // wait for an amount of time dictated by the degrees the robot should turn
  delay(degrees*5);

  analogWrite(rightMotorSpeed, MOVE_SPEED);
  analogWrite(leftMotorSpeed, MOVE_SPEED);
  digitalWrite(rightMotorState, FORWARD);
  digitalWrite(leftMotorState, FORWARD);
}

void fire() {

  if(num > 4) { return; }
  // turn on the light and the fan
  digitalWrite(flameLight, HIGH);
  digitalWrite(fanPin, HIGH);

  // slow the robot down and slowly aproach the fire
  digitalWrite(leftMotorState, FORWARD);
  digitalWrite(rightMotorState, FORWARD);
  analogWrite(leftMotorSpeed, FIRE_MOVE_SPEED);
  analogWrite(rightMotorSpeed, FIRE_MOVE_SPEED);

  if(leftFlameLevel > rightFlameLevel) {
    turnRobot(10, false);
  }
  else if(leftFlameLevel < rightFlameLevel) {
    turnRobot(10, true);
  }

  delay(1000);

  // re check how much fire there is
  findFire();

  // if the robot hit a wall
  if(digitalRead(frontBumper) == LOW) {
    digitalWrite(fanPin, LOW); // turn off the fan
    digitalWrite(flameLight, LOW); // turn off the flame indicator
    avoidWall();
  }

  // if the robot still sees a fire in front of it
  if(leftFlameLevel <= LEFT_FLAME_THRESHOLD || rightFlameLevel <= RIGHT_FLAME_THRESHOLD) {
    fire(); // try to put out the fire
  }
  // if there is no longer a fire in front of the robot
  else {
    digitalWrite(flameLight, LOW); // turn off the flame indicator
    digitalWrite(fanPin, LOW); // turn off the fan

    analogWrite(leftMotorSpeed, MOVE_SPEED); // resume moving at the normal speed
    analogWrite(rightMotorSpeed, MOVE_SPEED);
  }
}

void avoidWall() {
  bool shouldTurnRight = false;

  digitalWrite(rightMotorState, BACKWARD);
  digitalWrite(leftMotorState, BACKWARD);
  analogWrite(leftMotorSpeed, MOVE_SPEED);
  analogWrite(rightMotorSpeed, MOVE_SPEED);
  delay(100);

  if(timeSinceLastTurn < 1000) {
    shouldTurnRight = lastTurnRight;
  }

  else if(distanceHoriz >= WALL_THRESHOLD || distanceDiag >= WALL_THRESHOLD_DIAG) {
    shouldTurnRight = true;
    lastTurnRight = true;
  }
  else {
    shouldTurnRight = false;
    lastTurnRight = false;
  }

  timeSinceLastTurn = 0;
  turnRobot(30, shouldTurnRight);
}
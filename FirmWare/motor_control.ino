#include "config.h"
#include <SparkFun_TB6612.h>

// Motor objects
const int offsetA = 1;
const int offsetB = 1;
Motor motor1 = Motor(AIN1, AIN2, PWMA, offsetA, STBY);
Motor motor2 = Motor(BIN1, BIN2, PWMB, offsetB, STBY);

// Motor control functions
void driveMotors(int leftSpeed, int rightSpeed) {
  motor1.drive(constrain(leftSpeed, -255, 255));
  motor2.drive(constrain(rightSpeed, -255, 255));
}

void stopMotors() {
  motor1.drive(0);
  motor2.drive(0);
}

void moveForward(int speed, int duration = 0) {
  driveMotors(speed, speed);
  if (duration > 0) {
    delay(duration);
    stopMotors();
  }
}

void turnLeft(int turnDelay = TURN_DELAY) {
  driveMotors(-TURN_SPEED, TURN_SPEED);
  delay(turnDelay);
  stopMotors();
  delay(50);
}

void turnRight(int turnDelay = TURN_DELAY) {
  driveMotors(TURN_SPEED, -TURN_SPEED);
  delay(turnDelay);
  stopMotors();
  delay(50);
}

void uTurn(int turnDelay = U_TURN_DELAY) {
  driveMotors(TURN_SPEED, -TURN_SPEED);
  delay(turnDelay);
  stopMotors();
  delay(50);
}
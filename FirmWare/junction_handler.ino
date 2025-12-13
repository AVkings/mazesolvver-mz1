#include "config.h"

void handleJunctionLSRB() {
  // Check available paths
  bool leftAvailable = checkPath(0);      // Leftmost sensor
  bool straightAvailable = checkPath(NUM_SENSORS/2);  // Center sensor
  bool rightAvailable = checkPath(NUM_SENSORS-1);     // Rightmost sensor
  
  // LSRB priority: Left → Straight → Right → Back
  if (leftAvailable) {
    turnLeft();
    return;
  }
  
  if (straightAvailable) {
    moveForward(TURN_SPEED, JUNCTION_CROSS_TIME);
    return;
  }
  
  if (rightAvailable) {
    // Verify straight path first
    moveForward(TURN_SPEED, 120);
    delay(20);
    readSensorArray();
    
    if (IS_BLACK_LINE ? sensorValue[NUM_SENSORS/2] > 700 : sensorValue[NUM_SENSORS/2] < 300) {
      moveForward(TURN_SPEED, JUNCTION_CROSS_TIME);  // Actually straight
    } else {
      turnRight();  // Take right turn
    }
    return;
  }
  
  // No options - U-turn
  uTurn();
}

bool checkPath(int sensorIndex) {
  if (IS_BLACK_LINE) {
    return (sensorValue[sensorIndex] > JUNCTION_THRESHOLD);
  } else {
    return (sensorValue[sensorIndex] < (1000 - JUNCTION_THRESHOLD));
  }
}

void handleFinishLine() {
  // Robot stops and blinks LED
  stopMotors();
  while (1) {
    digitalWrite(13, HIGH);
    delay(300);
    digitalWrite(13, LOW);
    delay(300);
  }
}
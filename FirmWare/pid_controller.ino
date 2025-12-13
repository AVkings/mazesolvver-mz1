#include "config.h"

// PID variables
int P, D, I, previousError = 0, PIDvalue, error;
int lsp, rsp;
int currentSpeed = 15;

void calculatePID() {
  // Calculate weighted error based on sensor positions
  if (NUM_SENSORS == 7) {
    error = (3 * sensorValue[0] + 2 * sensorValue[1] + sensorValue[2]
            - sensorValue[4] - 2 * sensorValue[5] - 3 * sensorValue[6]);
  } else {
    error = (3 * sensorValue[1] + sensorValue[2]
            - sensorValue[4] - 3 * sensorValue[5]);
  }
  
  // Adjust for line thickness and color
  if (LINE_THICKNESS > 22) error = -error;
  if (IS_BLACK_LINE) error = -error;
  
  // PID calculation
  P = error;
  I = I + error;
  D = error - previousError;
  
  PIDvalue = (KP * P) + (KI * I) + (KD * D);
  previousError = error;
  
  // Calculate motor speeds
  lsp = currentSpeed - PIDvalue;
  rsp = currentSpeed + PIDvalue;
  
  // Constrain speeds
  lsp = constrain(lsp, 0, 255);
  rsp = constrain(rsp, 0, 255);
  
  // Apply acceleration
  if (currentSpeed < LF_SPEED) currentSpeed++;
}

void performLineFollow() {
  calculatePID();
  driveMotors(lsp, rsp);
}

void handleOffLine() {
  static bool brakeFlag = 0;
  
  if (error > 0) {
    if (BRAKE_ENABLED && !brakeFlag) {
      stopMotors();
      delay(30);
    }
    driveMotors(-100, 150);  // Turn right to find line
    brakeFlag = 1;
  } else {
    if (BRAKE_ENABLED && !brakeFlag) {
      stopMotors();
      delay(30);
    }
    driveMotors(150, -100);  // Turn left to find line
    brakeFlag = 1;
  }
}
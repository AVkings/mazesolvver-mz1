#include "config.h"

// Sensor arrays
int minValues[NUM_SENSORS], maxValues[NUM_SENSORS];
int threshold[NUM_SENSORS], sensorValue[NUM_SENSORS];
int onLine = 1;  // 1 = on line, 0 = off line

void initializeSensorPins() {
  // Configure ADC for faster reading
  sbi(ADCSRA, ADPS2);
  cbi(ADCSRA, ADPS1);
  cbi(ADCSRA, ADPS0);
}

void calibrateSensors() {
  Serial.println("Starting calibration...");
  
  // Initialize min/max values
  for (int i = 0; i < NUM_SENSORS; i++) {
    minValues[i] = analogRead(i);
    maxValues[i] = analogRead(i);
  }
  
  // Rotate robot to scan all surfaces
  for (int t = 0; t < 10000; t++) {
    driveMotors(50, -50);  // Rotate in place
    for (int i = 0; i < NUM_SENSORS; i++) {
      int reading = analogRead(i);
      if (reading < minValues[i]) minValues[i] = reading;
      if (reading > maxValues[i]) maxValues[i] = reading;
    }
  }
  
  // Calculate thresholds
  Serial.print("Thresholds: ");
  for (int i = 0; i < NUM_SENSORS; i++) {
    threshold[i] = (minValues[i] + maxValues[i]) / 2;
    Serial.print(threshold[i]);
    Serial.print(" ");
  }
  Serial.println();
  
  stopMotors();
  Serial.println("Calibration complete!");
}

void readSensorArray() {
  onLine = 0;
  
  for (int i = 0; i < NUM_SENSORS; i++) {
    // Map reading to 0-1000 range
    sensorValue[i] = map(analogRead(i), minValues[i], maxValues[i], 0, 1000);
    sensorValue[i] = constrain(sensorValue[i], 0, 1000);
    
    // Check if sensor detects line
    if (IS_BLACK_LINE) {
      if (sensorValue[i] > ON_LINE_THRESHOLD) onLine = 1;
    } else {
      if (sensorValue[i] < ON_LINE_THRESHOLD) onLine = 1;
    }
  }
}

bool checkAllSensorsOnLine() {
  for (int i = 0; i < NUM_SENSORS; i++) {
    if (IS_BLACK_LINE) {
      if (sensorValue[i] < 700) return false;
    } else {
      if (sensorValue[i] > 300) return false;
    }
  }
  return true;
}
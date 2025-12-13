#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif

#include <SparkFun_TB6612.h>

// Enter Line Details
bool isBlackLine = 0;           // 1 = black line, 0 = white line
unsigned int lineThickness = 30;  // mm, 10–35
unsigned int numSensors = 7;      // 5 or 7 sensors
bool brakeEnabled = 0;
bool L = 0;
bool M = 0;
bool R = 0;

#define AIN1 4
#define BIN1 6
#define AIN2 3
#define BIN2 7
#define PWMA 9
#define PWMB 10
#define STBY 5

const int offsetA = 1;
const int offsetB = 1;

Motor motor1 = Motor(AIN1, AIN2, PWMA, offsetA, STBY);
Motor motor2 = Motor(BIN1, BIN2, PWMB, offsetB, STBY);

int P, D, I, previousError, PIDvalue, error;
int lsp, rsp;
int lfSpeed = 55;
int currentSpeed = 15;

float Kp = 0.15;
float Kd = 5.65;
float Ki = 0.20;

int onLine = 1;
int minValues[7], maxValues[7], threshold[7], sensorValue[7];
bool brakeFlag = 0;

void setup() {
  sbi(ADCSRA, ADPS2);
  cbi(ADCSRA, ADPS1);
  cbi(ADCSRA, ADPS0);

  Serial.begin(9600);
  pinMode(11, INPUT_PULLUP);
  pinMode(12, INPUT_PULLUP);
  pinMode(13, OUTPUT);
  lineThickness = constrain(lineThickness, 10, 35);
}

void loop() {
  while (digitalRead(11)) {}  // wait for calibration start
  delay(1000);
  calibrate();
  while (digitalRead(12)) {}  // wait for start
  delay(1000);

  while (1) {
    readLine();

    // Check if all sensors are on the line
    bool allSensorsOnLine = true;
    for (int i = 0; i < 7; i++) {
      if (isBlackLine) {
        if (sensorValue[i] < 700) {
          allSensorsOnLine = false;
          break;
        }
      } else {
        if (sensorValue[i] > 300) {
          allSensorsOnLine = false;
          break;
        }
      }
    }

    if (allSensorsOnLine) {
      // Move forward a little
      motor1.drive(80);
      motor2.drive(80);
      delay(100);
      motor1.drive(0);
      motor2.drive(0);

      readLine();  // check sensors again

      // Check if all sensors still detect the line
      bool allSensorsStillOnLine = true;
      for (int i = 0; i < 7; i++) {
        if (isBlackLine) {
          if (sensorValue[i] < 700) {
            allSensorsStillOnLine = false;
            break;
          }
        } else {
          if (sensorValue[i] > 300) {
            allSensorsStillOnLine = false;
            break;
          }
        }
      }

      if (allSensorsStillOnLine) {
        // Line continues on all sensors → Stop the robot
        motor1.drive(0);
        motor2.drive(0);
        while (1) {
          digitalWrite(13, HIGH);
          delay(300);
          digitalWrite(13, LOW);
          delay(300);
        }
      } else {
        // Line did not continue on all sensors → move left as per LSRB
        handleJunction_LSRB();
        continue;
      }
    }

    // For all other cases, do PID line following normally

    if (currentSpeed < lfSpeed) currentSpeed++;
    if (onLine == 1) {
      linefollow();
      digitalWrite(13, HIGH);
      brakeFlag = 0;
    } else {
      digitalWrite(13, LOW);
      if (error > 0) {
        if (brakeEnabled && !brakeFlag) {
          motor1.drive(0);
          motor2.drive(0);
          delay(30);
        }
        motor1.drive(-100);
        motor2.drive(150);
        brakeFlag = 1;
      } else {
        if (brakeEnabled && !brakeFlag) {
          motor1.drive(0);
          motor2.drive(0);
          delay(30);
        }
        motor1.drive(150);
        motor2.drive(-100);
        brakeFlag = 1;
      }
    }
  }
}


void handleJunction_LSRB() {
  // Check left, straight, right, back in order using sensor data
  bool leftAvailable = false;
  bool straightAvailable = false;
  bool rightAvailable = false;

  // For 7 sensors, leftmost is sensorValue[0], center is sensorValue[3], rightmost is sensorValue[6]
  if (isBlackLine) {
    leftAvailable = (sensorValue[0] > 500);
    straightAvailable = (sensorValue[3] > 500);
    rightAvailable = (sensorValue[6] > 500);
  } else {
    leftAvailable = (sensorValue[0] < 600);
    straightAvailable = (sensorValue[3] < 600);
    rightAvailable = (sensorValue[6] < 600);
  }

  if (leftAvailable) {
    turnLeft();
    return;

  }

  if (straightAvailable) {
    goStraight();
    return;
  }

  if (rightAvailable) {
    // Move ahead a little to verify straight path
    motor1.drive(80);
    motor2.drive(80);
    delay(120); // Small forward movement
    motor1.drive(0);
    motor2.drive(0);
    delay(20);

    readLine();
    if (isBlackLine ? sensorValue[3] > 700 : sensorValue[3] < 300) {
      // Found straight after moving forward
      goStraight();
    } else {
      // No straight, so take right
      turnRight();
    }
    return;
  }

  // No options, turn back
  turnBack();
}

void turnRight() {
  // Slow and precise right turn (~90 degrees)
  motor1.drive(80);   // Left wheel forward
  motor2.drive(-80);  // Right wheel backward
  delay(150);         // Adjust this value carefully
  motor1.drive(0);
  motor2.drive(0);
  delay(50);          // Small pause before resuming
}

void turnLeft() {
  // Slow and precise left turn (~90 degrees)
  motor1.drive(-80);  // Left wheel backward
  motor2.drive(80);   // Right wheel forward
  delay(150);         // Adjust this value carefully
  motor1.drive(0);
  motor2.drive(0);
  delay(50);          // Small pause before resuming
}

void turnBack() {
  // U-turn (~180 degrees)
  motor1.drive(80);   // Left wheel forward
  motor2.drive(-80);  // Right wheel backward
  delay(300);         // Double the delay for 180 degree turn
  motor1.drive(0);
  motor2.drive(0);
  delay(50);          // Small pause before resuming
}

void goStraight() {
  // Move straight through junction
  motor1.drive(80);   // Both wheels forward
  motor2.drive(80);
  delay(200);         // Move forward to clear junction
  motor1.drive(0);
  motor2.drive(0);
  delay(50);          // Small pause before resuming
}

void linefollow() {
  if (numSensors == 7) {
    error = (3 * sensorValue[0] + 2 * sensorValue[1] + sensorValue[2]
            - sensorValue[4] - 2 * sensorValue[5] - 3 * sensorValue[6]);
  } else {
    error = (3 * sensorValue[1] + sensorValue[2]
            - sensorValue[4] - 3 * sensorValue[5]);
  }

  if (lineThickness > 22) {
    error = -error;
  }
  if (isBlackLine) {
    error = -error;
  }

  P = error;
  I = I + error;
  D = error - previousError;

  PIDvalue = (Kp * P) + (Ki * I) + (Kd * D);
  previousError = error;

  lsp = currentSpeed - PIDvalue;
  rsp = currentSpeed + PIDvalue;

  lsp = constrain(lsp, 0, 255);
  rsp = constrain(rsp, 0, 255);

  motor1.drive(lsp);
  motor2.drive(rsp);
}

void calibrate() {
  for (int i = 0; i < 7; i++) {
    minValues[i] = analogRead(i);
    maxValues[i] = analogRead(i);
  }

  for (int t = 0; t < 10000; t++) {
    motor1.drive(50);
    motor2.drive(-50);
    for (int i = 0; i < 7; i++) {
      int r = analogRead(i);
      if (r < minValues[i]) minValues[i] = r;
      if (r > maxValues[i]) maxValues[i] = r;
    }
  }

  for (int i = 0; i < 7; i++) {
    threshold[i] = (minValues[i] + maxValues[i]) / 2;
    Serial.print(threshold[i]);
    Serial.print(" ");
  }
  Serial.println();
  motor1.drive(0);
  motor2.drive(0);
}

void readLine() {
  onLine = 0;
  if (numSensors == 7) {
    for (int i = 0; i < 7; i++) {
      sensorValue[i] = map(analogRead(i), minValues[i], maxValues[i], 0, 1000);
      sensorValue[i] = constrain(sensorValue[i], 0, 1000);
      if (isBlackLine) {
        if (sensorValue[i] > 700) onLine = 1;
      } else {
        if (sensorValue[i] < 700) onLine = 1;
      }
    }
  } else {
    for (int i = 1; i < 6; i++) {
      sensorValue[i] = map(analogRead(i), minValues[i], maxValues[i], 0, 1000);
      sensorValue[i] = constrain(sensorValue[i], 0, 1000);
      if (isBlackLine) {
        if (sensorValue[i] > 700) onLine = 1;
      } else {
        if (sensorValue[i] < 700) onLine = 1;
      }
    }
  }
}

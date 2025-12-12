#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif

#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif

#include <SparkFun_TB6612.h>

// ------------------- Line Details -------------------
bool isBlackLine = 0;             // 0 = white line (as per competition)
unsigned int lineThickness = 30;  // 30mm as per competition rules
unsigned int numSensors = 7;      
bool brakeEnabled = 1;

// ------------------- Competition Variables -------------------
bool isDryRun = true;
bool competitionStarted = false;
unsigned long dryRunStartTime = 0;
unsigned long actualRunStartTime = 0;
const unsigned long DRY_RUN_TIME = 180000;  // 3 minutes
const unsigned long ACTUAL_RUN_TIME = 150000; // 2.5 minutes

// ------------------- Maze Mapping -------------------
char exploredPath[200] = "";
char shortestPath[100] = "";
int pathIndex = 0;
int intersectionCount = 0;
int checkpointsVisited = 0;
bool finishDetected = false;

// ------------------- LSRB State Machine -------------------
enum RobotState { FOLLOWING, AT_INTERSECTION, TURNING, FINISHED, BACKTRACKING };
RobotState currentState = FOLLOWING;

enum Direction { LEFT, STRAIGHT, RIGHT, BACK };
Direction nextMove = STRAIGHT;

// ------------------- Motor Pins ---------------------
#define AIN1 4
#define BIN1 6
#define AIN2 3
#define BIN2 7
#define PWMA 9
#define PWMB 10
#define STBY 5

// ------------------- Motor Config -------------------
const int offsetA = 1;
const int offsetB = 1;

Motor motor1 = Motor(AIN1, AIN2, PWMA, offsetA, STBY);
Motor motor2 = Motor(BIN1, BIN2, PWMB, offsetB, STBY);

// ------------------- PID Variables ------------------
int P, D, I, previousError, PIDvalue, error;
int lsp, rsp;
int lfSpeed = 120;
int currentSpeed = 80;

float Kp = 0.07;
float Kd = 0.85;
float Ki = 0.0001;

// ------------------- Sensors ------------------------
int onLine = 1;
int minValues[7], maxValues[7], threshold[7];
int sensorValue[7];
bool intersectionDetected = false;

// ====================================================
//                      SETUP
// ====================================================
void setup() {
  // Faster ADC reading
  sbi(ADCSRA, ADPS2);
  cbi(ADCSRA, ADPS1);
  cbi(ADCSRA, ADPS0);

  Serial.begin(9600);

  pinMode(11, INPUT_PULLUP);  // Button 1 - CALIBRATE
  pinMode(12, INPUT_PULLUP);  // Button 2 - START DRY RUN
  pinMode(13, OUTPUT);        // Red LED for finish

  Serial.println("=== TECHFEST MESHMERIZE BOT ===");
  Serial.println("BUTTON GUIDE:");
  Serial.println("BUTTON 11 (D11) - CALIBRATE SENSORS");
  Serial.println("BUTTON 12 (D12) - START DRY RUN");
  Serial.println("");
  Serial.println("WAITING FOR CALIBRATION...");
}

// ====================================================
//                       LOOP
// ====================================================
void loop() {
  // STEP 1: Wait for CALIBRATION button (Button 11)
  if (!competitionStarted) {
    Serial.println("Press BUTTON 11 to CALIBRATE sensors");
    while (digitalRead(11)) { 
      delay(100); 
    }
    delay(500);
    calibrate();
    
    // STEP 2: Wait for DRY RUN start button (Button 12)
    Serial.println("Calibration complete!");
    Serial.println("Press BUTTON 12 to START DRY RUN");
    while (digitalRead(12)) { 
      delay(100); 
    }
    delay(1000);
    
    competitionStarted = true;
    dryRunStartTime = millis();
    Serial.println("=== DRY RUN STARTED ===");
    Serial.println("Time: 3 minutes");
    isDryRun = true;
    finishDetected = false;

    // Dry Run - Explore all paths
    dryRun();

    // Calculate shortest path
    calculateShortestPath();

    // Automatic transition to Actual Run
    Serial.println("=== ACTUAL RUN STARTED ===");
    Serial.println("Time: 2.5 minutes");
    isDryRun = false;
    actualRunStartTime = millis();
    
    actualRun();

    // Competition complete
    printScoringReport();
    
    Serial.println("=== COMPETITION COMPLETE ===");
    Serial.println("Reset Arduino to start again");
    while(1) { 
      // Blink LED to indicate completion
      digitalWrite(13, HIGH);
      delay(500);
      digitalWrite(13, LOW);
      delay(500);
    }
  }
}

// ====================================================
//                    DRY RUN
// ====================================================
void dryRun() {
  currentState = FOLLOWING;
  pathIndex = 0;
  exploredPath[0] = '\0';
  intersectionCount = 0;
  checkpointsVisited = 0;

  while (isDryRun && (millis() - dryRunStartTime < DRY_RUN_TIME) && !finishDetected) {
    readLine();

    if (detectFinishBox()) {
      finishDetected = true;
      triggerFinishLED();
      Serial.println("*** FINISH BOX DETECTED IN DRY RUN ***");
      recordMove('F'); // F for Finish
      // Continue exploring other paths if time remains
      if (millis() - dryRunStartTime < DRY_RUN_TIME - 30000) {
        Serial.println("Backtracking to explore other paths...");
        executeTurn(BACK);
        finishDetected = false;
      }
      continue;
    }

    switch (currentState) {
      case FOLLOWING:
        if (detectIntersection()) {
          currentState = AT_INTERSECTION;
          safeStop();
          intersectionCount++;
          checkForCheckpoint();
          Serial.print("Intersection #");
          Serial.println(intersectionCount);
        } else {
          linefollow();
        }
        break;

      case AT_INTERSECTION:
        nextMove = getNextMoveDryRun();
        recordMove(getMoveChar(nextMove));
        Serial.print("Dry Run Move: ");
        Serial.println(getMoveChar(nextMove));
        currentState = TURNING;
        break;

      case TURNING:
        executeTurn(nextMove);
        currentState = FOLLOWING;
        currentSpeed = 80; // Reset speed after turn
        break;

      default:
        break;
    }

    printDryRunStatus();
    delay(10);
  }

  Serial.println("=== DRY RUN COMPLETED ===");
  Serial.print("Explored Path: ");
  Serial.println(exploredPath);
  Serial.print("Intersections found: ");
  Serial.println(intersectionCount);
  Serial.print("Checkpoints visited: ");
  Serial.println(checkpointsVisited);
}

// ====================================================
//                    ACTUAL RUN
// ====================================================
void actualRun() {
  currentState = FOLLOWING;
  int shortestPathIndex = 0;
  finishDetected = false;

  Serial.print("Following Shortest Path: ");
  Serial.println(shortestPath);

  while (!finishDetected && (millis() - actualRunStartTime < ACTUAL_RUN_TIME)) {
    readLine();

    if (detectFinishBox()) {
      finishDetected = true;
      triggerFinishLED();
      Serial.println("*** FINISHED ACTUAL RUN! ***");
      break;
    }

    switch (currentState) {
      case FOLLOWING:
        if (detectIntersection()) {
          currentState = AT_INTERSECTION;
          safeStop();
          checkForCheckpoint();
        } else {
          linefollow();
          // Higher speed in actual run
          if (currentSpeed < lfSpeed + 20) currentSpeed++;
        }
        break;

      case AT_INTERSECTION:
        if (shortestPath[shortestPathIndex] != '\0') {
          nextMove = getMoveFromChar(shortestPath[shortestPathIndex++]);
          Serial.print("Actual Run Move: ");
          Serial.println(getMoveChar(nextMove));
          currentState = TURNING;
        } else {
          // Reached end of shortest path but no finish detected
          Serial.println("ERROR: End of path but no finish detected!");
          currentState = FOLLOWING;
        }
        break;

      case TURNING:
        executePreciseTurn(nextMove);
        currentState = FOLLOWING;
        currentSpeed = 100; // Faster start after turns
        break;

      default:
        break;
    }

    // Print actual run status every 2 seconds
    static unsigned long lastActualPrint = 0;
    if (millis() - lastActualPrint > 2000) {
      int timeLeft = (ACTUAL_RUN_TIME - (millis() - actualRunStartTime)) / 1000;
      Serial.print("Actual Run - Time left: ");
      Serial.print(timeLeft);
      Serial.print("s | Next move: ");
      if (shortestPath[shortestPathIndex] != '\0') {
        Serial.println(shortestPath[shortestPathIndex]);
      } else {
        Serial.println("END");
      }
      lastActualPrint = millis();
    }

    delay(10);
  }

  if (!finishDetected) {
    Serial.println("*** ACTUAL RUN TIME EXPIRED ***");
  }
}

// ====================================================
//        MAZE SOLVING ALGORITHMS
// ====================================================
Direction getNextMoveDryRun() {
  // LSRB Algorithm for Dry Run
  if (checkLeftPath()) return LEFT;
  if (checkStraightPath()) return STRAIGHT;
  if (checkRightPath()) return RIGHT;
  return BACK;
}

void calculateShortestPath() {
  Serial.println("Calculating shortest path...");
  
  // Simple optimization for now - remove backtracks
  optimizePath();
  
  strcpy(shortestPath, exploredPath);
  
  // Remove the 'F' finish marker if present
  for (int i = 0; shortestPath[i] != '\0'; i++) {
    if (shortestPath[i] == 'F') {
      shortestPath[i] = '\0';
      break;
    }
  }
  
  Serial.print("Optimized Shortest Path: ");
  Serial.println(shortestPath);
}

void optimizePath() {
  bool changed = true;
  while (changed) {
    changed = false;
    
    for (int i = 0; exploredPath[i+2] != '\0'; i++) {
      if (exploredPath[i] == 'L' && exploredPath[i+1] == 'B') {
        exploredPath[i] = 'R';
        shiftPath(i+1);
        changed = true;
      }
      else if (exploredPath[i] == 'R' && exploredPath[i+1] == 'B') {
        exploredPath[i] = 'L';
        shiftPath(i+1);
        changed = true;
      }
    }
  }
}

// ====================================================
//        COMPETITION DETECTION ALGORITHMS
// ====================================================
bool detectIntersection() {
  int sensorsOnLine = 0;
  
  for (int i = 0; i < numSensors; i++) {
    bool sensorOnLine = (!isBlackLine && sensorValue[i] > 700);
    if (sensorOnLine) sensorsOnLine++;
  }

  if (sensorsOnLine >= 5 && !intersectionDetected) {
    intersectionDetected = true;
    return true;
  }

  if (sensorsOnLine < 3) {
    intersectionDetected = false;
  }

  return false;
}

bool detectFinishBox() {
  int sensorsOnLine = 0;
  
  for (int i = 0; i < numSensors; i++) {
    bool sensorOnLine = (!isBlackLine && sensorValue[i] > 700);
    if (sensorOnLine) sensorsOnLine++;
  }

  if (sensorsOnLine >= 6) {
    return confirmFinishBox();
  }
  
  return false;
}

bool confirmFinishBox() {
  // Store initial reading
  int initialOnLine = 0;
  for (int i = 0; i < numSensors; i++) {
    if (sensorValue[i] > 700) initialOnLine++;
  }

  // Quick forward check
  motor1.drive(80);
  motor2.drive(80);
  delay(200);
  safeStop();
  
  readLine();
  
  // Check if still wide
  int finalOnLine = 0;
  for (int i = 0; i < numSensors; i++) {
    if (sensorValue[i] > 700) finalOnLine++;
  }
  
  // Return to position
  motor1.drive(-80);
  motor2.drive(-80);
  delay(200);
  safeStop();
  
  // If still wide after moving, it's a finish box
  return (finalOnLine >= 5);
}

void checkForCheckpoint() {
  // Simple checkpoint detection
  if (intersectionCount % 2 == 0 && intersectionCount > 0) {
    checkpointsVisited++;
    Serial.print("*** CHECKPOINT #");
    Serial.print(checkpointsVisited);
    Serial.println(" REACHED ***");
  }
}

// ====================================================
//        YOUR PID LINE FOLLOWING
// ====================================================
void linefollow() {
  if (numSensors == 7) {
    error = (3 * sensorValue[0] +
             2 * sensorValue[1] +
             sensorValue[2] -
             sensorValue[4] -
             2 * sensorValue[5] -
             3 * sensorValue[6]);
  }

  if (lineThickness > 22) error = -error;
  if (isBlackLine) error = -error;

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

// ====================================================
//        PATH MANAGEMENT
// ====================================================
void recordMove(char move) {
  if (pathIndex < 199) {
    exploredPath[pathIndex++] = move;
    exploredPath[pathIndex] = '\0';
  }
}

char getMoveChar(Direction move) {
  switch (move) {
    case LEFT: return 'L';
    case STRAIGHT: return 'S';
    case RIGHT: return 'R';
    case BACK: return 'B';
    default: return '?';
  }
}

Direction getMoveFromChar(char move) {
  switch (move) {
    case 'L': return LEFT;
    case 'S': return STRAIGHT;
    case 'R': return RIGHT;
    case 'B': return BACK;
    default: return STRAIGHT;
  }
}

void shiftPath(int pos) {
  for (int i = pos; exploredPath[i] != '\0'; i++) {
    exploredPath[i] = exploredPath[i + 1];
  }
  pathIndex--;
}

// ====================================================
//        SENSOR & MOTOR FUNCTIONS
// ====================================================
bool checkLeftPath() {
  motor1.drive(-60);
  motor2.drive(60);
  delay(80);
  bool leftPath = (analogRead(0) > threshold[0]);
  motor1.drive(60);
  motor2.drive(-60);
  delay(80);
  safeStop();
  return leftPath;
}

bool checkStraightPath() {
  return (sensorValue[2] > 500 || sensorValue[3] > 500 || sensorValue[4] > 500);
}

bool checkRightPath() {
  motor1.drive(60);
  motor2.drive(-60);
  delay(80);
  bool rightPath = (analogRead(6) > threshold[6]);
  motor1.drive(-60);
  motor2.drive(60);
  delay(80);
  safeStop();
  return rightPath;
}

void executeTurn(Direction turn) {
  switch (turn) {
    case LEFT:
      motor1.drive(-80);
      motor2.drive(80);
      delay(350);
      break;
    case STRAIGHT:
      motor1.drive(100);
      motor2.drive(100);
      delay(250);
      break;
    case RIGHT:
      motor1.drive(80);
      motor2.drive(-80);
      delay(350);
      break;
    case BACK:
      motor1.drive(80);
      motor2.drive(-80);
      delay(650);
      break;
  }
  safeStop();
}

void executePreciseTurn(Direction turn) {
  switch (turn) {
    case LEFT:
      motor1.drive(-90);
      motor2.drive(90);
      delay(320);
      break;
    case STRAIGHT:
      motor1.drive(120);
      motor2.drive(120);
      delay(200);
      break;
    case RIGHT:
      motor1.drive(90);
      motor2.drive(-90);
      delay(320);
      break;
    case BACK:
      motor1.drive(90);
      motor2.drive(-90);
      delay(620);
      break;
  }
  safeStop();
}

// ====================================================
//        CALIBRATION & UTILITIES
// ====================================================
void calibrate() {
  digitalWrite(13, HIGH);
  Serial.println("Calibrating sensors...");

  for (int i = 0; i < 7; i++) {
    minValues[i] = analogRead(i);
    maxValues[i] = analogRead(i);
  }

  for (int i = 0; i < 3000; i++) {
    motor1.drive(60);
    motor2.drive(-60);

    for (int j = 0; j < 7; j++) {
      int val = analogRead(j);
      if (val < minValues[j]) minValues[j] = val;
      if (val > maxValues[j]) maxValues[j] = val;
    }
  }

  Serial.print("Calibration complete. Thresholds: ");
  for (int i = 0; i < 7; i++) {
    threshold[i] = (minValues[i] + maxValues[i]) / 2;
    Serial.print(threshold[i]);
    Serial.print(" ");
  }
  Serial.println();

  motor1.brake();
  motor2.brake();
  digitalWrite(13, LOW);
  delay(1000);
}

void readLine() {
  onLine = 0;
  for (int i = 0; i < numSensors; i++) {
    sensorValue[i] = map(analogRead(i), minValues[i], maxValues[i], 0, 1000);
    sensorValue[i] = constrain(sensorValue[i], 0, 1000);

    if (!isBlackLine && sensorValue[i] > 700) onLine = 1;
  }
}

void safeStop() {
  motor1.brake();
  motor2.brake();
  delay(50);
}

void triggerFinishLED() {
  digitalWrite(13, HIGH);
  Serial.println("*** RED LED ACTIVATED - FINISH ***");
}

// ====================================================
//        COMPETITION SCORING & STATUS
// ====================================================
void printScoringReport() {
  int dryRunTime = (millis() - dryRunStartTime) / 1000;
  int actualRunTime = finishDetected ? (millis() - actualRunStartTime) / 1000 : ACTUAL_RUN_TIME / 1000;

  int A = 25 * checkpointsVisited;
  int B = 30; // Dry run completion
  int C = max(0, 180 - dryRunTime);
  int S = finishDetected ? 30 : 0;
  int T = finishDetected ? max(0, 150 - actualRunTime) : 0;
  int L = 5;

  int totalScore = A + B + C + S + T + L;

  Serial.println("=== FINAL SCORING ===");
  Serial.print("Checkpoints (25×"); Serial.print(checkpointsVisited); Serial.print("): "); Serial.println(A);
  Serial.print("Dry Run Completion: "); Serial.println(B);
  Serial.print("Dry Run Time Bonus (180-"); Serial.print(dryRunTime); Serial.print("): "); Serial.println(C);
  Serial.print("Shortest Path: "); Serial.println(S);
  Serial.print("Actual Run Time Bonus (150-"); Serial.print(actualRunTime); Serial.print("): "); Serial.println(T);
  Serial.print("LED Bonus: "); Serial.println(L);
  Serial.print("=== TOTAL SCORE: "); Serial.print(totalScore); Serial.println(" ===");
}

void printDryRunStatus() {
  static unsigned long lastPrint = 0;
  if (millis() - lastPrint > 2000) {
    int timeLeft = (DRY_RUN_TIME - (millis() - dryRunStartTime)) / 1000;
    Serial.print("Dry Run - Time left: ");
    Serial.print(timeLeft);
    Serial.print("s | Path: ");
    Serial.println(exploredPath);
    lastPrint = millis();
  }
}

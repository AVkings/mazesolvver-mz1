// CONFIGURATION FILE - Edit these values for your setup

#ifndef CONFIG_H
#define CONFIG_H

// Hardware Configuration
#define NUM_SENSORS 7           // 5 or 7 sensors
#define LINE_THICKNESS 30       // mm (10-35)
#define IS_BLACK_LINE 0         // 1 = black line, 0 = white line
#define BRAKE_ENABLED 0         // 1 = enable braking

// PID Tuning (Adjust carefully!)
#define KP 0.15
#define KD 5.65
#define KI 0.20

// Speed Settings
#define LF_SPEED 55             // Base line following speed
#define ACCELERATION_RATE 15    // Ramp-up speed increment
#define TURN_SPEED 80           // Speed during turns
#define JUNCTION_CROSS_TIME 200 // Time to cross junction (ms)

// Motor Pin Definitions
#define AIN1 4
#define BIN1 6
#define AIN2 3
#define BIN2 7
#define PWMA 9
#define PWMB 10
#define STBY 5

// Sensor Thresholds
#define ON_LINE_THRESHOLD 700   // For black line detection
#define JUNCTION_THRESHOLD 500  // For junction detection

// Turn Timing (Calibrate for your robot!)
#define TURN_DELAY 150          // 90-degree turn time
#define U_TURN_DELAY 300        // 180-degree turn time

#endif
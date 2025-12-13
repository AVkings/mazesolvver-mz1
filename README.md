# 🤖 Maze-Solving Line-Following Robot – Custom PCB Project

A fully integrated line-following robot designed on a custom PCB, capable of navigating line mazes, scanning with the LSRB algorithm, and running the shortest path autonomously. Built for reliability and repeatability.



## 📁 File Structure

```
├── /Firmware
│   ├── sensor_test.ino          // Tests the 7-array IR sensor
│   ├── motor_test.ino           // Tests motor functionality
│   ├── line_follower_pid.ino    // Main PID line-following logic
│   ├── junction_t.ino           // T-junction navigation
│   ├── junction_plus.ino        // + junction navigation
│   └── README.md                // Firmware instructions & warnings
├── /PCB
│   ├── schematic.pdf            // Circuit schematic
│   ├── layout.png               // PCB layout preview
│   └── gerber.zip               // Production-ready Gerber files
├── /Docs
│   ├── components.txt           // Full BOM with exact part numbers
│   └── bot_working_flow.md      // Visual flowchart of robot logic
├── /Assets
│   └── demo_video_link.txt      // Link to demonstration video
├── LICENSE
└── README.md                    // You are here
```


## 🔧 Working of the Bot

### 1. **Line Following (PID)**
The robot uses a **7-array IR sensor** to detect the black line on a white surface. A **PID controller** adjusts motor speeds in real-time to keep the robot centered, ensuring smooth and stable motion.

### 2. **Junction Detection & Navigation**
- **T-Junction**: Decides turn based on the LSRB algorithm (Left/Right-hand rule).
- **+ Junction**: Similar logic, with priority for straights or turns depending on the maze phase.
- 
### 3. **Maze Scanning (LSRB – Coming Soon)**
During the first run, the robot applies the **LSRB algorithm** (Left-Straight-Right-Back) to explore the entire maze, recording each turn and junction.

### 4. **Shortest Path Calculation (Coming Soon)**
Once the finish line is reached, the recorded path is processed to **remove all dead-end loops**, leaving only the optimal route from start to finish.

### 5. **Optimized Sprint Run**
The robot is placed back at the start and runs the **shortest path** at high speed without further decision-making.


## ⚠️ Important Note for First-Time Users
Many of the provided firmware files combine **PID logic with specific navigation algorithms** (e.g., `pid_t_junction.ino`). This integration can be confusing if you're unfamiliar with PID or maze-solving logic.


**Recommendation:**  
Start with `sensor_test.ino` and `motor_test.ino` to verify your hardware before moving to the integrated PID files. Read the comments in each code file carefully—they explain what each section does and which variables you may need to adjust.


## 🧩 Components
All components are listed in `/Docs/components.txt` with exact specifications and recommended suppliers. The custom PCB integrates the microcontroller, motor driver, sensor ports, and power regulation—no external wiring needed if using the provided PCB design.


## 🚀 Getting Started
1. **Review** `components.txt` to ensure you have all parts.
2. **Manufacture** the PCB using the Gerber files in `/PCB/`.
3. **Upload** test sketches (`sensor_test.ino`, `motor_test.ino`) to confirm operation.
4. **Calibrate** sensor thresholds and PID constants for your surface.
5. **Test** junction handling on sample tracks before full maze runs.


## 📺 See It in Action
A demonstration video is linked in `/Assets/demo_video_link.txt`.


## 📄 License
This project is released under the **MIT License**. You are free to use, modify, and distribute it, but attribution is appreciated


**Built with curiosity and many iterations.**  
If you build this, share your results—I’d love to see your version! 🛠️✨

The current actual code which I have is given already as Codex.ino you can check it out!

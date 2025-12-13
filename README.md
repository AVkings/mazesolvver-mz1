# Maze solver bot - using PID, LSRB and dead elemination

This is a project where we have to make a bot which could solve mazes made out of lines and loops using PID and other algorithms
this competition reprents core concets of team work and algorithm impletation in code

if you want ot make same as me you just have to follow this repo

starting; this is the file structure u are provided with:

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


## Now lets talk about bot!
let me ask you a question: _what do you think how a robot would keep up itself on line?_
well the ans is PID
### PID; what the hell is it?
- well for bigginers i recommend watching some videos related to this concept
- pid is Propotional Integral Derivative
- this is the driver of your bot which helps your bot to be steady on line
- well pid is used in various devices for various purposes but here we r using it to inteprete the data from sensors to code
- hence we have used 7 array sensor it may vary on ur use

### _*now that you got to know how bot gets on line lets talk about algorithms*_
### 1. **Junction, Starting point, End point**
- well as we are talking about maze there are various types of maze and each and everymaze contains a intersection where two or more lines meet
- here we are specially going to learn about T junction and + junction
  #### T junction
  - this junction is also called a a node junction because a small dead end line is joint to a strait line liek a node of tree
  - this junction is called node junction when we make T rotate to left/ right
  - in normal terms to overcome T junction bot goes at front if no line detected  it would turn left and right (u will se why in next phrase)

  #### + junction
  - this junction is formed when two lines cross each other
  - the code is almost similar to T junction but you have to do some shanges so the bot dosent stop on loops

  #### lets talk about starting and end points
  - starting point is the point form where u calibarte ur both and amke a run
  - end point is a big patch of line color on which ur bot must stop and blink an led

### 2. **Scanning the maze _how would the bot react to each node/junction?_**
- well in competition there are two runs:
  #### 1. dry run
     - in this u get total of 3 min to scan the maze but how?
     - well here comes LSRB this is term or set of rules which decides how your bot would react on each junction
     - LSRB is the left hand algorithm which is used to solve maze
     - it stands for *LEFT STRAIGHT RIGHT BACK*
     - means the bot would give priority to left then straight and so on
       
  #### 2. Actual run
  - so this is the main run
  - so as your bot has scanned the maze now your bot has make to the finish line form satrting as fast as possible
  - for finding the shortest we have an another algo explained in next phase:

### 3. **SHortest path algo**
- While the bot is scanning the maze it is recording the maze turns/path side by side
- the recorded path is now being used to find shortest path
- well i cannot explain about this algo much as i havent developed it yet but still i can give u a sneak peak
- lets suppose the bot has recorded path SLBL so now the term B means back so whatever the terms imidiately before and after it are there are replaced by the term before      them like S so now the path becomes SS
- and at the end it just runs the recorded path hence achieving the shortest path



## What should you do now?
###### well if you want to create this project i recommend you going to through each and every code given in repo and also readme files 

### Getting started
*as i have introduced you with my basic idea that how this bot works now let me tell you how could you follow this repo*
### 1. getting components 
- i have provide dyou all the list of components needed in bill.png

### 2. understanding algorithm and core concepts 
- my most obvious advice for you to make this project is to watch tutorial about algoriths and PID
- if you couldnt find any you just contact me i would explain you or either i would give you best tutoring links

### 3. Going through Firmware
- you might take this part of repo lightly as it just contains small portion of main code
- but that code is evrything you need to make this project
- if you are able to understand those you can create one too

### 4. Reviewing Codex.ino
- the codex is not the most relevant code i would recommend
- i would not recommend that cod eyet eventhough we won through it but the reality is that that was the base code and the code which i used got deleted
- eventhough that code has almost everything i mentioned in this readme file but dont run that code instead use as a reference code


## Actual break through
AS of my competition is near i have to make code anyhow hence the remaining codes/resources would be provided soon so stay tune
btw ignore all spelling mistakes as my keyboard is cooked

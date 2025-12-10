# ROBOT
A C-based operating system simulation for a robot that prioritizes urgent tasks over routine missions
The Concept:
This program acts as a robot's brain, using priority scheduling to decide what to do next. It manages
two separate command queues:
1. Normal Queue: For routine tasks (e.g., "Move Forward").
2. Urgent Queue: For emergencies (e.g., "STOP! Obstacle detected").
The robot always checks the Urgent Queue first. If danger is detected, it pauses the normal mission to
handle the emergency immediately
Technical Features
Linked Lists: Uses pointers to create dynamic queues (Task 1 & 2), avoiding fixed-size arrays.
Structures: Defines complex command objects with ID, type, duration, and priority.
Dynamic Memory: Efficiently allocates ( malloc ) and frees memory as needed.
File Logging: Records a permanent history of actions in journal_robot.txt .
Simulation: Randomly triggers sensor events (obstacles, low battery) to test the priority logic.
How to Run
1. Compile:
gcc robot_project_v2.c -o robot
2. Run:
./robot
Summary
This project demonstrates the use of Structures, Pointers, and Dynamic Memory Management to
create a responsive and stable control system.

# Robot Control System - Graphical Simulation
## Installation & Compilation Guide (Linux/Ubuntu)

## Prerequisites

### 1. Install raylib library

#### Option A: Using apt (Ubuntu/Debian)
```bash
sudo apt update
sudo apt install libraylib-dev
```

#### Option B: Build from source (if apt version is outdated)
```bash
# Install dependencies
sudo apt install libasound2-dev mesa-common-dev libx11-dev libxrandr-dev libxi-dev xorg-dev libgl1-mesa-dev libglu1-mesa-dev

# Clone raylib
git clone https://github.com/raysan5/raylib.git
cd raylib/src

# Compile and install
make PLATFORM=PLATFORM_DESKTOP
sudo make install

# Update library cache
sudo ldconfig
```

### 2. Install build tools (if not already installed)
```bash
sudo apt install build-essential gcc make
```

## Compilation

### Method 1: Using Makefile (Recommended)
```bash
# Compile
make

# Run
make run

# Clean build files
make clean
```

### Method 2: Manual compilation
```bash
# Basic compilation
gcc -Wall -Wextra -std=c99 robot_simulation.c -o robot_simulation -lraylib -lm -lpthread -ldl -lrt -lX11

# Run
./robot_simulation
```
## Running the Simulation

```bash
./robot_simulation
```

## Features

### Visual Elements:
- **Grid Background**: Navigation reference
- **Robot**: Triangle shape pointing in movement direction
- **Direction Indicator**: Yellow line showing robot heading
- **Obstacles**: Red circles with "!" warning when detected
- **UI Panel**: Shows battery, position, angle, queue status, current command

### Command Types:
1. **AVANCER** (Forward) - Green
2. **RECULER** (Backward) - Orange  
3. **GAUCHE** (Turn Left) - Purple
4. **DROITE** (Turn Right) - Purple
5. **ARRET** (Emergency Stop) - Red
6. **RECHARGE** (Battery Recharge) - Yellow with pulsing circles

### Priority System:
- **Urgent Queue** (Red): Handles obstacles and low battery
- **Normal Queue** (Blue): Routine mission commands
- Urgent commands ALWAYS execute before normal ones

### Automatic Systems:
- **Obstacle Detection**: Random obstacles spawn, trigger emergency stop + reverse
- **Battery Management**: Auto-recharge when < 20%
- **Logging**: All actions written to `journal_robot.txt`
- **Queue Replenishment**: Adds random commands when queue is low

## Controls
- **ESC**: Exit simulation

## Output Files
- `journal_robot.txt`: Complete log of all robot actions with timestamps

## Troubleshooting

### Error: "raylib.h: No such file or directory"
```bash
# Install raylib development headers
sudo apt install libraylib-dev
```

### Error: "cannot find -lraylib"
```bash
# Ensure raylib is installed and library path is updated
sudo ldconfig
```

### Error: "undefined reference to dlopen/pthread_create"
```bash
# Add all required linking flags
gcc robot_simulation.c -o robot_simulation -lraylib -lm -lpthread -ldl -lrt -lX11
```

### Black screen or no window
- Check if your system supports OpenGL 3.3+
- Update graphics drivers
- Try running with `LIBGL_ALWAYS_SOFTWARE=1 ./robot_simulation`

## Technical Details

### Architecture:
- **Queue System**: Original linked-list implementation maintained
- **Priority Scheduling**: Urgent commands preempt normal execution
- **Real-time Rendering**: 60 FPS with delta-time based movement
- **Sensor Simulation**: Random events trigger priority responses

### Performance:
- Target FPS: 60
- Resolution: 1200x800
- Memory: Dynamic allocation for queue elements

## Code Structure:
```
robot_simulation.c
├── Data Structures (Commande, File, Robot, Obstacle)
├── Queue Functions (identical to original)
├── File Logging (unchanged)
├── Drawing Functions (raylib-based)
├── Robot Update Logic (physics + animation)
├── Sensor System (with visual feedback)
└── Main Loop (game loop + command execution)
```

## Example Session Output:
```
--- CHARGEMENT DE LA MISSION ---
--- DEMARRAGE DU ROBOT ---
[NORMAL] Execution ID 1
[NORMAL] Execution ID 2

!!! OBSTACLE DETECTE !!!

[ALERT] Commande URGENTE ajoutee a la file d'urgence !
[ALERT] Commande URGENTE ajoutee a la file d'urgente !
[URGENT] Execution ID 999
[URGENT] Execution ID 998
[NORMAL] Execution ID 3
...
```

## Notes:
- Robot wraps around screen edges (teleports)
- Battery drains by 15% per non-recharge command
- Obstacle appearance is randomized
- Commands auto-generate to maintain continuous demo
# Snake-Game
Snake game in OpenGL. Move the snake with arrow keys, collect food, avoid obstacles. Grow longer and don't hit yourself or walls!

Features:
Move the snake using arrow keys.
Collect food to grow the snake.
Avoid obstacles.
Game ends when the snake collides with itself, an obstacle, or goes out of bounds.
Getting Started:

Prerequisites:
C++ compiler with OpenGL support (e.g., GLFW)

Building:
(Assuming a specific build system like cmake is used, modify accordingly)
Bash
mkdir build
cd build
cmake ..
make


Running:
Bash
./snake_game
Use code with caution.

Gameplay:
Use the arrow keys (up, down, left, right) to control the snake's direction.
The snake will grow longer when it eats food (represented by a square).
Avoid obstacles (represented by squares) or the snake will die.
The game ends when the snake dies or reaches a certain length (modify the code to change this).

Code Structure:
The code is likely divided into multiple files:
main.cpp: Contains the game loop and initialization.
snake.cpp: Manages the snake's logic (movement, collision detection).
food.cpp: Handles food placement and logic.
obstacle.cpp (if applicable): Manages obstacle placement and logic.
graphics.cpp (or similar): Handles rendering the game elements (snake, food, obstacles).

Note:
This is a basic implementation and can be extended with features like scorekeeping, difficulty levels, power-ups, etc.

Further Information:
ridazubair99@gmail.com

Author : Rida Zubair

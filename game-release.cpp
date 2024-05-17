
//by rida zubair 
#ifndef SNAKE_CPP
#define SNAKE_CPP

#include <algorithm>
#include <cmath>
#include <ctime>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "util.h"
using namespace std;

void SetCanvasSize(int width, int height) {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, width, 0, height, -1, 1);  // set the screen size to given width and height.
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

const int canvasWidth = 650, canvasHeight = 700;
const int tileSize = 10;

bool isGameOver = false;

int bgColor = POWDER_BLUE;

// Snake Variables
const int maxSnakeLength = 1000;
int snakePos[maxSnakeLength][2];
int snakeLength = 4;
int oldSnakeTail[2];

int snakeSpeed = 200;//inc this for snake speed
int snakeSpeedTimer = 9;
double snakeSpeedTimerLimit = 0.5;//discrease this for speed increase 

int snakeDirectionX = 0;
int snakeDirectionY = -1;
int tempSnakeDirectionX = 0;
int tempSnakeDirectionY = -1;

// Score
int score = snakeLength - 4;
int highScore;

// Game History
const int maxGameHistory = 10;
int gameHistory[maxGameHistory];

// Hurdles
int maxHurdles = 3;
int hurdleLengthMax = 10;
int hurdlePos[15][4];
int hurdleTimer = 0;
int hurdleTimerLimit = 30 * FPS;

// Food Variables
const int maxFood = 5;
int foodPos[maxFood][3];
const int foodDuration = 15 * FPS;

// Power Food
// (note: power Food is always a 3x3 square and powerFoodPos is the middle of the square)
int powerFoodPos[3] = {0, 0, 0};
const int powerFoodDuration = 15 * FPS;
bool isPowerFoodVisible = true;
const int powerFoodRespawnDuration = 60 * FPS;
int powerFoodRespawnDurationCount = 0;

const int matrixWidth = canvasWidth / tileSize;
const int matrixHeight = canvasHeight / tileSize;
const int startY = 5;

int matrix[matrixWidth][matrixHeight];

// Menu
enum GameState {
    MAIN_MENU,
    START_GAME,
    RESUME_GAME,
    CHANGE_LEVEL_MENU,
    HIGH_SCORE,
    GAME_HISTORY,
    EXIT
};

GameState gameState = MAIN_MENU;

const int menuButtonsCount = 6;
string menuButtons[menuButtonsCount] = {"1. Start Game", "2. Resume Game", "3. Change Level", "4. High Score", "5. Game History", "6. Exit"};

const int menuButtonHeight = 30;
const int menuButtonWidth = 180;
const int menuButtonMargin = 10;
const int menuButtonStartX = canvasWidth / 2 - menuButtonWidth / 2;
const int menuButtonStartY = canvasHeight * 0.6;

void drawMenu() {
    for (int i = 0; i < menuButtonsCount; i++) {
        DrawString(menuButtonStartX, menuButtonStartY - i * (menuButtonHeight + menuButtonMargin), menuButtons[i].c_str(), colors[ROYAL_BLUE]);
    }
}

void loadHighScore() {
    ifstream highScoreFile;
    highScoreFile.open("highScore.txt");

    if (highScoreFile.is_open()) {
        highScoreFile >> highScore;
    } else {
        highScore = 0;
    }

    highScoreFile.close();
}

void loadGameHistory() {
    ifstream gameHistoryFile;
    gameHistoryFile.open("gameHistory.txt");

    if (gameHistoryFile.is_open()) {
        for (int i = 0; i < maxGameHistory; i++) {
            gameHistoryFile >> gameHistory[i];
        }
    } else {
        for (int i = 0; i < maxGameHistory; i++) {
            gameHistory[i] = 0;
        }
    }

    gameHistoryFile.close();
}

void saveGameHistory() {
    ofstream gameHistoryFile;
    gameHistoryFile.open("gameHistory.txt");

    if (gameHistoryFile.is_open()) {
        for (int i = 0; i < maxGameHistory; i++) {
            gameHistoryFile << gameHistory[i] << endl;
        }
    }

    gameHistoryFile.close();
}

void initMatrix() {
    for (int i = 0; i < matrixWidth; i++) {
        for (int j = 0; j < matrixHeight; j++) {
            matrix[i][j] = 0;
        }
    }
}

void initSnake() {
    snakePos[0][0] = 5;
    snakePos[0][1] = startY;

    for (int i = 1; i < snakeLength; i++) {
        snakePos[i][0] = snakePos[i - 1][0] - 1;
        snakePos[i][1] = snakePos[i - 1][1];
    }
}

void initFood(int i) {
    while (true) {
        foodPos[i][0] = rand() % (matrixWidth - 1);
        foodPos[i][1] = rand() % (matrixHeight - startY - 1) + startY;
        foodPos[i][2] = foodDuration;

        bool isFoodValid = true;

        for (int j = 0; j < maxFood; j++) {
            if (j == i || foodPos[j][2] == 0) {
                continue;
            }

            if (foodPos[i][0] == foodPos[j][0] || foodPos[i][1] == foodPos[j][1]) {
                isFoodValid = false;
                break;
            }

            if (foodPos[i][0] == foodPos[i][1] && foodPos[j][0] == foodPos[j][1]) {
                isFoodValid = false;
                break;
            }

            if (foodPos[i][0] == (matrixWidth - foodPos[i][1] - 1) && foodPos[j][0] == (matrixWidth - foodPos[j][1] - 1)) {
                isFoodValid = false;
                break;
            }

            for (int k = 0; k < snakeLength; k++) {
                if (foodPos[i][0] == snakePos[k][0] && foodPos[i][1] == snakePos[k][1]) {
                    isFoodValid = false;
                    break;
                }
            }

            if (isPowerFoodVisible) {
                if (foodPos[i][0] >= powerFoodPos[0] - 1 && foodPos[i][0] < powerFoodPos[0] + 2 &&
                    foodPos[i][1] >= powerFoodPos[1] - 1 && foodPos[i][1] < powerFoodPos[1] + 2) {
                    isFoodValid = false;
                    break;
                }
            }

            if (isFoodValid) {
                // Check collision with hurdles
                for (int k = 0; k < maxHurdles; k++) {
                    if (hurdlePos[k][3] == 0) {
                        if (hurdlePos[k][0] + hurdlePos[k][2] > foodPos[i][0] && hurdlePos[k][0] <= foodPos[i][0] &&
                            hurdlePos[k][1] == foodPos[i][1]) {
                            isFoodValid = false;
                            break;
                        }
                    } else {
                        if (hurdlePos[k][1] + hurdlePos[k][2] > foodPos[i][1] && hurdlePos[k][1] <= foodPos[i][1] &&
                            hurdlePos[k][0] == foodPos[i][0]) {
                            isFoodValid = false;
                            break;
                        }
                    }
                }
            }
        }

        if (isFoodValid) {
            break;
        }
    }
}

void initHurdles() {
    for (int i = 0; i < maxHurdles; i++) {
        while (true) {
            hurdlePos[i][0] = rand() % (matrixWidth - 1);
            hurdlePos[i][1] = rand() % (matrixHeight - startY - 1) + startY;
            hurdlePos[i][2] = rand() % hurdleLengthMax + 1;  // Length of the hurdle
            hurdlePos[i][3] = rand() % 2;                    // Orientation (0 for horizontal, 1 for vertical)

            bool isHurdleValid = true;

            if (hurdlePos[i][3] == 0) {
                if (hurdlePos[i][0] + hurdlePos[i][2] >= matrixWidth) {
                    isHurdleValid = false;
                }
            } else {
                if (hurdlePos[i][1] + hurdlePos[i][2] >= matrixHeight) {
                    isHurdleValid = false;
                }
            }

            // Check collision with snake
            if (isHurdleValid) {
                for (int j = 0; j < snakeLength; j++) {
                    if (hurdlePos[i][3] == 0) {
                        if (hurdlePos[i][0] + hurdlePos[i][2] > snakePos[j][0] && hurdlePos[i][0] <= snakePos[j][0] &&
                            hurdlePos[i][1] == snakePos[j][1]) {
                            isHurdleValid = false;
                            break;
                        }
                    } else {
                        if (hurdlePos[i][1] + hurdlePos[i][2] > snakePos[j][1] && hurdlePos[i][1] <= snakePos[j][1] &&
                            hurdlePos[i][0] == snakePos[j][0]) {
                            isHurdleValid = false;
                            break;
                        }
                    }
                }
            }

            // Check collision with food
            if (isHurdleValid) {
                for (int j = 0; j < maxFood; j++) {
                    if (hurdlePos[i][3] == 0) {
                        if (hurdlePos[i][0] + hurdlePos[i][2] > foodPos[j][0] && hurdlePos[i][0] <= foodPos[j][0] &&
                            hurdlePos[i][1] == foodPos[j][1]) {
                            isHurdleValid = false;
                            break;
                        }
                    } else {
                        if (hurdlePos[i][1] + hurdlePos[i][2] > foodPos[j][1] && hurdlePos[i][1] <= foodPos[j][1] &&
                            hurdlePos[i][0] == foodPos[j][0]) {
                            isHurdleValid = false;
                            break;
                        }
                    }
                }
            }

            // Check collision with power food (note: power Food is always a 3x3 square and powerFoodPos is the middle of the square)
            if (isHurdleValid) {
                if (hurdlePos[i][3] == 0) {
                    if (hurdlePos[i][0] + hurdlePos[i][2] > powerFoodPos[0] - 1 && hurdlePos[i][0] < powerFoodPos[0] + 2 &&
                        hurdlePos[i][1] == powerFoodPos[1]) {
                        isHurdleValid = false;
                    }
                } else {
                    if (hurdlePos[i][1] + hurdlePos[i][2] > powerFoodPos[1] - 1 && hurdlePos[i][1] < powerFoodPos[1] + 2 &&
                        hurdlePos[i][0] == powerFoodPos[0]) {
                        isHurdleValid = false;
                    }
                }
            }

            if (isHurdleValid) {
                break;
            }
        }
    }
}

bool checkCollisionWithPowerFood(int x, int y) {
    for (int i = powerFoodPos[0] - 1; i < powerFoodPos[0] + 2; i++) {
        for (int j = powerFoodPos[1] - 1; j < powerFoodPos[1] + 2; j++) {
            if (x == i && y == j) {
                return true;
            }
        }
    }

    return false;
}

void removePowerFoodFromMatrix() {
    for (int i = powerFoodPos[0] - 1; i < powerFoodPos[0] + 2; i++) {
        for (int j = powerFoodPos[1] - 1; j < powerFoodPos[1] + 2; j++) {
            matrix[i][j] = 0;
        }
    }
}

void togglePowerFoodVisibility() {
    isPowerFoodVisible = !isPowerFoodVisible;

    if (!isPowerFoodVisible) {
        removePowerFoodFromMatrix();
    }
}

void initPowerFood() {
    while (true) {
        powerFoodPos[0] = 1 + rand() % (matrixWidth - 3);
        powerFoodPos[1] = 1 + rand() % (matrixHeight - startY - 3) + startY;
        powerFoodPos[2] = powerFoodDuration;
        powerFoodRespawnDurationCount = powerFoodRespawnDuration;

        // Check collision with hurdles, snake, and normal food
        bool isPowerFoodValid = true;

        for (int i = 0; i < maxHurdles; i++) {
            if (hurdlePos[i][3] == 0) {
                if (hurdlePos[i][0] + hurdlePos[i][2] > powerFoodPos[0] - 1 && hurdlePos[i][0] < powerFoodPos[0] + 2 &&
                    hurdlePos[i][1] == powerFoodPos[1]) {
                    isPowerFoodValid = false;
                }
            } else {
                if (hurdlePos[i][1] + hurdlePos[i][2] > powerFoodPos[1] - 1 && hurdlePos[i][1] < powerFoodPos[1] + 2 &&
                    hurdlePos[i][0] == powerFoodPos[0]) {
                    isPowerFoodValid = false;
                }
            }
        }

        if (isPowerFoodValid) {
            for (int i = 0; i < maxFood; i++) {
                if (checkCollisionWithPowerFood(foodPos[i][0], foodPos[i][1])) {
                    isPowerFoodValid = false;
                    break;
                }
            }
        }

        if (isPowerFoodValid) {
            for (int i = 0; i < snakeLength; i++) {
                if (checkCollisionWithPowerFood(snakePos[i][0], snakePos[i][1])) {
                    isPowerFoodValid = false;
                    break;
                }
            }
        }

        if (isPowerFoodValid) {
            break;
        }
    }
}

void initFoods() {
    for (int i = 0; i < maxFood; i++) {
        foodPos[i][0] = 0;
        foodPos[i][1] = 0;
        foodPos[i][2] = 0;
    }

    for (int i = 0; i < maxFood; i++) {
        initFood(i);
    }
}

void updateHurdles() {
    hurdleTimer++;

    if (hurdleTimer > hurdleTimerLimit) {
        // Hurdles disappear after 30 seconds
        for (int i = 0; i < maxHurdles; i++) {
            if (hurdlePos[i][3] == 0) {
                for (int j = 0; j < hurdlePos[i][2]; j++) {
                    matrix[hurdlePos[i][0] + j][hurdlePos[i][1]] = 0;
                }
            } else {
                for (int j = 0; j < hurdlePos[i][2]; j++) {
                    matrix[hurdlePos[i][0]][hurdlePos[i][1] + j] = 0;
                }
            }
        }

        initHurdles();
        hurdleTimer = 0;
    }
}

void updateDirection() {
    snakeDirectionX = tempSnakeDirectionX;
    snakeDirectionY = tempSnakeDirectionY;
}

void updateSnakePos() {
    snakeSpeedTimer++;

    if (snakeSpeedTimer <= snakeSpeedTimerLimit) {
        return;
    }

    updateDirection();

    snakeSpeedTimer = 0;

    oldSnakeTail[0] = snakePos[snakeLength - 1][0];
    oldSnakeTail[1] = snakePos[snakeLength - 1][1];

    for (int i = snakeLength - 1; i > 0; i--) {
        snakePos[i][0] = snakePos[i - 1][0];
        snakePos[i][1] = snakePos[i - 1][1];
    }

    snakePos[0][0] += snakeDirectionX;
    snakePos[0][1] += snakeDirectionY;

    // check if snake is eating food
    for (int i = 0; i < maxFood; i++) {
        if (snakePos[0][0] == foodPos[i][0] && snakePos[0][1] == foodPos[i][1]) {
            snakeLength++;
            score += 5;

            initFood(i);

            snakePos[snakeLength - 1][0] = oldSnakeTail[0];
            snakePos[snakeLength - 1][1] = oldSnakeTail[1];

            break;
        }
    }

    // check if snake is eating power food
    if (isPowerFoodVisible) {
        if (snakePos[0][0] >= powerFoodPos[0] - 1 && snakePos[0][0] < powerFoodPos[0] + 2 &&
            snakePos[0][1] >= powerFoodPos[1] - 1 && snakePos[0][1] < powerFoodPos[1] + 2) {
            togglePowerFoodVisibility();

            snakeLength++;
            score += 20;

            snakePos[snakeLength - 1][0] = oldSnakeTail[0];
            snakePos[snakeLength - 1][1] = oldSnakeTail[1];
        }
    }

    // check if snake crossed the boundary
    if (snakePos[0][0] >= matrixWidth) {
        snakePos[0][0] = 0;
    } else if (snakePos[0][0] < 0) {
        snakePos[0][0] = matrixWidth - 1;
    }

    if (snakePos[0][1] >= matrixHeight) {
        snakePos[0][1] = startY;
    } else if (snakePos[0][1] < startY) {
        snakePos[0][1] = matrixHeight - 1;
    }

    // check if snake is eating itself
    for (int i = 1; i < snakeLength; i++) {
        if (snakePos[0][0] == snakePos[i][0] && snakePos[0][1] == snakePos[i][1]) {
            isGameOver = true;
            break;
        }
    }

    // check if snake is colliding with hurdles
    for (int i = 0; i < maxHurdles; i++) {
        if (hurdlePos[i][3] == 0) {
            if (hurdlePos[i][0] + hurdlePos[i][2] > snakePos[0][0] && hurdlePos[i][0] <= snakePos[0][0] &&
                hurdlePos[i][1] == snakePos[0][1]) {
                isGameOver = true;
                break;
            }
        } else {
            if (hurdlePos[i][1] + hurdlePos[i][2] > snakePos[0][1] && hurdlePos[i][1] <= snakePos[0][1] &&
                hurdlePos[i][0] == snakePos[0][0]) {
                isGameOver = true;
                break;
            }
        }
    }

    if (isGameOver) {
        for (int i = maxGameHistory - 1; i > 0; i--) {
            gameHistory[i] = gameHistory[i - 1];
        }

        gameHistory[0] = score;

        saveGameHistory();
    }
}

void updatePowerFoodOnMatrix() {
    if (isPowerFoodVisible) {
        for (int i = powerFoodPos[0] - 1; i < powerFoodPos[0] + 2; i++) {
            for (int j = powerFoodPos[1] - 1; j < powerFoodPos[1] + 2; j++) {
                matrix[i][j] = 7;
            }
        }
    }
}

void updateMatrix() {
    // update hurdles
    for (int i = 0; i < maxHurdles; i++) {
        if (hurdlePos[i][3] == 0) {
            for (int j = 0; j < hurdlePos[i][2]; j++) {
                matrix[hurdlePos[i][0] + j][hurdlePos[i][1]] = 8;  // Use a new value (8) for hurdles
            }
        } else {
            for (int j = 0; j < hurdlePos[i][2]; j++) {
                matrix[hurdlePos[i][0]][hurdlePos[i][1] + j] = 8;
            }
        }
    }

    // update food
    for (int i = 0; i < maxFood; i++) {
        matrix[foodPos[i][0]][foodPos[i][1]] = 5;
    }

    // update power food
    updatePowerFoodOnMatrix();

    // update snake
    matrix[snakePos[1][0]][snakePos[1][1]] = 9;
    matrix[oldSnakeTail[0]][oldSnakeTail[1]] = 0;

    matrix[snakePos[0][0]][snakePos[0][1]] = 10;

    for (int i = 1; i < snakeLength; i++) {
        matrix[snakePos[i][0]][snakePos[i][1]] = 9;
    }
}

void updateFood() {
    for (int i = 0; i < maxFood; i++) {
        foodPos[i][2]--;

        if (foodPos[i][2] == 0) {
            matrix[foodPos[i][0]][foodPos[i][1]] = 0;

            initFood(i);
        }
    }
}

void updatePowerFood() {
    if (isPowerFoodVisible) {
        powerFoodPos[2]--;

        if (powerFoodPos[2] == 0) {
            togglePowerFoodVisibility();
        }
    }

    powerFoodRespawnDurationCount--;

    if (powerFoodRespawnDurationCount == 0) {
        togglePowerFoodVisibility();
        initPowerFood();
    }
}

void updateAndSaveHighScore() {
    if (score > highScore) {
        highScore = score;
    }

    ofstream highScoreFile;
    highScoreFile.open("highScore.txt");

    if (highScoreFile.is_open()) {
        highScoreFile << highScore;
    }

    highScoreFile.close();
}

void update() {
    updateSnakePos();
    updateFood();
    updatePowerFood();
    updateHurdles();
    updateMatrix();
    updateAndSaveHighScore();
}

void init() {
    loadHighScore();
    loadGameHistory();
    initMatrix();
    initSnake();
    initFoods();
    initPowerFood();
    initHurdles();
}

void drawMatrix() {
    for (int i = 0; i < matrixWidth; i++) {
        for (int j = 0; j < matrixHeight; j++) {
            // Draw grid pattern
            // if ((i + j) % 2 == 0) {
            //     DrawSquare(i * tileSize, canvasHeight - j * tileSize - tileSize, tileSize, colors[LIGHT_BLUE]);
            // } else {
            //     DrawSquare(i * tileSize, canvasHeight - j * tileSize - tileSize, tileSize, colors[WHITE_SMOKE]);
            // }
            if (matrix[i][j] == 9) {
                DrawSquare(i * tileSize, canvasHeight - j * tileSize - tileSize, tileSize, colors[YELLOW]);
            }

            if (matrix[i][j] == 10) {
                DrawCircle(i * tileSize + tileSize / 2, canvasHeight - j * tileSize + tileSize / 2 - tileSize, tileSize * 0.7, colors[RED]);
            }

            if (matrix[i][j] == 5) {
                DrawCircle(i * tileSize + tileSize / 2, canvasHeight - j * tileSize + tileSize / 2 - tileSize, tileSize * 0.35, colors[HOT_PINK]);
            }

            if (i == powerFoodPos[0] && j == powerFoodPos[1] && isPowerFoodVisible) {
                DrawCircle(i * tileSize + tileSize / 2, canvasHeight - j * tileSize + tileSize / 2 - tileSize, tileSize * 1.5, colors[ORANGE]);
            }

            if (matrix[i][j] == 8) {
                DrawSquare(i * tileSize, canvasHeight - j * tileSize - tileSize, tileSize, colors[CHOCOLATE]);
            }
        }
    }
}

void drawScore() {
    stringstream ss;
    ss << "Score: " << score;
    string scoreString = ss.str();
    DrawString(canvasWidth - 100, canvasHeight - 20, scoreString.c_str(), colors[WHITE]);
}

void drawSnakeBar() {
    const int maxBarScore = 100;

    stringstream ss;
    ss << "SNAKE: ";
    string scoreString = ss.str();
    DrawString(2, canvasHeight - tileSize / 0.5, "SNAKE:", colors[WHITE]);
    DrawLine(100, canvasHeight - tileSize, 100 + (tileSize * 30), canvasHeight - tileSize, tileSize, colors[WHITE]);

    if (score <= maxBarScore) {
        DrawLine(100, canvasHeight - tileSize, 100 + (tileSize * 30 * score / maxBarScore), canvasHeight - tileSize, tileSize, colors[GREEN]);
    } else {
        DrawLine(100, canvasHeight - tileSize, 100 + (tileSize * 30), canvasHeight - tileSize, tileSize, colors[GREEN]);
    }
}

void displayMainGame() {
    drawMatrix();
    DrawLine(0, canvasHeight - startY * tileSize, canvasWidth, canvasHeight - startY * tileSize, 5, colors[WHITE]);

    if (isGameOver) {
        DrawString(canvasWidth / 2 - 50, canvasHeight / 2, "Game Over", colors[RED]);
    }

    drawScore();
    drawSnakeBar();
}

void resetGame() {
    isGameOver = false;
    snakeLength = 4;
    snakeDirectionX = 0;
    snakeDirectionY = -1;
    tempSnakeDirectionX = 0;
    tempSnakeDirectionY = -1;
    score = snakeLength - 4;
    isPowerFoodVisible = true;
    hurdleTimer = 0;

    init();
}

void Display() {
    // set the background color using function glClearColotgr.
    // to change the background play with the red, green and blue values below.
    // Note that r, g and b values must be in the range [0,1] where 0 means dim rid and 1 means pure red and so on.

    glClearColor(colors[bgColor][0], colors[bgColor][1],
                 colors[bgColor][2], 0);
    glClear(GL_COLOR_BUFFER_BIT);

    switch (gameState) {
        case MAIN_MENU:
            drawMenu();
            break;

        case START_GAME:
            displayMainGame();
            break;

        case RESUME_GAME:
            displayMainGame();
            break;

        case CHANGE_LEVEL_MENU:
            DrawString(250, 400, "Select Level:", colors[WHITE]);
            DrawString(250, 370, "1. Level 1", colors[WHITE]);
            DrawString(250, 340, "2. Level 2", colors[WHITE]);
            DrawString(250, 310, "3. Level 3", colors[WHITE]);
            DrawString(250, 280, "4. Level 4", colors[WHITE]);
            DrawString(250, 250, "5. Level 5", colors[WHITE]);
            DrawString(250, 220, "6. Level 6", colors[WHITE]);
            DrawString(250, 190, "7. Level 7", colors[WHITE]);
            break;

        case HIGH_SCORE: {
            DrawString(canvasWidth * 0.5 - 50, canvasHeight - 100, "High Score", colors[WHITE]);
            stringstream highScoreText;
            highScoreText << "Your High Score is " << highScore;
            DrawString(canvasWidth * 0.33, canvasHeight / 2 - 50, highScoreText.str().c_str(), colors[WHITE]);
            break;
        }

        case GAME_HISTORY: {
            DrawString(canvasWidth * 0.5 - 50, canvasHeight - 100, "Game History", colors[WHITE]);

            for (int i = 0; i < maxGameHistory; i++) {
                stringstream gameHistoryText;
                gameHistoryText << i + 1 << ". " << gameHistory[i];
                DrawString(canvasWidth * 0.45, canvasHeight - 150 - i * 30, gameHistoryText.str().c_str(), colors[WHITE]);
            }

            break;
        }
    }

    glutSwapBuffers();  // do not modify this line..
}

/*This function is called (automatically) whenever any non-printable key (such as up-arrow, down-arraw)
 * is pressed from the keyboard
 *
 * You will have to add the necessary code here when the arrow keys are pressed or any other key is pressed...
 *
 * This function has three argument variable key contains the ASCII of the key pressed, while x and y tells the
 * program coordinates of mouse pointer when key was pressed.
 *
 * */

void NonPrintableKeys(int key, int x, int y) {
    if ((gameState == START_GAME || gameState == RESUME_GAME) && !isGameOver) {
        if (key == GLUT_KEY_LEFT) {
            if (snakeDirectionX == 1 || tempSnakeDirectionX != snakeDirectionX) {
                return;
            }

            tempSnakeDirectionX = -1;
            tempSnakeDirectionY = 0;
        } else if (key == GLUT_KEY_RIGHT) {
            if (snakeDirectionX == -1 || tempSnakeDirectionX != snakeDirectionX) {
                return;
            }

            tempSnakeDirectionX = 1;
            tempSnakeDirectionY = 0;
        } else if (key == GLUT_KEY_UP) {
            if (snakeDirectionY == 1 || tempSnakeDirectionY != snakeDirectionY) {
                return;
            }

            tempSnakeDirectionX = 0;
            tempSnakeDirectionY = -1;
        } else if (key == GLUT_KEY_DOWN) {
            if (snakeDirectionY == -1 || tempSnakeDirectionY != snakeDirectionY) {
                return;
            }

            tempSnakeDirectionX = 0;
            tempSnakeDirectionY = 1;
        }
    }
    glutPostRedisplay();
}

/*This function is called (automatically) whenever any printable key (such as x,b, enter, etc.)
 * is pressed from the keyboard
 * This function has three argument variable key contains the ASCII of the key pressed, while x and y tells the
 * program coordinates of mouse pointer when key was pressed.
 * */
void PrintableKeys(unsigned char key, int x, int y) {
    if (key == KEY_ESC) {
        gameState = gameState == MAIN_MENU ? EXIT : MAIN_MENU;
    }

    switch (gameState) {
        case MAIN_MENU:
            if (key == '1') {
                gameState = START_GAME;

                resetGame();

            } else if (key == '2') {
                gameState = RESUME_GAME;
            } else if (key == '3') {
                gameState = CHANGE_LEVEL_MENU;
            } else if (key == '4') {
                gameState = HIGH_SCORE;
            } else if (key == '5') {
                gameState = GAME_HISTORY;
            } else if (key == '6') {
                gameState = EXIT;
            }
            break;

        case START_GAME:
            if (key == 'p' || key == 'P') {
                gameState = MAIN_MENU;
            }
            break;

        case RESUME_GAME:
            if (key == 'p' || key == 'P') {
                gameState = MAIN_MENU;
            }
            break;

        case CHANGE_LEVEL_MENU:
            if (key == '1') {
                hurdleTimerLimit = 40 * FPS;
                snakeSpeedTimerLimit = 12;
                hurdleLengthMax = 4;
                maxHurdles = 1;

                resetGame();
            } else if (key == '2') {
                hurdleTimerLimit = 35 * FPS;
                snakeSpeedTimerLimit = 10;
                hurdleLengthMax = 5;
                maxHurdles = 2;

                resetGame();
            } else if (key == '3') {
                hurdleTimerLimit = 30 * FPS;
                snakeSpeedTimerLimit = 8;
                hurdleLengthMax = 6;
                maxHurdles = 3;

                resetGame();
            } else if (key == '4') {
                hurdleTimerLimit = 25 * FPS;
                snakeSpeedTimerLimit = 6;
                hurdleLengthMax = 8;
                maxHurdles = 4;

                resetGame();
            } else if (key == '5') {
                hurdleTimerLimit = 20 * FPS;
                snakeSpeedTimerLimit = 5;
                hurdleLengthMax = 12;
                maxHurdles = 6;

                resetGame();
            } else if (key == '6') {
                hurdleTimerLimit = 10 * FPS;
                snakeSpeedTimerLimit = 4;
                hurdleLengthMax = 15;
                maxHurdles = 7;

                resetGame();
            } else if (key == '7') {
                hurdleTimerLimit = 5 * FPS;
                snakeSpeedTimerLimit = 3;
                hurdleLengthMax = 17;
                maxHurdles = 10;

                resetGame();
            }
            gameState = MAIN_MENU;
            break;
    }

    glutPostRedisplay();
}

/*
 * This function is called after every 1000.0/FPS milliseconds
 * (FPS is defined on in the beginning).
 * You can use this function to animate objects and control the
 * speed of different moving objects by varying the constant FPS.
 *
 * */

void Timer(int m) {
    // implement your functionality here
    switch (gameState) {
        case MAIN_MENU:
            break;

        case START_GAME:
            if (!isGameOver) {
                update();
            }
            break;

        case RESUME_GAME:
            if (!isGameOver) {
                update();
            }
            break;

        case CHANGE_LEVEL_MENU:
            break;

        case HIGH_SCORE:
            break;

        case EXIT:
            exit(0);
            break;
    }

    glutPostRedisplay();
    // once again we tell the library to call our Timer function after next 1000/FPS
    glutTimerFunc(1000.0 / FPS, Timer, 0);
}

/*
 * our gateway main function
 * */
int main(int argc, char* argv[]) {
    init();

    InitRandomizer();       // seed the random number generator...
    glutInit(&argc, argv);  // initialize the graphics library...

    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);   // we will be using color display mode
    glutInitWindowPosition(300, 200);               // set the initial position of our window
    glutInitWindowSize(canvasWidth, canvasHeight);  // set the size of our window
    glutCreateWindow("PF's Snake Game");            // set the title of our game window
    SetCanvasSize(canvasWidth, canvasHeight);       // set the number of pixels...

    // Register your functions to the library,
    // you are telling the library names of function to call for different tasks.
    // glutDisplayFunc(display); // tell library which function to call for drawing Canvas.
    glutDisplayFunc(Display);           // tell library which function to call for drawing Canvas.
    glutSpecialFunc(NonPrintableKeys);  // tell library which function to call for non-printable ASCII characters
    glutKeyboardFunc(PrintableKeys);    // tell library which function to call for printable ASCII characters
                                        // This function tells the library to call our Timer function after 1000.0/FPS milliseconds...
    glutTimerFunc(5.0 / FPS, Timer, 0);

    // now handle the control to library and it will call our registered functions when
    // it deems necessary...
    glutMainLoop();
    return 1;
}
#endif

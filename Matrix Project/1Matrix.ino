// Matrix
#include <stdint.h>
#include "LedControl.h"

#define ROWS    8
#define COLS    8
#define MAX_LVL 10
#define MAX_BRI 14

static const uint8_t matrix_csPin  = 10;
static const uint8_t matrix_clkPin = 11;
static const uint8_t matrix_dinPin = 12;
static const uint8_t matrixNo      = 1;
static LedControl    lc            = LedControl(matrix_dinPin, matrix_clkPin, matrix_csPin, matrixNo); //DIN, CLK, LOAD, No. DRIVER
uint8_t Matrix_brightness = 5;

void Matrix_Init () {
  lc.setIntensity(0, Matrix_brightness); // sets brightness (0~14 possible values)
  lc.clearDisplay(0); // clear screen
  lc.shutdown(0, false); // turn off power saving, enables display
}

void Matrix_UpdateBrightness ()
{
  lc.setIntensity(0, Matrix_brightness); // sets brightness (0~14 possible values)
}

static void Print (uint8_t mat[COLS], uint8_t firstCol) {
  for (uint8_t col = 0; col < COLS; ++col) {
    lc.setColumn (0, col, mat[(firstCol + col) % COLS]);
  }
}

// animation function
void Matrix_Animate() {
  static uint8_t  state = 0;
  static uint64_t next  = 0,
                  now;
  static uint8_t  animation[][8] = { // MatirxPrint prints colums so animation [0][0] is the first column of the first animation
   {0b10101100,
    0b01001110,
    0b10101011,
    0b00001001,
    0b00001001,
    0b01001011,
    0b10101110,
    0b01001100},

   {0b01001100,
    0b10101110,
    0b01001011,
    0b00001001,
    0b00001001,
    0b10101011,
    0b01001110,
    0b10101100}
  };

  now = millis();
  if (now >= next) {
    next = now + 100; // delay 0.1s
    Print (animation[state], 0);
    state = !state;
  }
}

// returns an obstacle
static uint8_t Factory (uint8_t lvl) {
  static const uint8_t factoryMin[] = {0,  0,  5,  5, 10, 10, 10, 15, 15, 15, 15},
                       factoryMax[] = {5, 10, 10, 15, 15, 21, 21, 21, 21, 28, 28};
  if (lvl > MAX_LVL) {return 0b00000000;}
  switch (random(factoryMin[lvl], factoryMax[lvl])){
    // 0 & 1
    case  0: return 0b11000000;
    case  1: return 0b00110000;
    case  2: return 0b00011000;
    case  3: return 0b00001100;
    case  4: return 0b00000011;
    // 1 & 2 & 3
    case  5: return 0b11100000;
    case  6: return 0b00111000;
    case  7: return 0b00111100;
    case  8: return 0b00011100;
    case  9: return 0b00000111;
    // 3 & 4 & 5 & 6
    case 10: return 0b11110000;
    case 11: return 0b11100001;
    case 12: return 0b11000011;
    case 13: return 0b10000111;
    case 14: return 0b00001111;
    // 5 & 6 & 7 & 8 & 9 & 10
    case 15: return 0b11111000;
    case 16: return 0b11110001;
    case 17: return 0b11100011;
    case 18: return 0b11000111;
    case 19: return 0b10001111;
    case 20: return 0b00011111;
    // 9 & 10
    case 21: return 0b11001111;
    case 22: return 0b11100111;
    case 23: return 0b11110011;
    case 24: return 0b00110011;
    case 25: return 0b11001100;
    case 26: return 0b01100110;
    case 27: return 0b10011001;
    // it shouldn't be here
    default: return 0b00000000;
  }
}

static const uint16_t nextMoveDebounce[MAX_LVL + 1] = {500, 450, 425, 400, 375, 350, 300, 250, 200, 175, 150}; // refresh rate for each level
static const uint8_t  nextObsToNextLvl[MAX_LVL + 1] = { 10,  10,  10,  15,  15,  15,  50,  50,  50,  50, 100}, // obstacles to pass without fail to advance to the next level
                      timeToFinishLvl [MAX_LVL + 1] = { 30,  35,  40,  50,  50,  50,  60,  60,  80,  80, 255}, // the time you have for each level or it's game over
                      obsDist                       = 4; // there will be a distance of 3 empty columns between each 2 consecutive obstacles
static uint8_t        cols[COLS],   // matrix kept as a circular list of columns
                      head,         // where is the first column of the matrix
                      playerPos,    // the y coordonate of the player
                      obsToNextLvl; // current number of obstacles to pass to advance to the next level
static uint64_t       now,          // the current time
                      nextMove;     // the time when the matrix has to refresh
uint8_t               lives,        // number of remaining lives (no more than 9)
                      level = 0;    // current level
float                 remTime,      // the remaining time to finish the current level
                      sc;       // score
                      
// called before running Matrix_InGame to set/reset the game variables
void Matrix_GameSetup ()
{
  lc.clearDisplay(0);// clear screen
  for (uint8_t col = 0; col < COLS; ++col) {
    cols[col] = 0b00000000; // clear matrix
  }
  head         = 0;
  lives        = 3;
  playerPos    = 4;
  sc           = 0;
  obsToNextLvl = nextObsToNextLvl[level];
  remTime      = timeToFinishLvl [level];
  nextMove     = millis() + obsToNextLvl;
}

// Play the game
void Matrix_InGame() {
  now = millis();
  if (now > nextMove) {
    if (remTime >= (now - nextMove + nextMoveDebounce[level])*1.0/1000) {
      remTime -= (now - nextMove + nextMoveDebounce[level])*1.0/1000;
    }
    else {if (level != MAX_LVL){gameState = GS_EndGame; return;}} // the last level has no time limit
    if (cols[head] & (1 << playerPos)) { // collision
      if (lives > 0) {lives -= 1;}
      else {gameState = GS_EndGame; return;}
      obsToNextLvl = nextObsToNextLvl[level] + COLS / obsDist; // restart level
      for (uint8_t col = 0; col < COLS; col += obsDist) { // Clear matrix of obstacles
        cols[col] = 0b00000000;
      }
    }
    // print matrix
    cols[head] |= (1 << playerPos);
    Print (cols, head);
    // update matrix
    if (head % obsDist == 0) {
      score = sc += ((cols[head] != (1 << playerPos)) * remTime * 100.0) / nextObsToNextLvl[level] / nextMoveDebounce[level];
      Serial.println(score);
      if (obsToNextLvl > 1) {obsToNextLvl--;}
      else {
        if (level < MAX_LVL) {
          level++;
          if (lives < 9) {lives++;} // number of lives should be only a digit
        }
        remTime = timeToFinishLvl[level]; // 30s time to finish the level
        obsToNextLvl = nextObsToNextLvl[level];
      }
      cols[head] = Factory(level); // update the next (future) last column
    }
    else {cols[head] = 0b00000000;}
    // move player
    head = (head + 1) % COLS;
    if (Js_btnIsPressed()) {
      if (playerPos < COLS - 1) {playerPos++;}
    }
    else {
      if (playerPos > 0) {playerPos--;}
    }
    // update when the next refresh to be
    nextMove = now + nextMoveDebounce[level];
  }
}

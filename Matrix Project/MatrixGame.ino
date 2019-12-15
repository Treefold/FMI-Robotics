#include <LiquidCrystal.h>
#include <EEPROM.h>
#include "LedControl.h"

#define ROWS 8
#define COLS 8

// Matrix
const uint8_t matrix_csPin  = 10;
const uint8_t matrix_clkPin = 11;
const uint8_t matrix_dinPin = 12;
const uint8_t matrixNo      = 1;
LedControl lc = LedControl(matrix_dinPin, matrix_clkPin, matrix_csPin, matrixNo); //DIN, CLK, LOAD, No. DRIVER

void MatrixPrint (uint8_t mat[COLS], uint8_t firstCol) {
  for (uint8_t col = 0; col < COLS; ++col) {
    lc.setColumn (0, col, mat[(firstCol + col) % COLS]);
  }
}

// Screen lcd
const uint8_t lcd_rsPin     = 7;
const uint8_t lcd_enablePin = 6;
const uint8_t lcd_d4Pin     = 5;
const uint8_t lcd_d5Pin     = 4;
const uint8_t lcd_d6Pin     = 3;
const uint8_t lcd_d7Pin     = 2;
LiquidCrystal lcd (lcd_rsPin, lcd_enablePin, lcd_d4Pin, lcd_d5Pin, lcd_d6Pin, lcd_d7Pin);

// joystick JS
const int  JS_vrxPin     = A0;
const int  JS_vryPin     = A1;
const int  JS_btnPin     = 13;
const bool JS_btnPressed = 0;
uint16_t   JS_vrxValue;
uint16_t   JS_vryValue;
uint16_t   JS_btnValue;

bool JS_btnIsPressed() {
  return (digitalRead (JS_btnPin) == JS_btnPressed
          && digitalRead (JS_btnPin) == JS_btnPressed
          && digitalRead (JS_btnPin) == JS_btnPressed);
}

// Highscore HS
#define HS_RESET (bool)    false
#define HS_TYPE            uint16_t
#define HS_LEN   (uint8_t) 2 // (bytes)
#define NAME_LEN (uint8_t) 8 // (bytes)
char    HS_Name [NAME_LEN];
enum {
  HS_ADR   = 0,
  NAME_ADR = HS_ADR + HS_LEN
};
HS_TYPE HS_Read () {
  HS_TYPE score = 0;
  for (uint8_t currByte = 0; currByte < HS_LEN; ++currByte) {
    score = (score << 8) + EEPROM.read (HS_ADR + currByte);
  }
  return score;
}
void HS_Write (HS_TYPE score) {
  for (uint8_t currByte = HS_LEN; currByte > 0; --currByte) {
    EEPROM.write (HS_ADR + currByte - 1, (score & 0xFF));
    score >>= 8;
  }
}
void NAME_Read (char name[NAME_LEN]) {
  for (uint8_t currByte = 0; currByte < NAME_LEN - 1; ++currByte) {
    name[currByte] = EEPROM.read (NAME_ADR + currByte);
    // Serial.println( (int) EEPROM.read (NAME_ADR + currByte));
  }
  name[NAME_LEN - 1] = '\0';
}
void NAME_Write (char name[NAME_LEN]) {
  uint8_t currByte = 0;
  while (currByte < NAME_LEN && name[currByte] != '\0') {
    // Serial.print (name[currByte]);
    EEPROM.write (NAME_ADR + currByte, name[currByte]);
    currByte++;
  }
  while (currByte < NAME_LEN) {
    EEPROM.write (NAME_ADR + (currByte++), '\0');
  }
}
void HS_Update (HS_TYPE newScore, char name[NAME_LEN], bool ignoreMax)
{
  HS_TYPE lastScore = HS_Read();
  if (newScore > lastScore || ignoreMax) {
    HS_Write   (newScore);
    NAME_Write (name);
  }
}
void lcd_printString (char *str) {
  uint8_t ch = 0;
  while (ch < 16 && str[ch] != '\0') {
    lcd.print ((char) str[ch++]);
  }
  while (ch++ < 16) {
    lcd.print (' ');
  }
}
void setup() {
  Serial.begin (9600);
  lc.shutdown(0, true); // turn on power saving, disables display
  lc.setIntensity(0, 2); // sets brightness (0~15 possible values)
  lc.clearDisplay(0);// clear screen
  lc.shutdown(0, false); // turn off power saving, enables display
  lc.clearDisplay(0);// clear screen
  lcd.begin(16, 2);
  lcd.clear();
  pinMode (JS_btnPin, INPUT_PULLUP);
  if (HS_RESET) {
    strcpy (HS_Name, "wNot Set");
    HS_Update (0, HS_Name, true);
  }
  else {
    NAME_Read (HS_Name);
  }
}
enum GameStates : uint8_t { // GS
  GS_Meniu,
  GS_InGame,
  GS_Pause,
  GS_EndGame,
  GS_Hs,
  GS_HsSaveName
} gameState = GS_InGame; // GS_Meniu;

enum MeniuState : uint8_t {
  MS_FirstState, // link to the previous of the last state
  MS_Start,
  MS_Lvl,
  MS_Hs,
  MS_LastState   // link to the next of the first state
} meniuState = MS_Start;

HS_TYPE score = 0;
uint8_t lives = 3,
        level = 0;

const char     endMsg[]       = "Congratulation, you finished the game. Press button to restart";
const uint8_t  maxLvl    = 10;
char           name[NAME_LEN] = "       ";
int8_t         currLet        = 0;
uint64_t       currTime,
               startTime,
               lastLvlUpTime,
               startCountDown,
               lastShown;
uint16_t       currMsgBit,
               lastVrxValue = 512,
               lastVryValue = 512;
bool           lastBtnValue = !JS_btnPressed,
               lockedMeniu  = false,
               saveName     = false,
               waitBtnRls   = false;
 float         remTime      = 30;

void Meniu() {
  if (!lockedMeniu) {
    JS_vrxValue = analogRead (JS_vrxPin);
    if (JS_vrxValue < 300) {
      if (lastVrxValue >= 300) {
        meniuState = meniuState - 1;
        lastBtnValue = !JS_btnPressed;
        lastVrxValue = 0;
      }
    }
    else {
      if (JS_vrxValue > 600) {
        if (lastVrxValue <= 600) {
          meniuState = meniuState + 1;
          lastBtnValue = !JS_btnPressed;
          lastVrxValue = 1023;
        }
      }
      else {
        lastVrxValue = 512;
      }
    }
  }
  switch (meniuState) {
    case MS_Start:
      lcd.setCursor (0, 0);
      lcd.print ("BoringGame Meniu");
      lcd.setCursor (0, 1);
      // check the btn was really pressed
      if (!waitBtnRls && JS_btnIsPressed()) {
        if (lastBtnValue == !JS_btnPressed) {
          startCountDown = millis();
          lastBtnValue   = JS_btnPressed;
          lockedMeniu    = true;
          lcd.print (">Start in 3      ");
          break;
        }
        if (millis() > startCountDown + 3000) {
          startTime  = lastLvlUpTime = millis();
          score      = 3 * level;
          gameState  = GS_InGame;
          waitBtnRls = true;
          lcd.clear();
          lc.shutdown(0, false); // turn off power saving, enables display
          lc.clearDisplay(0);// clear screen
          lc.setLed(0, 3, 3, true);
        }
        else {
          lcd.setCursor (10, 1);
          if (millis() > startCountDown + 2000) {
            lcd.print (1);
          }
          else {
            if (millis() > startCountDown + 1000) {
              lcd.print (2);
            }
          }
        }
      }
      else {
        if (digitalRead (JS_btnPin) == !JS_btnPressed) {
          waitBtnRls = false;
        }
        lcd.print (">Press to start ");
        lockedMeniu  = false;
        lastBtnValue = !JS_btnPressed;
      }
      break;
    case MS_Lvl:
      lcd.setCursor (0, 0);
      lcd.print ((lockedMeniu) ? ">Press To UnLock" : ">Press To Lock  ");
      lcd.setCursor (0, 1);
      lcd.print ("level: ");
      lcd.print (level);
      if (level == 0) {
        lcd.print (" - MIN");
      }
      if (level >= maxLvl) {
        lcd.print (" - MAX");
      }
      lcd.print ("        ");
      if (JS_btnIsPressed()) {
        if (lastBtnValue == !JS_btnPressed) {
          lockedMeniu  = !lockedMeniu;
          lastBtnValue = JS_btnPressed;
        }
      }
      else {
        lastBtnValue = !JS_btnPressed;
      }
      if (lockedMeniu) {
        JS_vryValue = analogRead (JS_vryPin);
        if (JS_vryValue < 300) {
          if (lastVryValue >= 300) {
            if (level > 0) {
              --level;
            }
            lastVryValue = 0;
          }
        }
        else {
          if (JS_vryValue > 600) {
            if (lastVryValue <= 600) {
              if (level < maxLvl) {
                ++level;
              }
              lastVryValue = 1023;
            }
          }
          else {
            lastVryValue = 512;
          }
        }
      }
      break;
    case MS_Hs:
      lcd.setCursor (0, 0);
      lcd.print (">HS:            ");
      lcd.setCursor (5, 0);
      lcd.print (HS_Read());
      lcd.print ("-");
      lcd_printString (HS_Name);
      lcd.setCursor (0, 1);
      if (JS_btnIsPressed()) {
        if (lastBtnValue == !JS_btnPressed) {
          startCountDown = millis();
          lastBtnValue   = JS_btnPressed;
          lockedMeniu    = true;
          lcd.print (">Reset in 3      ");
          break;
        }
        if (millis() > startCountDown + 3000) {
          strcpy (HS_Name, "Not Set");
          HS_Update (0, HS_Name, true);
          lcd.print (">Done           ");
        }
        else {
          lcd.setCursor (10, 1);
          if (millis() > startCountDown + 2000) {
            lcd.print (1);
          }
          else {
            if (millis() > startCountDown + 1000) {
              lcd.print (2);
            }
          }
        }
      }
      else {
        lcd.print ("Hold to reset HS");
        lockedMeniu = false;
        lastBtnValue = !JS_btnPressed;
      }
      break;
    case MS_LastState:  meniuState = MS_FirstState + 1; break;
    case MS_FirstState: meniuState = MS_LastState  - 1; break;
  }
}

void EndGame() {
  lcd.setCursor (0, 0);
  lcd.print ("Lvl:");
  if (level < 10) {
    lcd.print (" ");
  }
  lcd.print (level);
  lcd.print (" Score:");
  if (score < 100) {
    lcd.print (" ");
  }
  lcd.print (score);
  if (score < 10) {
    lcd.print (" ");
  }
  lcd.setCursor (0, 1);
  if (millis() > lastShown + 500) {
    lastShown = millis();
    if (endMsg[currMsgBit] == '\0') {
      currMsgBit = 0;
    }
    lcd_printString (endMsg + currMsgBit++);
  }
  if (!waitBtnRls && JS_btnIsPressed()) {
    waitBtnRls = true;
    if (score > HS_Read()) {
      waitBtnRls = true;
      gameState  = GS_Hs;
    }
    else {
      gameState  = GS_Meniu;
      meniuState = MS_Lvl;
      saveName   = false;
      level     -= 5;
      score      = 3 * level;
    }
  }
  else {
    if (digitalRead (JS_btnPin) == !JS_btnPressed) {
      waitBtnRls = false;
    }
  }
}

void HsMeniu() {
  lcd.setCursor (0, 0);
  lcd.print ("NEW HS -        ");
  lcd.setCursor (9, 0);
  lcd.print (score);
  lcd.setCursor (0, 1);
  if (analogRead (JS_vrxPin) < 300) {
    saveName = true;
  }
  if (analogRead (JS_vrxPin) > 600) {
    saveName = false;
  }
  if (!waitBtnRls && JS_btnIsPressed()) {
    if (lastBtnValue == !JS_btnPressed) {
      lcd.setCursor (0, 1);
      lcd.print ((saveName) ? ">Set Name: Yes 3" : ">Set Name:  No 3");
      startCountDown = millis();
      lastBtnValue   = JS_btnPressed;
      return;
    }
    if (millis() > startCountDown + 3000) {
      waitBtnRls = true;
      if (saveName) {
        gameState = GS_HsSaveName;
        strcpy (name, "       ");
        currLet = 0;
        lastVrxValue = 512;
        lastVryValue = 512;
        lcd.setCursor (0, 0);
        lcd.print ("Name:|         |");
        return;
      }
      else {
        strcpy (HS_Name, "UnKnown");
        HS_Update (score, HS_Name, true);
        gameState  = GS_Meniu;
        meniuState = MS_Lvl;
        saveName   = false;
        level     -= 5;
        score      = 3 * level;
      }
    }
    else {
      lcd.setCursor (15, 1);
      if (millis() > startCountDown + 2000) {
        lcd.print (1);
      }
      else {
        if (millis() > startCountDown + 1000) {
          lcd.print (2);
        }
      }
    }
  }
  else {
    if (digitalRead (JS_btnPin) == !JS_btnPressed) {
      waitBtnRls = false;
    }
    lcd.print ((saveName) ? "Set Name: Yes > " : "Set Name: < No  ");
    lastBtnValue = !JS_btnPressed;
  }
}
void LcdInGame() {
  lcd.setCursor (0, 0);
  lcd.print ("Lives: ");lcd.print (lives);
  lcd.print (" Lvl: "); lcd.print (level);
  lcd.setCursor (0, 1);
  //lcd.print ("Score: ");lcd.print (score);
  lcd.print ("TIME: ");
  if (level < maxLvl) {lcd.print ((uint16_t) remTime);}
  else                {lcd.print ("INF");}
}
void HsSaveName() {
  lcd.setCursor (6, 0);
  for (uint8_t currCh = 0; currCh < 7; ++currCh) {
    if (currCh == currLet) {
      lcd.print ('>');
      lcd.print (name[currCh]);
      lcd.print ('<');
    }
    else {
      lcd.print (name[currCh]);
    }
  }
  lcd.setCursor (0, 1);
  if (!waitBtnRls && JS_btnIsPressed()) {
    if (lastBtnValue == !JS_btnPressed) {
      lcd.setCursor (0, 1);
      lcd.print (">Name saved in 3");
      startCountDown = millis();
      lockedMeniu    = true;
      lastBtnValue   = JS_btnPressed;
      return;
    }
    if (millis() > startCountDown + 3000) {
      strcpy (HS_Name, name);
      HS_Update (score, HS_Name, true);
      gameState   = GS_Meniu;
      meniuState  = MS_Lvl;
      saveName    = false;
      lockedMeniu = false;
      level       = 0;
      score       = 0;
      waitBtnRls = true;
    }
    else {
      lcd.setCursor (15, 1);
      if (millis() > startCountDown + 2000) {
        lcd.print (1);
      }
      else {
        if (millis() > startCountDown + 1000) {
          lcd.print (2);
        }
      }
    }
  }
  else {
    if (digitalRead (JS_btnPin) == !JS_btnPressed) {
      waitBtnRls = false;
    }
    lcd.print (" Press To Save  ");
    lastBtnValue = !JS_btnPressed;
    lockedMeniu = false;
  }
  if (!lockedMeniu) {
    //Serial.println (currLet);
    JS_vrxValue = analogRead (JS_vrxPin);
    JS_vryValue = analogRead (JS_vryPin);
    if (JS_vrxValue < 300) {
      if (lastVrxValue >= 300 && JS_vryValue >= 300 && JS_vryValue <= 600 && currLet > 0) {
        currLet--;
        lastVrxValue = 0;
        lastVryValue = 512;
      }
    }
    else {
      if (JS_vrxValue > 600) {
        if (lastVrxValue <= 600 && JS_vryValue >= 300 && JS_vryValue <= 600 && currLet < 6) {
          currLet++;
          lastVrxValue = 1023;
          lastVryValue = 512;
        }
      }
      else {
        lastVrxValue = 512;
        if (JS_vryValue < 300) {
          if (lastVryValue >= 300) {
            lastVryValue = 0;
            if (name[currLet] == ' ') {
              name[currLet] = 'Z';
            }
            else {
              if (name[currLet] == 'A') {
                name[currLet] = ' ';
              }
              else {
                name[currLet]--;
              }
            }
          }
        }
        else {
          if (JS_vryValue > 600) {
            if (lastVryValue <= 600) {
              lastVryValue = 1023;
              if (name[currLet] == ' ') {
                name[currLet] = 'A';
              }
              else {
                if (name[currLet] == 'Z') {
                  name[currLet] = ' ';
                }
                else {
                  name[currLet]++;
                }
              }
            }
          }
          else {
            lastVryValue = 512;
          }
        }
      }
    }
  }
}
void loop() {
  switch (gameState) {
    case GS_Meniu:
      MatrixAnimate();
      Meniu();
      break;
    case GS_InGame:
      LcdInGame();
      MatrixInGame();
      break;
    case GS_EndGame:
      MatrixAnimate();
      EndGame();
      break;
    case GS_Hs:
      MatrixAnimate();
      HsMeniu();
      break;
    case GS_HsSaveName:
      MatrixAnimate();
      HsSaveName();
      break;
    default:
      MatrixAnimate();
      gameState = GS_Meniu;
      break;
  }
}
uint8_t factoryMin[] = {0,  0,  5,  5, 10, 10, 10, 15, 15, 15, 15},
        factoryMax[] = {5, 10, 10, 15, 15, 21, 21, 21, 21, 28, 28};
uint8_t Factory (uint8_t lvl) {
  if (lvl > maxLvl) {return 0b00000000;}
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
    default: return 0b00000000; 
  }
}
//= ========================================================================================================================
const uint16_t nextMoveDebounce[maxLvl + 1] = {500, 450, 425, 400, 375, 350, 300, 250, 200, 175, 150};
const uint8_t  nextObsToNextLvl[maxLvl + 1]     = { 10,  10,  10,  15,  15,  15,  50,  50,  50,  50, 250};
const uint8_t obsDist = 4;
uint8_t cols[8] = {0, 0, 0, 0, 0, 0, 0, 0}, playerPos = 0, head = 0, obsToNextLvl = nextObsToNextLvl[level];
uint64_t now, nextMove = millis();
void MatrixAnimate() {
  static bool     state = 0;
  static uint64_t next  = 0, 
                  now;
  static uint8_t  animation[][8] = { // MatirxPrint prints colums so animation [0][0] is the firs column of the fist animation
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
    next = now + 100; // delay 0.1 s
    MatrixPrint (animation[state], 0);
    state = !state;
  }
}
void MatrixInGame() {
  // return MatrixAnimate();
  now = millis();
  if (now > nextMove) {
    if (remTime >= (now - nextMove + 500)*1.0/1000) {
      remTime -= (now - nextMove + 500)*1.0/1000;
    }
    else {gameState = GS_EndGame; return;}
    if (cols[head] & (1 << playerPos)) { // collision
      if (lives > 0) {lives -= 1;}
      else {gameState = GS_EndGame; return;}
      obsToNextLvl = nextObsToNextLvl[level] + COLS / obsDist; // restart level
      for (uint8_t col = 0; col < COLS; col += obsDist) { // Clear matrix of obstacles
        cols[col] = 0b00000000;
      }
    }
    cols[head] |= (1 << playerPos);
    MatrixPrint (cols, head);
    if (head % obsDist == 0) {
      if (obsToNextLvl > 1) {obsToNextLvl--;}
      else {
        if (level < maxLvl) {
          level++;
          if (lives < 9) {lives++;} // number of lives should be only a digit
        }
        obsToNextLvl = nextObsToNextLvl[level];
      }
      cols[head] = Factory(level);
    }
    else {cols[head] = 0b00000000;}
    head = (head + 1) % COLS;
    if (JS_btnIsPressed()) {
      if (playerPos < COLS - 1) {playerPos++;}
    }
    else {   
      if (playerPos > 0) {playerPos--;}
    }
    nextMove = now + nextMoveDebounce[level];
  }
}

#include <LiquidCrystal.h>
#include <EEPROM.h>

// Screen lcd
const int lcd_rsPin     = 12;
const int lcd_enablePin = 11;
const int lcd_d4Pin     = 5;
const int lcd_d5Pin     = 4;
const int lcd_d6Pin     = 3;
const int lcd_d7Pin     = 2;
LiquidCrystal lcd (lcd_rsPin, lcd_enablePin, lcd_d4Pin, lcd_d5Pin, lcd_d6Pin, lcd_d7Pin);

// joystick JS
const int  JS_vrxPin     = A0;
const int  JS_vryPin     = A1;
const int  JS_btnPin     = 13;
const bool JS_btnPressed = 0;
uint16_t   JS_vrxValue;
uint16_t   JS_vryValue;
uint16_t   JS_btnValue;

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
    Serial.println( (int) EEPROM.read (NAME_ADR + currByte));
  } 
  name[NAME_LEN - 1] = '\0';
}
void NAME_Write (char name[NAME_LEN]) {
  uint8_t currByte = 0;
  while (currByte < NAME_LEN && name[currByte] != '\0') {
    Serial.print (name[currByte]);
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
  while (ch < 16 && str[ch] != '\0') {lcd.print ((char) str[ch++]);}
  while (ch++ < 16)                  {lcd.print (' ');}
}
void setup() {
  Serial.begin (9600);
  lcd.begin(16, 2);
  lcd.clear();
  pinMode (JS_btnPin, INPUT_PULLUP);
  if (HS_RESET) {strcpy (HS_Name, "wNot Set"); HS_Update (0, HS_Name, true);}
  else          {NAME_Read (HS_Name);}
  lcd.setCursor (0, 0);
  lcd.print ("Name:|         |");
}

enum GameStates: uint8_t { // GS
  GS_Meniu,
  GS_InGame, 
  GS_Pause,
  GS_EndGame,
  GS_Hs,
  GS_HsSaveName
} gameState = GS_Meniu;

enum MeniuState: uint8_t {
  MS_FirstState, // link to the previous of the last state
  MS_Start,
  MS_Lvl,
  MS_Hs,
  MS_LastState   // link to the next of the first state
} meniuState = MS_Start;

HS_TYPE score = 0;
uint8_t lives = 3, 
        level = 5;

const char     endMsg[]       = "Congratulation, you finished the game. Press button to restart";
const uint8_t  maxBeginLvl    = 10;
const uint64_t endTime        = 10000, // ms
               lvlUpTime      =  2000; // ms
char           name[NAME_LEN] = "       ";
int8_t         currLet      = 0;
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
void loop() {
  switch (gameState) {
  case GS_Meniu:
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
        if (!waitBtnRls
         && digitalRead (JS_btnPin) == JS_btnPressed
         && digitalRead (JS_btnPin) == JS_btnPressed
         && digitalRead (JS_btnPin) == JS_btnPressed)
        {
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
        if (level == 0)           {lcd.print (" - MIN");}
        if (level >= maxBeginLvl) {lcd.print (" - MAX");}
        lcd.print ("        ");
        if (digitalRead (JS_btnPin) == JS_btnPressed
         && digitalRead (JS_btnPin) == JS_btnPressed
         && digitalRead (JS_btnPin) == JS_btnPressed)
        {
          if (lastBtnValue == !JS_btnPressed) {
            lockedMeniu  = !lockedMeniu;   
            lastBtnValue = JS_btnPressed;
          }
        }
        else {lastBtnValue = !JS_btnPressed;}
        if (lockedMeniu) {
          JS_vryValue = analogRead (JS_vryPin);
          if (JS_vryValue < 300) {
            if (lastVryValue >= 300) {
              if (level > 0) {--level;}
              lastVryValue = 0;
            }
          }
          else {
            if (JS_vryValue > 600) {
              if (lastVryValue <= 600) {
                if (level < maxBeginLvl) {++level;}
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
        lcd.setCursor (0, 1);if (digitalRead (JS_btnPin) == JS_btnPressed
         && digitalRead (JS_btnPin) == JS_btnPressed
         && digitalRead (JS_btnPin) == JS_btnPressed)
        {
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
    break;
  case GS_InGame:
    currTime = millis();
    if (currTime > lastLvlUpTime + lvlUpTime) {
      lastLvlUpTime += lvlUpTime;
      ++ level;
      score = 3 * level;
    }
    if (currTime > startTime + endTime) {
      lastShown  = 0;
      currMsgBit = 0;
      gameState  = GS_EndGame;
    }
    lcd.setCursor (0, 0);
    lcd.print ("Lives: ");
    lcd.print (lives);
    lcd.print (" Lvl: ");
    lcd.print (level);
    lcd.setCursor (0, 1);
    lcd.print ("Score: ");
    lcd.print (score);
    break;
  case GS_EndGame:
    lcd.setCursor (0, 0);
    lcd.print ("Lvl:");
    if (level < 10) {lcd.print (" ");}
    lcd.print (level);
    lcd.print (" Score:");
    if (score < 100) {lcd.print (" ");}
    lcd.print (score);
    if (score < 10) {lcd.print (" ");}
    lcd.setCursor (0, 1);
    if (millis() > lastShown + 500) {
      lastShown = millis();
      if (endMsg[currMsgBit] == '\0') {currMsgBit = 0;}
      lcd_printString (endMsg + currMsgBit++);
    }
     if (!waitBtnRls
       && digitalRead (JS_btnPin) == JS_btnPressed
       && digitalRead (JS_btnPin) == JS_btnPressed
       && digitalRead (JS_btnPin) == JS_btnPressed)
      {
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
    break;
    case GS_Hs:
      lcd.setCursor (0, 0);
      lcd.print ("NEW HS -        ");
      lcd.setCursor (9, 0);
      lcd.print (score);
      lcd.setCursor (0, 1);
      if (analogRead (JS_vrxPin) < 300) {saveName = true;}
      if (analogRead (JS_vrxPin) > 600) {saveName = false;}
      if (!waitBtnRls
         && digitalRead (JS_btnPin) == JS_btnPressed
         && digitalRead (JS_btnPin) == JS_btnPressed
         && digitalRead (JS_btnPin) == JS_btnPressed)
        {
          if (lastBtnValue == !JS_btnPressed) {
            lcd.setCursor (0, 1);
            lcd.print ((saveName) ? ">Set Name: Yes 3" : ">Set Name:  No 3");
            startCountDown = millis();
            lastBtnValue   = JS_btnPressed;  
            break;
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
              break;
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
    break;
    case GS_HsSaveName:
      lcd.setCursor (6, 0);
      for (uint8_t currCh = 0; currCh < 7; ++currCh) {
        if (currCh == currLet) {
          lcd.print ('>');
          lcd.print (name[currCh]);
          lcd.print ('<');
        }
        else {lcd.print (name[currCh]);}
      }
      lcd.setCursor (0, 1);
      if (!waitBtnRls
         && digitalRead (JS_btnPin) == JS_btnPressed
         && digitalRead (JS_btnPin) == JS_btnPressed
         && digitalRead (JS_btnPin) == JS_btnPressed)
      {
        if (lastBtnValue == !JS_btnPressed) {
          lcd.setCursor (0, 1);
          lcd.print (">Name saved in 3");
          startCountDown = millis();
          lockedMeniu    = true;
          lastBtnValue   = JS_btnPressed;  
          break;
        }
        if (millis() > startCountDown + 3000) {
            strcpy (HS_Name, name);
            HS_Update (score, HS_Name, true); 
            gameState   = GS_Meniu;
            meniuState  = MS_Lvl;
            saveName    = false;
            lockedMeniu = false;
            level      -= 5;
            score       = 3 * level; 
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
      Serial.println (JS_vrxValue);
      if (JS_vrxValue < 300) {
        if (lastVrxValue >= 300 && JS_vryValue >= 300 && JS_vryValue <= 600 && currLet > 0) {currLet--;lastVrxValue = 0;lastVryValue = 512;}
      }
      else {
        if (JS_vrxValue > 600) {
          if (lastVrxValue <= 600 && JS_vryValue >= 300 && JS_vryValue <= 600 && currLet < 6) {currLet++;lastVrxValue = 1023;lastVryValue = 512;}
        }
        else {
          lastVrxValue = 512;
          if (JS_vryValue < 300) {
            if (lastVryValue >= 300) {
              lastVryValue = 0;
              if (name[currLet] == ' ') {name[currLet] = 'Z';}
              else {
                if (name[currLet] == 'A') {name[currLet] = ' ';}
                else {name[currLet]--;}
              }
            }
          }
          else {
            if (JS_vryValue > 600) {
              if (lastVryValue <= 600) {
                lastVryValue = 1023;
                if (name[currLet] == ' ') {name[currLet] = 'A';}
                else {
                  if (name[currLet] == 'Z') {name[currLet] = ' ';}
                  else {name[currLet]++;}
                }
              }
            }
            else {lastVryValue = 512;}
          }
        }
      }
    }
    break;
  }
}

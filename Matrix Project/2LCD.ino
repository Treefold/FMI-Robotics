// Screen lcd
#include <LiquidCrystal.h>
#define LCD_ROWS 2
#define LCD_COLS 16

const uint8_t lcd_rsPin     = 7;
const uint8_t lcd_enablePin = 6;
const uint8_t lcd_d4Pin     = 5;
const uint8_t lcd_d5Pin     = 4;
const uint8_t lcd_d6Pin     = 3;
const uint8_t lcd_d7Pin     = 2;
LiquidCrystal lcd (lcd_rsPin, lcd_enablePin, lcd_d4Pin, lcd_d5Pin, lcd_d6Pin, lcd_d7Pin);

bool           lastBtnValue = !Js_btnPressed,
               lockedMeniu  = false, //
               saveName     = false,
               waitBtnRls   = false;
uint16_t       currMsgBit,
               lastVrxValue = 512,
               lastVryValue = 512;
const char     endMsg[]       = "Congratulation, you have just died! Press button to continue";
char           name[NAME_LEN] = "       ";
int8_t         currLet        = 0;
uint64_t       currTime,
               startTime,
               lastLvlUpTime,
               startCountDown,
               lastShown;

void Lcd_Init() {
  lcd.begin(LCD_COLS, LCD_ROWS);
  lcd.clear();
}

static void Lcd_printString (char *str) {
  uint8_t ch = 0;
  while (ch < LCD_COLS && str[ch] != '\0') {
    lcd.print ((char) str[ch++]);
  }
  while (ch++ < LCD_COLS) {
    lcd.print (' ');
  }
}

void Lcd_Meniu() {
  if (!lockedMeniu) {
    Js_ReadX();
    if (Js_vrxValue < Js_LeftPoint) {
      if (lastVrxValue != Js_LeftPoint) {
        meniuState = (MeniuState) ((int) meniuState - 1);
        lastBtnValue = !Js_btnPressed;
        lastVrxValue = Js_LeftPoint;
      }
    }
    else {
      if (Js_vrxValue > Js_RightPoint) {
        if (lastVrxValue != Js_RightPoint) {
          meniuState = (MeniuState) ((int) meniuState + 1);
          lastBtnValue = !Js_btnPressed;
          lastVrxValue = Js_RightPoint;
        }
      }
      else {
        lastVrxValue = Js_NoPoint;
      }
    }
  }
  switch (meniuState) {
    case MS_Start:
      lcd.setCursor (0, 0);
      lcd.print ("BoringGame Meniu");
      lcd.setCursor (0, 1);
      // check the btn was really pressed
      if (Js_btnIsPressed()){
        if (!waitBtnRls) {
          if (lastBtnValue == !Js_btnPressed) {
            startCountDown = millis();
            lastBtnValue   = Js_btnPressed;
            lockedMeniu    = true;
            lcd.print (">Start in 3      "); // wait 3s
            break;
          }
          if (millis() > startCountDown + 3000) { // done waiting
            Matrix_GameSetup();
            gameState  = GS_InGame;
            waitBtnRls = true;
            lcd.clear();
          }
          else {
            lcd.setCursor (10, 1);
            if (millis() > startCountDown + 2000) { // wait 1s
              lcd.print (1);
            }
            else {
              if (millis() > startCountDown + 1000) { // wait 2s
                lcd.print (2);
              }
            }
          }
        }
        else {
          lcd.print (">Press to start ");
          lockedMeniu  = false;
          lastBtnValue = !Js_btnPressed;
        }
      }
      else {
        waitBtnRls = false;
        lcd.print (">Press to start ");
        lockedMeniu  = false;
        lastBtnValue = !Js_btnPressed;
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
      if (level >= MAX_LVL) {
        lcd.print (" - MAX");
      }
      lcd.print ("        ");
      if (Js_btnIsPressed()) {
        if (lastBtnValue == !Js_btnPressed) {
          lockedMeniu  = !lockedMeniu;
          lastBtnValue = Js_btnPressed;
        }
      }
      else {
        lastBtnValue = !Js_btnPressed;
      }
      if (lockedMeniu) {
        Js_vryValue = analogRead (Js_vryPin);
        if (Js_vryValue < 300) {
          if (lastVryValue >= 300) {
            if (level > 0) {
              --level;
            }
            lastVryValue = 0;
          }
        }
        else {
          if (Js_vryValue > 600) {
            if (lastVryValue <= 600) {
              if (level < MAX_LVL) {
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
      Lcd_printString (HS_Name);
      lcd.setCursor (0, 1);
      if (Js_btnIsPressed()) {
        if (lastBtnValue == !Js_btnPressed) {
          startCountDown = millis();
          lastBtnValue   = Js_btnPressed;
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
        lastBtnValue = !Js_btnPressed;
      }
      break;
    case MS_LastState:  meniuState = (MeniuState) ((int) MS_FirstState + 1); break;
    case MS_FirstState: meniuState = (MeniuState) ((int) MS_LastState  - 1); break;
  }
}

void Lcd_EndGame() {
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
    Lcd_printString ((char*)endMsg + (currMsgBit++));
  }
  if (!waitBtnRls && Js_btnIsPressed()) {
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
    if (digitalRead (Js_btnPin) == !Js_btnPressed) {
      waitBtnRls = false;
    }
  }
}

void Lcd_HsMeniu() {
  lcd.setCursor (0, 0);
  lcd.print ("NEW HS -        ");
  lcd.setCursor (9, 0);
  lcd.print (score);
  lcd.setCursor (0, 1);
  if (analogRead (Js_vrxPin) < 300) {
    saveName = true;
  }
  if (analogRead (Js_vrxPin) > 600) {
    saveName = false;
  }
  if (!waitBtnRls && Js_btnIsPressed()) {
    if (lastBtnValue == !Js_btnPressed) {
      lcd.setCursor (0, 1);
      lcd.print ((saveName) ? ">Set Name: Yes 3" : ">Set Name:  No 3");
      startCountDown = millis();
      lastBtnValue   = Js_btnPressed;
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
    if (digitalRead (Js_btnPin) == !Js_btnPressed) {
      waitBtnRls = false;
    }
    lcd.print ((saveName) ? "Set Name: Yes > " : "Set Name: < No  ");
    lastBtnValue = !Js_btnPressed;
  }
}
void Lcd_InGame() {
  lcd.setCursor (0, 0);
  lcd.print ("Lives: ");lcd.print (lives);
  lcd.print (" Lvl: "); lcd.print (level);
  lcd.setCursor (0, 1);
  //lcd.print ("Score: ");lcd.print (score);
  lcd.print ("TIME: ");
  if (level < MAX_LVL) {lcd.print ((uint16_t) remTime);}
  else                {lcd.print ("INF");}
}
void Lcd_HsSaveName() {
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
  if (!waitBtnRls && Js_btnIsPressed()) {
    if (lastBtnValue == !Js_btnPressed) {
      lcd.setCursor (0, 1);
      lcd.print (">Name saved in 3");
      startCountDown = millis();
      lockedMeniu    = true;
      lastBtnValue   = Js_btnPressed;
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
    if (digitalRead (Js_btnPin) == !Js_btnPressed) {
      waitBtnRls = false;
    }
    lcd.print (" Press To Save  ");
    lastBtnValue = !Js_btnPressed;
    lockedMeniu = false;
  }
  if (!lockedMeniu) {
    //Serial.println (currLet);
    Js_vrxValue = analogRead (Js_vrxPin);
    Js_vryValue = analogRead (Js_vryPin);
    if (Js_vrxValue < 300) {
      if (lastVrxValue >= 300 && Js_vryValue >= 300 && Js_vryValue <= 600 && currLet > 0) {
        currLet--;
        lastVrxValue = 0;
        lastVryValue = 512;
      }
    }
    else {
      if (Js_vrxValue > 600) {
        if (lastVrxValue <= 600 && Js_vryValue >= 300 && Js_vryValue <= 600 && currLet < 6) {
          currLet++;
          lastVrxValue = 1023;
          lastVryValue = 512;
        }
      }
      else {
        lastVrxValue = 512;
        if (Js_vryValue < 300) {
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
          if (Js_vryValue > 600) {
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

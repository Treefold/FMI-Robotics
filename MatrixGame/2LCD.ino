// LCD *DO NOT RENAME FILE*
#include <LiquidCrystal.h>
#define LCD_ROWS 2
#define LCD_COLS 16

const uint8_t lcd_rsPin     = 9;
const uint8_t lcd_enablePin = 8;
const uint8_t lcd_d4Pin     = 7;
const uint8_t lcd_d5Pin     = 6;
const uint8_t lcd_d6Pin     = 5;
const uint8_t lcd_d7Pin     = 4;
LiquidCrystal lcd (lcd_rsPin, lcd_enablePin, lcd_d4Pin, lcd_d5Pin, lcd_d6Pin, lcd_d7Pin);

bool           lastBtnValue   = !js_btnPressed,
               lockedMeniu    = false, //
               saveName       = false,
               waitBtnRls     = false;
uint16_t       currMsgBit,
               lastVrxValue   = 512,
               lastVryValue   = 512;
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
  lcd.setCursor (0, 0);
  lcd.print (" Are YOU breave ");
  lcd.setCursor (0, 1);
  lcd.print (" enough to try? ");
  delay (5000); // I have nothing to do during this waiting time so millis doesn't make a difference
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
    if (js_vrxValue < js_LeftPoint) {
      if (lastVrxValue != js_LeftPoint) {
        meniuState   = (MeniuState) ((int) meniuState - 1);
        waitBtnRls   = true;
        lastVrxValue = js_LeftPoint;
      }
    }
    else {
      if (js_vrxValue > js_RightPoint) {
        if (lastVrxValue != js_RightPoint) {
          meniuState   = (MeniuState) ((int) meniuState + 1);
          waitBtnRls   = true;
          lastVrxValue = js_RightPoint;
        }
      }
      else {
        lastVrxValue = js_NoPoint;
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
          if (lastBtnValue == !js_btnPressed) {
            startCountDown = millis();
            lastBtnValue   = js_btnPressed;
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
          lastBtnValue = !js_btnPressed;
        }
      }
      else {
        lcd.print (">Press to start ");
        waitBtnRls   = false;
        lockedMeniu  = false;
        lastBtnValue = !js_btnPressed;
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
        level = MAX_LVL;
        lcd.print (" - MAX");
      }
      lcd.print ("        ");
      if (Js_btnIsPressed()) {
        if (lastBtnValue == !js_btnPressed) {
          lockedMeniu  = !lockedMeniu;
          lastBtnValue = js_btnPressed;
        }
      }
      else {
        lastBtnValue = !js_btnPressed;
      }
      if (lockedMeniu) {
        Js_ReadY ();
        if (js_vryValue > js_DownPoint) {
          if (lastVryValue != js_DownPoint) {
            if (level > 0) {--level;}
            lastVryValue = js_DownPoint;
          }
        }
        else {
          if (js_vryValue < js_UpPoint) {
            if (lastVryValue != js_UpPoint) {
              if (level < MAX_LVL) {++level;}
              lastVryValue = js_UpPoint;
            }
          }
          else {
            lastVryValue = js_NoPoint;
          }
        }
      }
      break;
    case MS_Brightness:
      lcd.setCursor (0, 0);
      lcd.print ((lockedMeniu) ? ">Press To UnLock" : ">Press To Lock  ");
      lcd.setCursor (0, 1);
      lcd.print ("Brightness:     ");
      lcd.setCursor (12, 1);
      lcd.print (matrix_brightness);
      if (Js_btnIsPressed()) {
        if (lastBtnValue == !js_btnPressed) {
          lockedMeniu  = !lockedMeniu;
          lastBtnValue = js_btnPressed;
        }
      }
      else {
        lastBtnValue = !js_btnPressed;
      }
      if (lockedMeniu) {
        Js_ReadY ();
        if (js_vryValue > js_DownPoint) {
          if (lastVryValue != js_DownPoint) {
            if (matrix_brightness > 1) {--matrix_brightness;}
            lastVryValue = js_DownPoint;
          }
        }
        else {
          if (js_vryValue < js_UpPoint) {
            if (lastVryValue != js_UpPoint) {
              if (matrix_brightness < MAX_BRI) {++matrix_brightness;}
              lastVryValue = js_UpPoint;
            }
          }
          else {
            lastVryValue = js_NoPoint;
          }
        }
        Matrix_UpdateBrightness();
      }
      break;
    case MS_Hs:
      static uint8_t rank = 0;
      char   nameHS[NAME_LEN];
      lcd.setCursor (0, 0);
      lcd.print ((lockedMeniu) ? ">Press To UnLock" : ">Press To Lock  ");
      lcd.setCursor (0, 1);
      lcd.print ("HS");
      lcd.print (rank + 1);
      lcd.print (":           ");
      lcd.setCursor (5, 1);
      lcd.print (HS_Read(rank));
      lcd.print ("-");
      NAME_Read(nameHS, rank);
      Lcd_printString (nameHS);

      if (Js_btnIsPressed()) {
        if (lastBtnValue == !js_btnPressed) {
          lockedMeniu  = !lockedMeniu;
          lastBtnValue = js_btnPressed;
        }
      }
      else {
        lastBtnValue = !js_btnPressed;
      }
      if (lockedMeniu) {
        Js_ReadY ();
        if (js_vryValue > js_DownPoint) {
          if (lastVryValue != js_DownPoint) {
            if (rank < HS_NO - 1) {++rank;}
            lastVryValue = js_DownPoint;
          }
        }
        else {
          if (js_vryValue < js_UpPoint) {
            if (lastVryValue != js_UpPoint) {
              if (rank > 0) {--rank;}
              lastVryValue = js_UpPoint;
            }
          }
          else {
            lastVryValue = js_NoPoint;
          }
        }
      }
      
      break;
    case MS_InfoCreator:
      lcd.setCursor (0, 0);
      lcd.print ("Creator:  Surcea");
      lcd.setCursor (0, 1);
      lcd.print ("Mihai  -  Daniel");
      break;
    case MS_InfoGit:
      lcd.setCursor (0, 0);
      lcd.print ("   github.com/  ");
      lcd.setCursor (0, 1);
      lcd.print ("    Treefold    ");
      break;
    case MS_InfoRobotics:
      lcd.setCursor (0, 0);
      lcd.print ("  Flappy Birds  ");
      lcd.setCursor (0, 1);
      lcd.print ("@UnibucRobotics ");
      break;
    case MS_LastState:  meniuState = (MeniuState) ((int) MS_FirstState + 1); break;
    case MS_FirstState: meniuState = (MeniuState) ((int) MS_LastState  - 1); break;
    default:            meniuState = (MeniuState) ((int) MS_FirstState + 1); break; 
  }
}

void Lcd_EndGame() {
  lcd.setCursor (0, 0);
  lcd.print ("Lvl:");
  if (level < 10) {lcd.print (" ");} // only one digit
  lcd.print (level);
  lcd.print (" Score:");
  if (score < 100) {lcd.print (" ");} // less than 3 digits
  lcd.print (score);
  if (score < 10) { lcd.print (" ");} // only one digit
  lcd.setCursor (0, 1);
  if (millis() > lastShown + 500) { // refresh rate
    lastShown = millis();
    if (endMsg[currMsgBit] == '\0') {currMsgBit = 0;} // reprint the message
    Lcd_printString ((char*)endMsg + (currMsgBit++));
  }
  if (Js_btnIsPressed()) {
    if (!waitBtnRls) {
      waitBtnRls = true;
      if (score > HS_Read(HS_NO - 1)) { // last rank
        saveName   = false;
        gameState  = GS_Hs;
      }
      else {
        gameState  = GS_Meniu;
        meniuState = MS_Start;
      }
    }
  }
  else {waitBtnRls = false;}
}

void Lcd_HsMeniu() {
  lcd.setCursor (0, 0);
  lcd.print ("NEW HS -        ");
  lcd.setCursor (9, 0);
  lcd.print (score);
  lcd.setCursor (0, 1);
  Js_ReadX ();
  if (js_vrxValue < js_LeftPoint)  {saveName = true;}
  if (js_vrxValue > js_RightPoint) {saveName = false;}
  if (Js_btnIsPressed()){
    if (!waitBtnRls) {
      if (lastBtnValue == !js_btnPressed) {
        lcd.setCursor (0, 1);
        lcd.print ((saveName) ? ">Set Name: Yes 3" : ">Set Name:  No 3");
        startCountDown = millis();
        lastBtnValue   = js_btnPressed;
        return;
      }
      if (millis() > startCountDown + 3000) {
        waitBtnRls = true;
        if (saveName) {
          gameState = GS_HsSaveName;
          strcpy (name, "       ");
          currLet = 0;
          lastVrxValue = js_NoPoint;
          lastVryValue = js_NoPoint;
          lcd.setCursor (0, 0);
          lcd.print ("Name:|         |");
          return;
        }
        else {
          strcpy (name, "UnKnown");
          HS_Update (score, name);
          gameState  = GS_Meniu;
          meniuState = MS_Start;
          saveName   = false;
          return;
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
        return;
      }
    }
  }
  else {
    waitBtnRls   = false;
    lastBtnValue = !js_btnPressed;
  }
  lcd.print ((saveName) ? "Set Name: Yes > " : "Set Name: < No  ");
}

void Lcd_InGame() {
  lcd.setCursor (0, 0);
  lcd.print ("Lives: ");lcd.print (lives);
  lcd.print (" Lvl: "); lcd.print (level);
  lcd.setCursor (0, 1);
  lcd.print ("TIME: ");
  if (level < MAX_LVL) {
    if ((uint16_t) remTime < 10) {lcd.print (" ");}
    lcd.print ((uint16_t) remTime);
  }
  else {lcd.print ("INF");}
  lcd.print (" SC: ");
  lcd.print (score);
}

void Lcd_HsSaveName() {
  lcd.setCursor (6, 0);
  for (uint8_t currCh = 0; currCh < NAME_LEN - 1; ++currCh) { // -1 for the last char is '\0'
    if (currCh == currLet) {
      lcd.print ('>');
      lcd.print (name[currCh]);
      lcd.print ('<');
    }
    else {lcd.print (name[currCh]);}
  }
  
  lcd.setCursor (0, 1);
  if (Js_btnIsPressed()) {
    if (!waitBtnRls) {
      if (lastBtnValue == !js_btnPressed) {
        lcd.setCursor (0, 1);
        lcd.print (">Name saved in 3");
        startCountDown = millis();
        lockedMeniu    = true;
        lastBtnValue   = js_btnPressed;
      }
      if (millis() > startCountDown + 3000) {
        HS_Update (score, name);
        gameState   = GS_Meniu;
        meniuState  = MS_Start;
        saveName    = false;
        lockedMeniu = false;
        waitBtnRls  = true;
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
      return;
    }
  }
  else {
    lcd.print (" Press To Save  ");
    waitBtnRls   = false;
    lastBtnValue = !js_btnPressed;
    lockedMeniu  = false;
  }
  
  if (!lockedMeniu) {
    Js_ReadX ();
    Js_ReadY ();
    if (js_vrxValue < js_LeftPoint) {
      if (lastVrxValue != js_LeftPoint && js_vryValue >= js_UpPoint && js_vryValue <= js_DownPoint && currLet > 0) {
        currLet--;
        lastVrxValue = js_LeftPoint;
        lastVryValue = js_NoPoint;
      }
    }
    else {
      if (js_vrxValue > js_RightPoint) {
        if (lastVrxValue != js_RightPoint && js_vryValue >= js_UpPoint && js_vryValue <= js_DownPoint && currLet < NAME_LEN - 2) {
          // -2 for each: the last one is '\0' and cannot move to the right letter if there are no more letters
          currLet++;
          lastVrxValue = js_RightPoint;
          lastVryValue = js_NoPoint;
        }
      }
      else {
        lastVrxValue = js_NoPoint;
        if (js_vryValue > js_DownPoint) {
          if (lastVryValue != js_DownPoint) {
            lastVryValue = js_DownPoint;
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
          if (js_vryValue < js_UpPoint) {
            if (lastVryValue != js_UpPoint) {
              lastVryValue = js_UpPoint;
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
            lastVryValue = js_NoPoint;
          }
        }
      }
    }
  }
}

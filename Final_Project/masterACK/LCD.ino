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
    if (vrxValue == left) {
      if (lastVrxValue != left) {
        meniuState   = (MeniuState) ((int) meniuState - 1);
        waitBtnRls   = true;
        lastVrxValue = left;
      }
    }
    else {
      if (vrxValue == right) {
        if (lastVrxValue != right) {
          meniuState   = (MeniuState) ((int) meniuState + 1);
          waitBtnRls   = true;
          lastVrxValue = right;
        }
      }
      else {
        lastVrxValue = neutral;
      }
    }
  }
  switch (meniuState) {
    case MS_Start:
      lcd.setCursor (0, 0);
      lcd.print ("BoringGame Meniu");
      lcd.setCursor (0, 1);
      // check the btn was really pressed
      if (btnIsPressed()) {
        if (!waitBtnRls) {
          if (lastBtnValue == !pressed) {
            startCountDown = millis();
            lastBtnValue   = pressed;
            lockedMeniu    = true;
            lcd.print (">Start in 3      "); // wait 3s
            break;
          }
          if (millis() > startCountDown + 3000) { // done waiting
            while (mesagePlayer1(0x90 | level) != 0x7F);
            gameState  = GS_InSoloGame;
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
          lastBtnValue = !pressed;
        }
      }
      else {
        lcd.print (">Press to start ");
        waitBtnRls   = false;
        lockedMeniu  = false;
        lastBtnValue = !pressed;
      }
      break;
    case MS_StartDual:
      lcd.setCursor (0, 0);
      lcd.print ("MultiBoring Game");
      lcd.setCursor (0, 1);
      // check the btn was really pressed
      if (btnIsPressed()) {
        if (!waitBtnRls) {
          if (lastBtnValue == !pressed) {
            startCountDown = millis();
            lastBtnValue   = pressed;
            lockedMeniu    = true;
            lcd.print (">Start in 3      "); // wait 3s
            break;
          }
          if (millis() > startCountDown + 3000) { // done waiting
            setBrightness1 (bri);
            setBrightness2 (bri);
            while (mesagePlayer1(0x90 | level) != 0x7F);
            while (mesagePlayer2(0x90 | level) != 0x7F);
            gameState  = GS_InDualGame;
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
          lastBtnValue = !pressed;
        }
      }
      else {
        lcd.print (">Press to start ");
        waitBtnRls   = false;
        lockedMeniu  = false;
        lastBtnValue = !pressed;
      }
      break;
    case MS_Login:
      switch (loginState) {
        case guest:
          lcd.setCursor (0, 0);
          lcd.print ("You are a guest!");
          lcd.setCursor(0, 1);
          lcd.print (" Press to login ");
          if (btnIsPressed()) {
            if (!waitBtnRls) {
              rsp = mesageFingerp(0);
              gameState  = GS_FingerPrint;
              loginState = getCode;
              waitBtnRls = 1;
            }
          }
          else {
            waitBtnRls   = 0;
            lastBtnValue = !pressed;
          }
          break;
        case getCode:
          lcd.setCursor (0, 0);
          lcd.print ("Enter your code:");
          lcd.setCursor (0, 1);
          lcd.print ("(1-32)   ");
          rsp = mesageFingerp(1);
          lcd.print (rsp & 0x7F);
          lcd.print ("           ");
          if (rsp == 0x80) { // cancel
            rsp        = mesageFingerp(0);
            userId     = 0;
            loginState = guest;
            gameState  = GS_Meniu;
          }
          if (rsp > 0x80) {
            rsp        = mesageFingerp(0);
            userId     = rsp & 0x7F;
            gameState  = GS_Meniu;
            loginState = logged;
            authState  = deleting;
          }
          break;
        case logged:
          lcd.setCursor (0, 0);
          lcd.print ("Logged in as ");
          lcd.print (userId);
          lcd.print ("    ");
          lcd.setCursor (0, 1);
          lcd.print ("Press to log out");
          if (btnIsPressed()) {
            if (!waitBtnRls) {
              userId     = 0;
              loginState = guest;
              authState  = notAuth;
              waitBtnRls = 1;
            }
          }
          else {
            waitBtnRls = 0;
          }
          break;
        default: loginState = guest; break;
      }
      break;
    case MS_Auth:
      switch (authState) {
        case notAuth:
          lcd.setCursor (0, 0);
          lcd.print ("You are a guest!");
          lcd.setCursor(0, 1);
          lcd.print ("PressTo register");
          if (btnIsPressed()) {
            if (!waitBtnRls) {
              rsp = mesageFingerp(0);
              gameState = GS_FingerPrint;
              authState = setCode;
              waitBtnRls = 1;
            }
          }
          else {
            waitBtnRls   = 0;
            lastBtnValue = !pressed;
          }
          break;
        case setCode:
          rsp = mesageFingerp(2);
          lcd.setCursor (0, 0);
          lcd.print ("Enter your code:");
          lcd.setCursor (0, 1);
          lcd.print ("(1-32)   ");
          lcd.print (rsp & 0x7F);
          lcd.print ("           ");
          if (rsp == 0x80) { // cancel
            rsp       = mesageFingerp(0);
            userId    = 0;
            
            authState = notAuth;
            gameState = GS_Meniu;
          }
          if (rsp > 0x80) {
            rsp       = mesageFingerp(0);
            userId    = rsp & 0x7F;
            gameState = GS_Meniu;
            lockedMeniu    = true;
            authState = naming;
            saveName  = true;
            strncpy (name, "           ", NAME_LEN);
          }
          break;
        case naming:
          Lcd_SaveName();
          if (!saveName) {
            meniuState = MS_Login;
            loginState = logged;
            authState  = deleting;
          }
          break;
        case deleting:
          lcd.setCursor (0, 0);
          lcd.print ("Logged in as ");
          lcd.print (userId);
          lcd.print ("    ");
          lcd.setCursor (0, 1);
          if (btnIsPressed()) {
            if (!waitBtnRls) {
              if (lastBtnValue == !pressed) {
                startCountDown = millis();
                lastBtnValue   = pressed;
                lockedMeniu    = true;
                lcd.print ("Delete User in 3"); // wait 3s
                break;
              }
              if (millis() > startCountDown + 3000) { // done waiting
                if (mesageFingerp(0x80 | userId) == 0x82) { // user successfully deleted
                  userId      = 0;
                  lockedMeniu = false;
                  loginState  = guest;
                  authState   = notAuth;
                  waitBtnRls  = 1;
                }
                else {
                  lcd.setCursor (0, 1);
                  lcd.print ("Hold to tryAgain");
                }

              }
              else {
                lcd.setCursor (15, 1);
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
              lcd.print ("Press to delete ");
              lockedMeniu  = false;
              lastBtnValue = !pressed;
            }
          }
          else {
            lcd.print ("Press to delete ");
            waitBtnRls   = false;
            lockedMeniu  = false;
            lastBtnValue = !pressed;
          }
          break;
        default: authState = notAuth; break;
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
      if (btnIsPressed()) {
        if (lastBtnValue == !pressed) {
          lockedMeniu  = !lockedMeniu;
          lastBtnValue = pressed;
        }
      }
      else {
        lastBtnValue = !pressed;
      }
      if (lockedMeniu) {
        if (vryValue == down) {
          if (lastVryValue != down) {
            if (level > 0) {
              --level;
            }
            lastVryValue = down;
          }
        }
        else {
          if (vryValue == up) {
            if (lastVryValue != up) {
              if (level < MAX_LVL) {
                ++level;
              }
              lastVryValue = up;
            }
          }
          else {
            lastVryValue = neutral;
          }
        }
      }
      break;
    case MS_Bri:
      lcd.setCursor (0, 0);
      lcd.print ((lockedMeniu) ? ">Press To UnLock" : ">Press To Lock  ");
      lcd.setCursor (0, 1);
      lcd.print ("Brightness ");
      lcd.print (bri);
      if (bri == 0)  {
        lcd.print ("min");
      }
      if (bri == 15) {
        lcd.print ("MAX");
      }
      lcd.print ("     ");
      if (btnIsPressed()) {
        if (lastBtnValue == !pressed) {
          lockedMeniu  = !lockedMeniu;
          lastBtnValue = pressed;
          lastVrxValue = vrxValue;
          lastVryValue = vryValue;
        }
      }
      else {
        lastBtnValue = !pressed;
      }
      if (lockedMeniu) {
        if (vryValue == down) {
          if (lastVryValue != down) {
            if (bri > 0) {
              --bri;
            }
            lastVryValue = down;
            setBrightness1 (bri);
            setBrightness2 (bri, false);
          }
        }
        else {
          if (vryValue == up) {
            if (lastVryValue != up) {
              if (bri < 15) {
                ++bri;
              }
              lastVryValue = up;
              setBrightness1 (bri);
              setBrightness2 (bri, false);
            }
          }
          else {
            lastVryValue = neutral;
          }
        }
      }
      break;
    case MS_Hs:
      static uint8_t rank = 1;
      static User    usr;
      usr.id = GetIdByRank(rank);
      lcd.setCursor (0, 0);
      lcd.print ((lockedMeniu) ? ">Press To UnLock" : ">Press To Lock  ");
      lcd.setCursor (0, 1);
      lcd.print ("HS");
      lcd.print (rank);
      if (usr.id == 0) {lcd.print (": 0 - NOTSET ");}
      else {
        EEPROM_GetUser (usr);
        // PrintUser (usr);
        lcd.print (": ");
        lcd.print (usr.score);
        lcd.print ("-");
        lcd.print (usr.name);
        lcd.print ("           ");
      }

      if (btnIsPressed()) {
        if (lastBtnValue == !pressed) {
          lockedMeniu  = !lockedMeniu;
          lastBtnValue = pressed;
        }
      }
      else {
        lastBtnValue = !pressed;
      }
      if (lockedMeniu) {
        if (vryValue == down) {
          if (lastVryValue != down) {
            if (rank < RANKERS_NO ) {++rank;}
            lastVryValue = down;
          }
        }
        else {
          if (vryValue == up) {
            if (lastVryValue != up) {
              if (rank > 1) {
                --rank;
              }
              lastVryValue = up;
            }
          }
          else {
            lastVryValue = neutral;
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

void Lcd_EndSoloGame() {
  lcd.setCursor (0, 0);
  lcd.print ("Lvl:");
  if (level < 10) {
    lcd.print (" "); // only one digit
  }
  lcd.print (level);
  lcd.print (" Score:");
  if (score < 100) {
    lcd.print (" "); // less than 3 digits
  }
  lcd.print (score);
  if (score < 10) {
    lcd.print (" "); // only one digit
  }
  lcd.setCursor (0, 1);
  if (millis() > lastShown + 750) { // refresh rate
    lastShown = millis();
    if (endMsg[currMsgBit] == '\0') {
      currMsgBit = 0; // reprint the message
    }
    Lcd_printString ((char*)endMsg + (currMsgBit++));
  }
  if (btnIsPressed  ()) {
    if (!waitBtnRls) {
      waitBtnRls = true;
      if (userId != 0 && score > GetScoreById (userId)) {
        SetScoreById (userId, score);
      }
      uint8_t r = RANKERS_NO;
      while (score > GetScoreByRank(r) && r > 0) {
        SetIdByRank (r+1, GetIdByRank(r));
        --r;
      }
      SetIdByRank (r+1, userId);
      gameState  = GS_Meniu;
      meniuState = MS_Start;
    }
  }
  else {
    waitBtnRls = false;
  }
}

void Lcd_InSoloGame  () {
  lcd.setCursor (0, 0);
  lcd.print ("Lives: "); lcd.print (lives);
  lcd.print (" Lvl: "); lcd.print (level);
  lcd.setCursor (0, 1);
  lcd.print ("TIME: ");
  if (level < MAX_LVL) {
    if (remTime < 10) {
      lcd.print (" ");
    }
    lcd.print (remTime);
  }
  else {
    lcd.print ("INF");
  }
  lcd.print (" SC: ");
  lcd.print (score);
}

void Lcd_PrintScore (uint8_t lives, uint8_t level, uint8_t score, uint8_t remTime) {
  lcd.print ("Hp");
  lcd.print(lives);
  lcd.print("Lvl");
  if (level == 10) {lcd.print ("Inf");}
  else {
    lcd.print (" ");
    lcd.print (level);
    lcd.print (" ");    
  }
  lcd.print ("S");
  if (score  < 10) {lcd.print(" ");}
  lcd.print (score);
  if (score  < 100) {lcd.print(" ");}
  lcd.print ("T");
  lcd.print (remTime);
  lcd.print ("  ");
}

void Lcd_InDualGame  () {
  lcd.setCursor (0, 0);
  Lcd_PrintScore (lives, level, score, remTime);
  lcd.setCursor (0, 1);
  Lcd_PrintScore (lives2, level2, score2, remTime2);
}

void Lcd_SaveName() {
  lcd.setCursor (6, 0);
  for (uint8_t currCh = 0; currCh < NAME_LEN; ++currCh) {
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
  if (btnIsPressed()) {
    if (!waitBtnRls) {
      if (lastBtnValue == !pressed) {
        lcd.setCursor (0, 1);
        lcd.print (">Name saved in 3");
        startCountDown = millis();
        lastBtnValue   = pressed;
      }
      if (millis() > startCountDown + 3000) {
        User usr(userId);
        strcpy (usr.name, name);
        EEPROM_SetUser (usr);
        
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
    lastBtnValue = !pressed;
    lockedMeniu  = false;
  }

  if (!lockedMeniu) {
    if (vrxValue == left) {
      if (lastVrxValue != left && vryValue == up && vryValue == down && currLet > 0) {
        currLet--;
        lastVrxValue = left;
        lastVryValue = neutral;
      }
    }
    else {
      if (vrxValue == right) {
        if (lastVrxValue != right && vryValue == up && vryValue == down && currLet < NAME_LEN - 1) {
          // -1: the last one is '\0' and cannot move to the right letter if there are no more letters
          currLet++;
          lastVrxValue = right;
          lastVryValue = neutral;
        }
      }
      else {
        lastVrxValue = neutral;
        if (vryValue == down) {
          if (lastVryValue != down) {
            lastVryValue = down;
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
          if (vryValue == up) {
            if (lastVryValue != up) {
              lastVryValue = up;
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
            lastVryValue = neutral;
          }
        }
      }
    }
  }
}

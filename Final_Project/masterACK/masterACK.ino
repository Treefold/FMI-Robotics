// Game states and meniu states can be change anywhere
// This file must have the name of its containing file
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"
#define  PIN const uint8_t
#define neutral 0
#define left    1
#define right   2
#define down    1
#define up      2
#define pressed 1
#define MAX_LVL  10
#define LCD_COLS 16
#define SCORE_LEN   1
#define NAME_LEN    7

struct User {
  uint8_t id, 
          score;
  char    name [NAME_LEN + 1]; // the '\0' isn't saved

  User (uint8_t _id = 0) {
    id = _id;
    score = 0;
  }
};

PIN cePin               = 9, // rf24 cip enable
    csnPin              = 8, // rf24 cip select not
    incomingDataPin     = 2; // rf24 interrupt signal

uint8_t    vrxValue     = neutral,
           vryValue     = neutral,
           userId       = 0,
           bri          = 3,
           rsp          = 0;
bool       btnValue     = !pressed,
           lastBtnValue = !pressed,
           lockedMeniu  = false, //
           saveName     = false,
           waitBtnRls   = false;
uint16_t   currMsgBit,
           lastVrxValue = 0,
           lastVryValue = 0;
const char endMsg[]     = "Congratulation, you have just died! Press button to continue";
int8_t     currLet      = 0,
           level        = 3,
           score        = 0,
           lives        = 3,
           remTime      = 0,
           currLet2     = 0,
           level2       = 3,
           score2       = 0,
           lives2       = 3,
           remTime2     = 0;
uint64_t   currTime,
           startTime,
           lastLvlUpTime,
           startCountDown,
           lastShown;

enum GameStates : uint8_t { // GS
  GS_Meniu,
  GS_FingerPrint,
  GS_InSoloGame,
  GS_InDualGame,
  GS_EndSoloGame,
  GS_EndDualGame
} gameState = GS_Meniu;

enum MeniuState : uint8_t {
  MS_FirstState, // link to the previous of the last state
  MS_Start,
  MS_StartDual,
  MS_Login,
  MS_Auth,
  MS_Lvl,
  MS_Bri,
  MS_Hs,
  MS_InfoCreator,
  MS_InfoGit,
  MS_InfoRobotics,
  MS_LastState   // link to the next of the first state
} meniuState = MS_Start;

enum {
  logIn,
  selectGame,
  inGame,
  details
} masterState = logIn;

enum {
  guest, // ast to not remain a guest
  getCode,
  logged
} loginState = guest;

enum {
  notAuth,
  setCode,
  naming,
  deleting
} authState = notAuth;

// too many pins to declare each of them as a define
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);
RF24 radio(cePin, csnPin);  // CE, CSN
byte addresses[][6] = {"SSlve", "SMstr", "1Slve", "1Mstr", "2Slve", "2Mstr"};
struct Msg {
  uint8_t id, nr;
} msg;

uint8_t mesageFingerp(uint8_t st) {
  while (radio.available()) {
    radio.read (&msg, sizeof(Msg));
  }
  msg.id = 'S';
  msg.nr = st;
  radio.openWritingPipe(addresses[0]);
  if ( radio.write(&msg, sizeof(Msg)) ) {
    msg.id = 0;
    while (radio.available()) {
      radio.read( &msg, sizeof(Msg));
      if (msg.id != 'S') {
        Serial.println(msg.id);
        continue;
      }
      return msg.nr;
    }
  } else {
    Serial.println("Sending to Fingerprint failed");  // If no ack response, sending failed
  }
  return 0; // fault
}

uint8_t mesagePlayer1(uint8_t st, uint8_t id = '1') {
  while (radio.available()) {
    radio.read( &msg, sizeof(Msg));
  }
  msg.id = id;
  msg.nr = st;
  radio.openWritingPipe(addresses[2]);
  if ( radio.write(&msg, sizeof(Msg)) ) {
    msg.id = 0;
    while (radio.available()) {
      radio.read( &msg, sizeof(Msg));
      if (msg.id != id) {
        // Serial.print("Id "); Serial.println(msg.id);
        continue;
      }
      return msg.nr;
    }
  } else {
    //Serial.println("Sending to Controler1 failed");  // If no ack response, sending failed
  }
  return 0; // fault
}

uint8_t mesagePlayer2(uint8_t st, uint8_t id = '2') {
  while (radio.available()) {
    radio.read( &msg, sizeof(Msg));
  }
  msg.id = id;
  msg.nr = st;
  radio.openWritingPipe(addresses[4]);
  if ( radio.write(&msg, sizeof(Msg)) ) {
    msg.id = 0;
    while (radio.available()) {
      radio.read( &msg, sizeof(Msg));
      if (msg.id != id) {
        // Serial.print("Id "); Serial.println(msg.id);
        continue;
      }
      return msg.nr;
    }
  } else {
    //Serial.println("Sending to Controler2 failed");  // If no ack response, sending failed
  }
  return 0; // fault
}

void Askjs() { // ask joystick for its states: vrX, vrY, btn
  static uint8_t states;
  while (((states = mesagePlayer1 (0x80)) & 0xE0) != 0x80); // = 100byyxx
  vrxValue =  states & 3;              // xx
  vryValue = (states & (3 << 2)) >> 2; // yy
  btnValue = (states & (1 << 4)) >> 4; // b
}

void setBrightness1 (uint8_t bri) {
  while (mesagePlayer1 (0xC0 | bri)  != (0xE0 | bri));
}

void setBrightness2 (uint8_t bri, uint8_t inf = true, uint8_t tries = 3) {
  while (mesagePlayer2 (0xC0 | bri)  != (0xE0 | bri) && (inf || tries > 0)) --tries;
  // if inf = true => I don't care about tries, even if it underflows
}

bool btnIsPressed () {
  return btnValue == pressed;
}

bool getGameData1() { // max 10 ms if it's on
  //static uint64_t lastCheck = 0;
  //if (millis() > lastCheck + 50) {
    //lastCheck = millis();
    while (((rsp = mesagePlayer1 (0xB0)) & 0xF0) != 0x10);
    level = rsp & 0x0F-1;
    while ((rsp = mesagePlayer1 (0, 's')) == 0);
    score = rsp - 1;
    if (level != 10) {
      while ((rsp = mesagePlayer1 (0, 't')) == 0);
      remTime = rsp;
      if (remTime > 0) {
        remTime -= 1;
      }
      else {
        return 0;
      }
    }
    while (((rsp = mesagePlayer1 (0xD0)) & 0xF0) != 0x20);
    lives = (rsp & 0x0F) - 2;
    if (lives > 0) {
      lives -= 1;
    }
    else           {
      return 0;
    }
  //}
  return 1;
}

bool getGameData2() { // max 10 ms if it's on
  //static uint64_t lastCheck2 = 0;
  //if (millis() > lastCheck2 + 50) {
    //lastCheck2 = millis();
    while (((rsp = mesagePlayer2 (0xB0)) & 0xF0) != 0x10);
    level2 = rsp & 0x0F-1;
    while ((rsp = mesagePlayer2 (0, 's')) == 0);
    score2 = rsp - 1;
    if (level2 != 10) {
      while ((rsp = mesagePlayer2 (0, 't')) == 0);
      remTime2 = rsp;
      if (remTime2 > 0) {
        remTime2 -= 1;
      }
      else {
        return 0;
      }
    }
    while (((rsp = mesagePlayer2 (0xD0)) & 0xF0) != 0x20);
    lives2 = (rsp & 0x0F) - 2;
    if (lives2 > 0) {
      lives2 -= 1;
    }
    else            {
      return 0;
    }
  //}
  return 1;
}

void setup() {
  // EEPROM_Clean();
  Serial.begin(9600);
  Serial.println("start");
  lcd.begin(16, 2);
  lcd.setBacklight(100);
  lcd.clear();
  radio.begin();
  radio.enableAckPayload();
  radio.enableDynamicPayloads();
  radio.stopListening();
  radio.openWritingPipe(addresses[0]);
  radio.openReadingPipe(1, addresses[1]);
  radio.openReadingPipe(2, addresses[2]);
  radio.openReadingPipe(3, addresses[2]);
  lcd.setCursor (0, 0);
  lcd.print ("JoystickNotFound");
  lcd.setCursor (0, 1);
  lcd.print ("Please try again");
  User u (1);
  PrintUser (u);
  setBrightness1 (bri); // this also signals that the controler shoudn't be in "game mode";
  setBrightness2 (bri, false); // try to stop the second one
}

void loop(void) {
  switch (gameState) {
    case GS_Meniu:        Askjs(); Lcd_Meniu();          break;
    case GS_FingerPrint:           Lcd_Meniu();          break;
    case GS_InSoloGame:
      if (getGameData1()) {
        Lcd_InSoloGame();
        break;
      }
      else {
        gameState = GS_EndSoloGame;
        break;
      }
    case GS_InDualGame:
      if (getGameData1() | getGameData2())
      {
        Lcd_InDualGame();
        break;
      }
      else {
        gameState = GS_EndDualGame;
        break;
      }
    case GS_EndSoloGame: Askjs();  Lcd_EndSoloGame();    break;
    case GS_EndDualGame: Askjs();  Lcd_EndSoloGame();    break;
    default:             gameState = GS_Meniu;           break;
  }
}

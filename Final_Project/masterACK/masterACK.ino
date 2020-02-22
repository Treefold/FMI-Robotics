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
PIN cePin           = 9, // rf24 cip enable
    csnPin          = 8, // rf24 cip select not
    incomingDataPin = 2; // rf24 interrupt signal
    
uint8_t vrxValue = neutral,
        vryValue = neutral,
        user;
bool    btnValue = !pressed;

enum GameStates : uint8_t { // GS
  GS_Meniu,
  GS_FingerPrint,
  GS_InGame
} gameState = GS_Meniu;

enum MeniuState : uint8_t {
  MS_FirstState, // link to the previous of the last state
  MS_Start,
  MS_Login,
  MS_Auth,
  MS_Lvl,
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
  authed
} authState = notAuth;

// too many pins to declare each of them as a define
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  
RF24 radio(cePin, csnPin);  // CE, CSN
byte addresses[][6] = {"SSlve", "SMstr", "1Slve", "1Mstr"};
struct Msg {uint8_t id, nr;} msg;

void Askjs() { // ask joystick for its states: vrX, vrY, brn
  //static uint32_t lastJsMsg = 0;
  //if (lastJsMsg + 50 < millis()) {return 0;}
  //lastJsMsg = millis();
  msg.id = '1';
  msg.nr = 0x80;
  radio.openWritingPipe(addresses[2]);
  if (radio.write(&msg, sizeof(Msg))) { 
      while (radio.available() ) { 
        radio.read( &msg, sizeof(Msg));  
        //Serial.println(msg.nr, BIN);   
        vrxValue = msg.nr & 3;
        vryValue = (msg.nr & (3<<2)) >> 2;
        btnValue = (msg.nr & (1<<4)) >> 4;
      }
  } else {
    Serial.println("Sending failed."); 
  }
}

bool btnIsPressed () {
  return btnValue == pressed;
}


void setup() {
  Serial.begin(9600);
  Serial.println("start");
  lcd.begin(16,2); 
  lcd.clear(); 
  radio.begin();
  radio.enableAckPayload();
  radio.enableDynamicPayloads();
  radio.stopListening();
  radio.openWritingPipe(addresses[0]);
  radio.openReadingPipe(1, addresses[1]);
  radio.openReadingPipe(2, addresses[2]);
}

uint8_t mesageFingerp(uint8_t st) {
  msg.id = 'S';
  msg.nr = st;
  radio.openWritingPipe(addresses[0]);
  if ( radio.write(&msg, sizeof(Msg)) ) {
      msg.id = 0;
      while (radio.available()) {
        radio.read( &msg, sizeof(Msg)); 
        if(msg.id != 'S') {Serial.println(msg.id);continue;} 
        return msg.nr;
      }
  } else {
    Serial.println("Sending failed");  // If no ack response, sending failed
  }
  return 1;
}

void loop(void) {
  switch (gameState) {
    case GS_Meniu: Askjs(); break;
    case GS_FingerPrint: break;
    case GS_InGame: break;
    default: gameState = GS_Meniu; break;
  }
  Lcd_Meniu(); 
}

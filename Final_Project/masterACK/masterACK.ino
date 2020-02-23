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
        user     = 0,
        bri      = 3;
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
  MS_Bri,
  MS_Hs,
  MS_InfoCreator,
  MS_InfoGit,
  MS_InfoRobotics,
  MS_LastState   // link to the next of the first state
} meniuState = MS_Bri;// MS_Start;

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
  deleting
} authState = notAuth;

// too many pins to declare each of them as a define
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  
RF24 radio(cePin, csnPin);  // CE, CSN
byte addresses[][6] = {"SSlve", "SMstr", "1Slve", "1Mstr", "2Slve", "2Mstr"};
struct Msg {uint8_t id, nr;} msg;

uint8_t mesageFingerp(uint8_t st) {
  // while (radio.available()) {radio.read (&msg, sizeof(Msg));}
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
    Serial.println("Sending to Fingerprint failed");  // If no ack response, sending failed
  }
  return 0xFF; // fault
}

uint8_t mesagePlayer1(uint8_t st) {
  while (radio.available()) {radio.read( &msg, sizeof(Msg));}
  msg.id = '1';
  msg.nr = st;
  radio.openWritingPipe(addresses[2]);
  if ( radio.write(&msg, sizeof(Msg)) ) {
      msg.id = 0;
      while (radio.available()) {
        radio.read( &msg, sizeof(Msg)); 
        if(msg.id != '1') {Serial.println(msg.id);continue;}   
        return msg.nr;
      }
  } else {
    Serial.println("Sending to Controler failed");  // If no ack response, sending failed
  }
  return 0xFF; // fault
}

void Askjs() { // ask joystick for its states: vrX, vrY, btn
  static uint8_t states;
  while (((states = mesagePlayer1 (0x80)) & 0xE0) != 0x80); // = ---byyxx
  // if (states == 0xFF) {return;}  // fault message
  vrxValue =  states & 3;            // xx
  vryValue = (states & (3<<2)) >> 2; // yy
  btnValue = (states & (1<<4)) >> 4; // b
}

void setBrightness (uint8_t bri) {
  while (mesagePlayer1 (0xC0 | bri)  != (0xE0|bri));
}

bool btnIsPressed () {
  return btnValue == pressed;
}

void setup() {
  Serial.begin(9600);
  Serial.println("start");
  lcd.begin(16,2); 
  lcd.setBacklight(100);
  lcd.clear(); 
  radio.begin();
  radio.enableAckPayload();
  radio.enableDynamicPayloads();
  radio.stopListening();
  radio.openWritingPipe(addresses[0]);
  radio.openReadingPipe(1, addresses[1]);
  radio.openReadingPipe(2, addresses[2]);
  setBrightness (3);
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

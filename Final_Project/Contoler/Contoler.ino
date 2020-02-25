//  *DO NOT RENAME FILE*
//  the files names matter due to the compilling order
#include <SPI.h>
#include "RF24.h"
#define CONTROL 2 // 1 | 2
#define  PIN const uint8_t
PIN cePin           = 8, // rf24 cip enable
    csnPin          = 9, // rf24 cip select not
    incomingDataPin = 2; // rf24 interrupt signal
RF24 radio(cePin, csnPin);  // CE, CSN
byte addresses[][6] = {
  #if   CONTROL == 1 
    "1Slve", "1Mstr"
  #else              
    "2Slve", "2Mstr"
  #endif
};
struct Msg {
  uint8_t id, nr;
} msg;

uint8_t game = 1;

void att () {
  attachInterrupt (digitalPinToInterrupt (incomingDataPin), receiveData, LOW);
}
void deta () {
  detachInterrupt (digitalPinToInterrupt (incomingDataPin));
}

void receiveData() {
  radio.read (&msg, sizeof(Msg));
  Serial.print ((char)msg.id);Serial.print (" ");Serial.println (msg.nr, HEX);
  if (msg.id == '1' + CONTROL - 1) { // state = controler
    if (msg.nr == 0x80) {
      msg.nr = 0x80 | joystickState(); // mark with 0x1------- as a valit message
    }
    else {
      if ((msg.nr & 0x90) == 0x90) {
        switch (msg.nr & 0xF0) {
          case 0x90:
            if (isInGame == false) {
              //if (CONTROL == 2) {delay(10000);}
              level = msg.nr & 0x0F;
              Matrix_GameSetup();
              isInGame = true;
            }
            msg.nr = 0x7F;
            break;
          case 0xB0:
            msg.nr = 0x10 | level + 1;
            break;
          case 0xD0:
            msg.nr = 0x20 | lives + 2;
            break;
          default:   msg.id = msg.nr = 0;            break;
        }
      }
      else {
        if ((msg.nr & 0xC0) == 0xC0) {
          // this means the master has restarted => stop the game
          isInGame = false;
          if (msg.nr & 0x20) {
            msg.nr = 0xC0 | matrix_brightness;
          }
          else {
            matrix_brightness = msg.nr & (0x0F);
            lc.setIntensity(0, matrix_brightness);
            msg.nr = 0xE0 | matrix_brightness;
          }
        }
        else {
          msg.id = msg.nr = 0;
        }
      }
    }
  }
  else {
    if (msg.id == 's') {msg.nr = score + 1;}
    else {
      if (msg.id == 't') {msg.nr = remTime + 1;}
      else {msg.id = msg.nr = 0;}
    }
  }
  radio.writeAckPayload(1, &msg, sizeof(Msg));
}

void setup() {
  Serial.begin (9600);
  Serial.println("start");
  radio.begin();
  radio.enableAckPayload();
  radio.enableDynamicPayloads();
  radio.openWritingPipe(addresses[1]);
  radio.openReadingPipe(1, addresses[0]);
  while (radio.available()) {
    radio.read(&msg, sizeof(Msg));
  }
  att();
  radio.startListening();
  Js_Init();
  Matrix_Init();
}

void loop() {
  if (isInGame) {
    Matrix_InGame();
  }
  else          {
    Matrix_Animate();
  }
}

//  *DO NOT RENAME FILE*
//  the files names matter due to the compilling order
#include <SPI.h>
#include "RF24.h"
#define  PIN const uint8_t
PIN cePin           = 8, // rf24 cip enable
    csnPin          = 9, // rf24 cip select not
    incomingDataPin = 2; // rf24 interrupt signal  
RF24 radio(cePin, csnPin);  // CE, CSN
byte addresses[][6] = {"1Slve", "1Mstr"};
struct Msg {uint8_t id, nr;} msg;

enum {
  controler,
  gaming
} genState = controler;

uint8_t game = 1;

uint8_t joystickState () {
  // state = 0x000byyxx where:
  // b = button state: 0 - not pressed, 1 - pressed
  // yy = 00 - nothing, 01 - up,   10 - down
  // xx = 00 - nothing, 01 - left, 10 - right
  uint8_t state = 0,
          x     = analogRead   (A1),
          y     = analogRead   (A0),
          btn   = !digitalRead (3);
  
  if (x > 767) { // left
    state |= (1);
  }
  else { 
    if (x < 256) { // right
      state |= (2);
    }
    // else not modified
  }
  
  if (y > 767) { // up
    state |= (1) << 2;
  }
  else { 
    if (x < 256) { // down
      state |= (2) << 2;
    }
    // else not modified
  }
      
  state |= (btn) << 5; 

  return state;
}

void receiveData() {
  radio.read (&msg, sizeof(Msg));
  if (msg.id == '1' && msg.nr & 0x80) { // state = controler
    msg.nr = 0x80 | joystickState(); // mark with 0x1------- as a valit message
  }
  else {msg.id = msg.nr = 0;}
  radio.writeAckPayload(1, &msg, sizeof(Msg));
}

void setup() {
  Serial.begin (9600);
  radio.begin();
  radio.enableAckPayload();
  radio.enableDynamicPayloads();
  radio.openWritingPipe(addresses[1]);
  radio.openReadingPipe(1, addresses[0]);
  attachInterrupt (digitalPinToInterrupt (incomingDataPin), receiveData, LOW);
  radio.startListening();
  Js_Init();
  Matrix_Init();
}

void loop() {
    //Matrix_InGame();
    Serial.print (analogRead(A0));
    Serial.print (" ");
    Serial.print (analogRead(A1));
    Serial.print (" ");
    Serial.println (digitalRead(3));
}

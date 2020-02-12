#include <TimerOne.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#define  PIN const uint8_t
#define  PLYE_NO 4
/* 
 *  number of players 1-4: 
 *  only 6 channels for the radio receiver/transmiter  
 *  0:   writing pipe                         5 bytes      
 *  1:   reading pipe  - fingerprint sensor   5 bytes
 *  2-5: reading pipes - players              1 byte
 *  the 2-5 pipes copy the pipe 1("one", not 0) bytes, 
 *    exept for the first byte which
 *  ! pipe 0 can be used as a reading pipe, but the 
 *    writing pipe delets the reading pipe so you will
 *    have to reinitialize after each writing, taking
 *    the risk of losing some interrupts
 *  ! pipe 1 can be reutilized after authentification
 *    in order to have one more player, but four are
 *    enough
 */
PIN cePin           = 9, // rf24 cip enable
    csnPin          = 8, // rf24 cip select not
    incomingDataPin = 2; // rf24 interrupt signal

RF24 radio(cePin, csnPin);  // CE, CSN

//address through which two modules communicate.
const byte address[PLYE_NO + 1][6] = {
  "SPLYR", // sensor "player" address
  "1PLYR"
  #if PLYE_NO > 1
  ,"2PLYR"
  #endif    
  #if PLYE_NO > 2
  ,"3PLYR"
  #endif    
  #if PLYE_NO > 3
  ,"4PLYR"
  #endif    
};
enum Pipes: int {
  fingerprintSensor, 
  player1
  #if PLYE_NO > 1
  ,player2
  #endif    
  #if PLYE_NO > 2
  ,player3
  #endif    
  #if PLYE_NO > 3
  ,player4
  #endif   
};

struct Msg {
  uint8_t id, nr;
};

void tim() {
  static uint8_t x = 0;
  Msg msg;
  msg.id = x;
  msg.nr = 66 + 25 * x;
  
  radio.stopListening();
  delay(3);
  radio.openWritingPipe(address[x + 1]);
  radio.write(&msg, sizeof(msg));
  x = (x+1)%PLYE_NO;
  delay (3);
  radio.startListening();
}

void receiveData(){
  Msg msg;
  radio.read (&msg, sizeof(msg));
  Serial.print (msg.id);
  Serial.print (" ");
  Serial.println (msg.nr);
}

void setup()
{
  while (!Serial);
  Serial.begin (9600);
  Serial.println ("Transmiting");
  radio.begin();
  radio.stopListening();
  // pipe 1 for fingerpint 
  // and pipes from 2 to PLYE_NO(1-4) + 1 for players
  for (uint8_t pipe = 0; pipe <= PLYE_NO; ++pipe) {
    radio.openReadingPipe (pipe + 1, address [pipe]);
  }
  attachInterrupt (digitalPinToInterrupt (incomingDataPin), receiveData, LOW);
  Timer1.initialize(1000000);
  Timer1.attachInterrupt (tim);
}

void loop()
{
}

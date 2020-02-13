



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

enum SLAVE_STATS {
  idle,    // cancel
  getUser, // authenticate
  setUser, // register
};

const byte address[6] = "SPLYR";

struct Msg {
  uint8_t id, nr;
} msgCmd;

bool msgAcc = false, received = false;

void receiveData(){
  Msg msgRsp;
  radio.read (&msgRsp, sizeof(msgRsp));
  Serial.print ((char) msgRsp.id);
  Serial.print (" ");
  Serial.println (msgRsp.nr & 0x7F);
}

void setup()
{
  while (!Serial);
  Serial.begin (9600);
  Serial.println("Listening");
  radio.begin();
  radio.stopListening();
  // pipe 1 for fingerpint 
  attachInterrupt (digitalPinToInterrupt (incomingDataPin), receiveData, LOW);
  msgCmd.id = 'S';
  msgCmd.nr = getUser;
  radio.openWritingPipe (address);
  radio.write (&msgCmd, sizeof (msgCmd));
  radio.openReadingPipe (1, address);
  radio.startListening();
}

void loop()
{
}

#include <TimerOne.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#define  PIN const uint8_t

const byte ADDRESS[4][6] = {"1PLYR", "2PLYR", "3PLYR", "4PLYR"}; // master sends requests 

PIN cePin           = 9, // rf24 cip enable
    csnPin          = 8, // rf24 cip select not
    incomingDataPin = 2; // rf24 interrupt signal
     
RF24 radio(cePin, csnPin);

struct Msg {
  uint8_t id, nr;
};

void receiveData() {
  Msg msg;
  radio.read(&msg, sizeof(msg));
  Serial.print(msg.id);
  Serial.print(" ");
  Serial.println(msg.nr);
  msg.nr = msg.nr + 1 - 2 * (msg.nr == 1);
  radio.stopListening();
  delay(3);
  radio.openWritingPipe (ADDRESS[msg.id]);
  msg.id += 2;
  radio.write (&msg, sizeof(msg));
  delay(3);
  radio.startListening();
}

void setup()
{
  while (!Serial);
  Serial.begin(9600);
  Serial.println ("Listening");  
  
  radio.begin();
  radio.stopListening();
  radio.openReadingPipe (1, ADDRESS[0]);
  radio.openReadingPipe (2, ADDRESS[1]);
  radio.openReadingPipe (3, ADDRESS[2]);
  radio.openReadingPipe (4, ADDRESS[3]);
  attachInterrupt (digitalPinToInterrupt (incomingDataPin), receiveData, LOW);
  radio.startListening();

  //Timer1.initialize(1000000);         // initialize timer1, and set a 1/2 second period
  //Timer1.attachInterrupt(callback);  // attaches callback() as a timer overflow interrupt
}

void loop()
{
}

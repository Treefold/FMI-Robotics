#include <TimerOne.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#define  PIN const uint8_t
#include <Adafruit_Fingerprint.h>
SoftwareSerial mySerial(6, 7);
Adafruit_Fingerprint finger = Adafruit_Fingerprint (&mySerial);

enum STATS {
  idle,    // cancel
  getUser, // authenticate
  setUser, // register
} stat = getUser;

const byte ADDRESS[6] = "SPLYR";

PIN cePin           = 9, // rf24 cip enable
    csnPin          = 8, // rf24 cip select not
    incomingDataPin = 2; // rf24 interrupt signal
     
RF24 radio(cePin, csnPin);

struct Msg {
  uint8_t id, nr;
};

void receiveData() {
  Msg msg;
  radio.read (&msg, sizeof (msg));
  if (msg.id == 'S') {
    if (msg.nr == idle)                    {stat = idle;}
    if (stat == idle && msg.nr == getUser) {
      stat = getUser;
      Timer1.attachInterrupt (getFingerprintId);
    }
    if (stat == idle && msg.nr == setUser) {stat = setUser;}
    radio.stopListening();
    delay(3);
    radio.write (&msg, sizeof (msg));
    delay(3);
    radio.startListening();
  }
}

// returns -1 if failed, otherwise returns ID #
int getFingerprintIDez() {
  uint8_t p;
  if (finger.getImage()         != FINGERPRINT_OK
   || finger.image2Tz()         != FINGERPRINT_OK
   || finger.fingerFastSearch() != FINGERPRINT_OK
  ) {return -1;}
  return finger.fingerID; // found a match!
}

void getFingerprintId() {
  int id;
  if ((id = getFingerprintIDez()) == -1) return;
  Serial.print("Found ID #"); Serial.println(id); 
  Msg msg;
  msg.id = 'S';
  msg.nr = id & 0x80; // set the first bit to mark that this is a valid id 
  radio.stopListening();
  delay(3);
  radio.write (&msg, sizeof (msg));
  delay(3);
  stat = idle;
  radio.startListening();
  Timer1.detachInterrupt ();
}

void setup()  
{
  Timer1.initialize(50);
  finger.begin(57600);
  Serial.begin(9600);
  if (finger.verifyPassword()) {
    Serial.println("Found fingerprint sensor!");
  } else {
    Serial.println("Did not find fingerprint sensor");
    while (1) {delay(1);}
  }
  radio.begin();
  radio.stopListening();
  radio.openWritingPipe (ADDRESS);
  radio.openReadingPipe (1, ADDRESS);
  attachInterrupt (digitalPinToInterrupt (incomingDataPin), receiveData, LOW);
  radio.startListening();
  
  stat = getUser;
  Timer1.attachInterrupt (getFingerprintId);
}

void loop()                     // run over and over again
{
  return;
  switch (stat) {
    case idle:    delay (100);        break;
    case getUser: getFingerprintId(); break;
  }
}

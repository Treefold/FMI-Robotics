#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#define  PIN const uint8_t
#include <Adafruit_Fingerprint.h>
SoftwareSerial mySerial(6, 7);
Adafruit_Fingerprint finger = Adafruit_Fingerprint (&mySerial);
           
PIN cePin           = 9, // rf24 cip enable
    csnPin          = 8, // rf24 cip select not
    incomingDataPin = 2; // rf24 interrupt signal
     
RF24 radio(cePin, csnPin);

enum STATS {
  idle,    // cancel
  getUser, // authenticate
  setUser, // register
} stat = idle;

const byte address[6] = "SPLYR";

struct Msg {
  uint8_t id, nr;
};

void receiveData() {
  Msg msg;
  radio.read (&msg, sizeof (msg));
  if (msg.id == 'S') {
    Serial.print ((char) msg.id);
    Serial.println(msg.nr);   
    if (msg.nr == idle)                    {stat = idle;}
    if (stat == idle && msg.nr == getUser) {stat = getUser;}
    if (stat == idle && msg.nr == setUser) {stat = setUser;}
  }
}

// returns -1 if failed, otherwise returns ID #
int getFingerprintIDez() {
  return 7;
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
  radio.stopListening();
  Msg msg;
  msg.id = 1;//'S';
  msg.nr = 2;//id | 0x80; // set the first bit to mark that this is a valid id
  radio.openWritingPipe (address);
  radio.write (&msg, sizeof (msg));
  stat = idle;
  radio.openReadingPipe (1, address);
  radio.startListening();
}

void setup() {
  Serial.begin(9600);
  radio.printDetails();
  Serial.println("Waiting...");
  finger.begin(57600);
  delay(3);
  if (finger.verifyPassword()) {
    Serial.println("Found fingerprint sensor!");
  } else {
    Serial.println("Did not find fingerprint sensor");
    while (1) {delay(1);}
  }
  radio.begin();
  radio.stopListening();
  radio.openReadingPipe (1, address);
  attachInterrupt (digitalPinToInterrupt (incomingDataPin), receiveData, LOW);
  radio.startListening();
}

void loop() {
  switch (stat) {
    case idle:    delay (50);        break;
    case getUser: getFingerprintId(); break;
  }
}

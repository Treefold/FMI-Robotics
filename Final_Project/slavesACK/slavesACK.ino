#include <SPI.h>
#include "RF24.h"
#include <Adafruit_Fingerprint.h>
#define  PIN const uint8_t
PIN cePin           = 9, // rf24 cip enable
    csnPin          = 8, // rf24 cip select not
    fingerRxPin     = 7, // fingerprint sensor RX
    fingerTxPin     = 6, // fingerprint sensor TX
    incomingDataPin = 2; // rf24 interrupt signal
RF24 radio(cePin, csnPin);  // CE, CSN
SoftwareSerial mySerial(fingerTxPin, fingerRxPin);
Adafruit_Fingerprint finger = Adafruit_Fingerprint (&mySerial);
byte addresses[][6] = {"SSlve", "SMstr"};
struct Msg {uint8_t id, nr;} msg;
uint8_t readId = 10;

enum { // STATES
  idle,    // cancel
  getUser, // authenticate
  setUser // register
} stat = idle;

enum { // MESSAGES
  notStarted,
  working,
  finished
} progress = notStarted;

enum { // fingerprint enroll states
  setId,
  placee,
  removee,
  replacee
} enrollState = setId;

uint8_t result = 0;

// returns 0 if failed, otherwise returns ID #
int getFingerprintIDez() {
  if (finger.getImage()         != FINGERPRINT_OK
   || finger.image2Tz()         != FINGERPRINT_OK
   || finger.fingerFastSearch() != FINGERPRINT_OK
  ) {return 0;}
  return finger.fingerID; // found a match!
}

void getFingerprintId() {
  int id;
  if ((id = getFingerprintIDez()) > 0) {
    result   = id | 0x80; // set the first bit to mark that this is a valid id
    Serial.println (result);
    progress = finished;
  }
}

void receiveData() {
  radio.read (&msg, sizeof(Msg));
  if (msg.id == 'S') { 
    switch (msg.nr) {
      case idle: 
        stat = idle; 
        progress = notStarted;
        break;
      case getUser:
        if (stat == getUser) {
          msg.nr = (progress == finished) ? result : 0;
        }
        else {
          stat = getUser;
          progress = working;
        }
        break;
      case setUser: 
        if (stat == setUser) {
          msg.nr = (progress == finished) ? result : 0;
        }
        else {
          stat = setUser;
          progress = working;
        }
        break;
      default: msg.id = msg.nr = 0; break;
    }
  }
  else {msg.id = msg.nr = 0;}
  radio.writeAckPayload(1, &msg, sizeof(Msg));
}

void setFingerprintId() {
  switch (enrollState) {
    case placee:
      // Serial.print("Waiting for valid finger to enroll as #"); Serial.println(readId); // debug purpose
      if (finger.getImage()  != FINGERPRINT_OK) {return;}
      if (finger.image2Tz(1) != FINGERPRINT_OK) {return;}
      enrollState = removee;
    // no break
    case removee:
      //Serial.println("Remove finger"); // debug purpose
      if (finger.getImage() != FINGERPRINT_NOFINGER) {return;}
      enrollState = replacee;
    // no break;
    case replacee:
      // Serial.println ("Place same finger again"); // debug purpose
      if (finger.getImage()  != FINGERPRINT_OK) {return;}
      if (finger.image2Tz(2) != FINGERPRINT_OK) {
        enrollState = placee;
        return;
      }
      if (finger.createModel()      == FINGERPRINT_OK
       && finger.storeModel(readId) == FINGERPRINT_OK){
        result = readId | 0x80;
        progress = finished; 
        // Serial.println ("Done"); // debug purpose
      }
    // no break;
    default: enrollState = placee; break;
  }  
}

void setup() {
  Serial.begin(9600);
  finger.begin(57600);
  delay(3);
  if (finger.verifyPassword()) {
    Serial.println("Found fingerprint sensor!");
  } else {
    Serial.println("Did not find fingerprint sensor");
    while (1) {delay(1000);}
  }
  
  radio.begin();
  radio.enableAckPayload();
  radio.enableDynamicPayloads();
  radio.openWritingPipe(addresses[1]);
  radio.openReadingPipe(1, addresses[0]);
  attachInterrupt (digitalPinToInterrupt (incomingDataPin), receiveData, LOW);
  radio.startListening();
}
void loop(void) {
  if (stat == idle || progress == finished) {delay(50);}
  else {
    switch (stat) {
      case getUser: getFingerprintId(); break;
      case setUser: setFingerprintId(); break;
      default:      stat = idle;        break;
    }
  }
}

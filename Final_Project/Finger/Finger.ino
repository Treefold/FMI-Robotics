#include <SPI.h>
#include "RF24.h"
#include <Adafruit_Fingerprint.h>
#include <Keypad.h>
#define  PIN const uint8_t
const byte ROWS = 4; //four rows
const byte COLS = 3; //three columns
char numberKeys[ROWS][COLS] = {
    { '1','2','3' },
    { '4','5','6' },
    { '7','8','9' },
    { '*','0','#' }
};
uint8_t rowPins[ROWS] = {A0, A1, A2, A3}; //connect to the row pinouts of the keypad
uint8_t colPins[COLS] = {A4, A5, A6}; //connect to the column pinouts of the keypad
Keypad numpad( makeKeymap(numberKeys), rowPins, colPins, sizeof(rowPins), sizeof(colPins) );

PIN cePin           = 8, // rf24 cip enable
    csnPin          = 9, // rf24 cip select not
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

enum {
  getId,
  place
} loginState = getId;

enum { // fingerprint enroll states
  setId,
  placee,
  removee,
  replacee
} enrollState = setId;

uint8_t result = 0;
char key;
uint16_t num = 0;

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
  if ((id = getFingerprintIDez()) == num) {
    progress = finished;
  }
  if (id>0) {Serial.println (id);}
}

void receiveData() {
  radio.read (&msg, sizeof(Msg));
  if (msg.id == 'S') { 
    switch (msg.nr) {
      case idle: 
        stat     = idle; 
        progress = notStarted;
        msg.nr   = 0x80;
        break;
      case getUser:
        if (stat == getUser) {
          msg.nr = ((progress == finished) ? 0x80 : 0) | num;
        }
        else {
          stat       = getUser;
          loginState = getId;
          msg.nr     = 0;
          num        = 0;
          progress   = working;
        }
        break;
      case setUser: 
        if (stat == setUser) {
          msg.nr = ((progress == finished) ? 0x80 : 0) | num;
        }
        else {
          stat        = setUser;
          enrollState = setId;
          msg.nr      = 0;
          num         = 0;
          progress    = working;
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
    case setId:
      key = numpad.getKey(); break;
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
        result = num | 0x80;
        progress = finished; 
        // Serial.println ("Done"); // debug purpose
      }
    // no break;
    default: enrollState = placee; break;
  }  
}

void keypadEvent_num(KeypadEvent key) {
    swOnState (key);
}

void swOnState( char key ) {
    switch(numpad.getState()) {
        case PRESSED:
          if (isdigit(key)) {
            num = num* 10 + key - '0';
            if (num > 39) {num = 0;}
            Serial.println (num);
          }
          break;
        case HOLD:
          if (key == '*') {
            if (num == 0) break;
            if (stat == getUser) {loginState  = place;}
            if (stat == setUser) {enrollState = placee;}
          }
          break;
        default: break;
    }
}

void setup() {
  Serial.begin(9600);
  numpad.begin( makeKeymap(numberKeys) );
  numpad.addEventListener(keypadEvent_num);
  numpad.setHoldTime(500);            

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
      case getUser: 
        switch(loginState) {
          case getId: key = numpad.getKey(); break;
          case place: getFingerprintId();    break;
          default: loginState = getId;       break;
        }
        break;
      case setUser: setFingerprintId(); break;
      default:      stat = idle;        break;
    }
  }
}

#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"
#define  PIN const uint8_t
PIN cePin           = 9, // rf24 cip enable
    csnPin          = 8, // rf24 cip select not
    incomingDataPin = 2; // rf24 interrupt signal
// too many pins to declare each of them as a define
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  
RF24 radio(cePin, csnPin);  // CE, CSN
byte addresses[][6] = {"SSlve", "SMstr"};
struct Msg {uint8_t id, nr;} msg;

void setup() {
  Serial.begin(9600);
  lcd.begin(16,2); 
  lcd.clear(); 
  radio.begin();
  radio.enableAckPayload();
  radio.enableDynamicPayloads();
  radio.stopListening();
  radio.openWritingPipe(addresses[0]);
  radio.openReadingPipe(1, addresses[1]);
}

enum {
  logIn,
  selectGame,
  inGame,
  details
} masterState = logIn;

enum {
  
}

void mesage() {
  msg.id = 'S';
    msg.nr = 1;
    if ( radio.write(&msg, sizeof(Msg)) ) { 
      
        while (radio.available() ) {                    // If an ack with payload was received
          radio.read( &msg, sizeof(Msg));                  // Read it, and display the response time
          unsigned long timer = micros();

          Serial.print(F("Got response "));
          Serial.println(msg.nr);                                 // Increment the counter variable
        }
      

    } else {
      Serial.println(F("Sending failed."));  // If no ack response, sending failed
    }
}

void doLogIn() {
  lcd.setCursor(0, 0);
  lcd.print ("   Guest   LogIn");
  lcd.setCursor(0, 1);
  lcd.print ("   CreateNew    ");
  
}

void loop(void) {
  switch(masterState) {
    case logIn: doLogIn();
      
  }
}

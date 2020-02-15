#include <SPI.h>
#include "RF24.h"
#define  PIN const uint8_t
PIN cePin           = 9, // rf24 cip enable
    csnPin          = 8, // rf24 cip select not
    incomingDataPin = 2; // rf24 interrupt signal
RF24 radio(cePin, csnPin);  // CE, CSN
byte addresses[][6] = {"SSlve", "SMstr"};
struct Msg {uint8_t id, nr;} msg;
                                           
byte counter = 1; 

void setup() {
  Serial.begin(9600);
  radio.begin();
  radio.enableAckPayload();
  radio.enableDynamicPayloads();
  radio.openWritingPipe(addresses[0]);
  radio.openReadingPipe(1, addresses[1]);
  radio.startListening();
}

void loop(void) {
    uint16_t gotByte;                                           // Initialize a variable for the incoming response

    radio.stopListening();                                  // First, stop listening so we can talk.
    Serial.print(F("Now sending "));                         // Use a simple byte counter as payload
    Serial.println(counter);

    unsigned long time = micros();                          // Record the current microsecond count
    msg.id = 6;
    msg.nr = counter;
    if ( radio.write(&msg, sizeof(Msg)) ) {                       // Send the counter variable to the other radio
      if (!radio.available()) {                           // If nothing in the buffer, we got an ack but it is blank
        Serial.print(F("Got blank response. round-trip delay: "));
        Serial.print(micros() - time);
        Serial.println(F(" microseconds"));
      } else {
        while (radio.available() ) {                    // If an ack with payload was received
          radio.read( &msg, sizeof(Msg));                  // Read it, and display the response time
          unsigned long timer = micros();

          Serial.print(F("Got response "));
          Serial.print(msg.id * 1000 + msg.nr);
          Serial.print(F(" round-trip delay: "));
          Serial.print(timer - time);
          Serial.println(F(" microseconds"));
          counter++;                                  // Increment the counter variable
        }
      }

    } else {
      Serial.println(F("Sending failed."));  // If no ack response, sending failed
    }

    delay(1000);  // Try again later
}

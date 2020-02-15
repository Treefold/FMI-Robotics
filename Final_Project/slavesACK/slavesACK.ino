#include <SPI.h>
#include "RF24.h"
#define  PIN const uint8_t
PIN cePin           = 9, // rf24 cip enable
    csnPin          = 8, // rf24 cip select not
    incomingDataPin = 2; // rf24 interrupt signal
RF24 radio(cePin, csnPin);  // CE, CSN
byte addresses[][6] = {"SSlve", "SMstr"};
struct Msg {uint8_t id, nr;} msg;

void receiveData() {
    radio.read (&msg, sizeof(Msg));
    // Since this is a call-response. Respond directly with an ack payload.
    msg.nr += 1;                                // Ack payloads are much more efficient than switching to transmit mode to respond to a call
    radio.writeAckPayload(1, &msg, sizeof(Msg)); // This can be commented out to send empty payloads.
    Serial.print(F("Loaded next response "));
    Serial.println(msg.id*1000 + msg.nr);
}

void setup() {
  Serial.begin(9600);
  Serial.println ("Listening...");
  radio.begin();
  radio.enableAckPayload();
  radio.enableDynamicPayloads();

  radio.openWritingPipe(addresses[1]);
  radio.openReadingPipe(1, addresses[0]);

  attachInterrupt (digitalPinToInterrupt (incomingDataPin), receiveData, LOW);

  radio.startListening();
}
void loop(void) {
}

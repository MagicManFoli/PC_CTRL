
#include <Arduino.h>

#include "Codes.h"
// http://tmrh20.github.io/RF24/
#include <RF24.h>
#include <SPI.h>

// Setup for RF24 Codes
const uint8_t PIN_CE = 7;
const uint8_t PIN_CS = 8;
/* SETUP
NRF --> Arduino
GND -> GND
VCC -> 3.3V (+ 10uF)
CE -> 7
CS -> 8
SCK -> 13
MOSI -> 11
MISO -> 12
IRQ -> ??
*/

/* http://forum.arduino.cc/index.php?topic=421081
  2.4Ghz, default: Channel 76
  pipes determine receiver channel
  pipe 0 always sending, pipe 1-5 allow parallel receiving

  radio interference possible, but unlikely and controlled by Library
  default: 5 tries to send, up to 15x possible

  radio.avaiable() already has complete messages, no need to wait

  two_way possible, but difficult
  ackPayload eases operation and handling, but only allows up to 32 bytes in parallel
  ackPayLoad causes slave to always be one message behind master

  possible additions:
  http://tmrh20.github.io/RF24Mesh/
  http://tmrh20.github.io/RF24Network/
*/

// SPI +
RF24 radio(PIN_CE, PIN_CS);

//bool role = 0;    //send = 1, receive = 0


void setup(){
  Serial.begin(9600);
  Serial.println(F("RF_Test starting..."));

  radio.begin();
  radio.stopListening();

  //low power because of small power supply and close proximity
  radio.setPALevel(RF24_PA_LOW);

  // low data rate for better range
  radio.setDataRate(RF24_250KBPS);

  // Use dynamic payloads to improve response time
  radio.enableDynamicPayloads();

  // set pipes for communication;
  //  avoid pipe 0 http://maniacalbits.blogspot.de/2013/04/rf24-avoiding-rx-pipe-0-for-enhanced.html

  //### wtf??
  //radio.openWritingPipe("SAMI");    // what does this address do?

  radio.startListening();
  Serial.println(F("Ready for commands!"));
}

bool toggle = 0;

void loop(){
  Serial.println("Sending Test Command");
  toggle = !toggle;

  //transmit toggle via broadcast


  // transmit (BROADCAST, GENERIC, BLINK, 3)
}

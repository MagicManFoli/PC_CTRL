
#include <Arduino.h>

#include <avr/interrupt.h>
#include <avr/sleep.h>

#include "Codes.h"
// http://tmrh20.github.io/RF24/

#include <RF24Network.h>
#include <RF24.h>
#include <SPI.h>


/* -- edit in config-h --
//payload-size in byte
#define MAX_PAYLOAD_SIZE 16
#define DISABLE_FRAGMENTATION
#define RF24NetworkMulticast
#define ENABLE_SLEEP_MODE
*/

// Setup for RF24 Codes
const uint8_t PIN_CE = 7;
const uint8_t PIN_CS = 8;
/* SETUP
NRF --> Arduino
---------------
GND -> GND
VCC -> 3.3V (+ >10uF | 47uF)
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

  radio.available() already has complete messages, no need to wait

  two_way possible, but difficult
  ackPayload eases operation and handling, but only allows up to 32 bytes in parallel
  ackPayLoad causes slave to always be one message behind master

  possible additions:
  http://tmrh20.github.io/RF24Mesh/
  http://tmrh20.github.io/RF24Network/

  255 Connections could be possible, but difficult
  https://www.insidegadgets.com/2013/06/09/nrf24-multi-network-allowing-for-255-addresses/

  -- Network --

  Each Node can only connect to 6 other Nodes, tree struucture needed
  -> 6 children for root, after that 1 parent and 5 children

  https://tmrh20.github.io/RF24Network/helloworld_tx_8ino-example.html
  https://tmrh20.github.io/RF24Network/helloworld_rx_8ino-example.html

  In RF24Network, the master is just 00
  Children of master are 01,02,03,04,05
  Children of 01 are 011,021,031,041,051

  multicastLevel
  setup_watchdog  -> max 8s
  sleepNode
  is_valid_address
  multicastRelay  -> enable forwarding
  available
  read/peek

*/

// SPI +
RF24 radio(PIN_CE, PIN_CS);

RF24Network network(radio);

const uint16_t this_node = 01;    //first child, this node
const uint16_t main_node = 00;    //master Node
const uint16_t other_node = 02;   //second child, other Node

void setup(){
  Serial.begin(9600);
  Serial.println(F("RF_Test starting..."));

  SPI.begin();    //start for radio ##needed?

  if (!radio.begin()){    //failed init, continued use will do weird stuff to network code
    Serial.println(F("No response from module"));
    delay(1000);  // wait for... stuff...
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    cli();  //disable interrupts
    sleep_mode();
  }
  //-- network --
  network.begin(90, this_node);   // no idea where the 90 is from

  Serial.println(F("Setup finished."));
}

bool toggle = 0;

uint32_t last_send = 0;
uint32_t last_blink = 0;
uint32_t now;
uint32_t interval = 4000; //ms
bool success;

RF24NetworkHeader header(other_node);   //generate header for transmission

void loop(){
  network.update();   //call often to enable network features

  now = millis();

  if ( now - last_blink >= 1000){
    last_blink = now;
    Serial.println(F("Toggle"));
    toggle = !toggle;

    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
  }

  if ( now - last_send >= interval){
    last_send = now;
    Serial.println(F("Sending Test Command"));

    cmd payload = {this_node, 0, 0, 0};   //test payload

    success = network.write(header, &payload, sizeof(payload));
    if (success == true) Serial.println("Transmission was successful");
    else Serial.println("Transmission failed");
  }

  // transmit (BROADCAST, GENERIC, BLINK, 3)
}

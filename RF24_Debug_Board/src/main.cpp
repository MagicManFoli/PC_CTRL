
#include <Arduino.h>

#include <avr/interrupt.h>
#include <avr/power.h>
#include <avr/sleep.h>
#include <avr/io.h>

#include "Codes.h"

//LowPower.h     //.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);

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

const uint8_t PIN_LED_1 = 3;
const uint8_t PIN_LED_2 = 4;
const uint8_t PIN_BT_1 = 5;
const uint8_t PIN_BT_2 = 6;


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
IRQ -> D2

LED1 -> D3
LED2 -> D4
BT1 -> D5
BT2 -> D6

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

const uint16_t this_node = CMD::NODE::TESTER;    // 01 OCTAL
const uint16_t other_node = CMD::NODE::PC_NODE;   // 02

// struct cmd   //forward declaration?

volatile cmd_payload payload{(uint16_t) -1,(uint16_t) -1, 0, 0, 0};
//volatile RF24NetworkHeader header;

/* -----------  Functions ------------------- */

volatile bool toggle = 0;    //volatile because of interrupt modification
volatile bool changed = 0;    //not needed if payload is modified

const uint16_t t_deb = 500; //ms
volatile uint32_t last = 0;
volatile uint8_t PORTD_HIST = 0xFF;
/*
// ## toggle on interrupt by PIN_BT_1
ISR(PCINT2_vect){    //toggle
  //##debounce

  if (PIND & (1 << 5)){
    Serial.println(F("ISR_toggle"));
    toggle = !toggle;
    digitalWrite(PIN_LED_1, toggle);
  }
  else if (PIND & (1 << 6)){
    Serial.println(F("Sending"));
    // set payload != 0 to signal loop()
    header = header(other_node);   //generate header for transmission
    payload = {this_node, CMD::GENERIC, CMD::SET_LED, toggle};  //switch other led depending on own status
  }
}
*/

void enable_interrupts(){
  cli();
  // pin change interrupt (example for D4)
  PCMSK2 |= bit (PCINT21);  // want pin 5
  PCMSK2 |= bit (PCINT22);  // want pin 6
  PCIFR  |= bit (PCIF2);    // clear any outstanding interrupts
  PCICR  |= bit (PCIE2);    // enable pin change interrupts for D0 to D7

  sei();
}

ISR (PCINT2_vect){  //PORTD
  uint8_t changedbits;

  changedbits = PIND ^ PORTD_HIST;    //pins that are different
  PORTD_HIST = PIND;    //update changed pins

  uint32_t now = millis();    //won't increment in function but current is ok
  if ((now - last) < t_deb){  //quick changes, still bouncing
    last = now;
    return;
  }
  //first execution is always right
  last = now;

  if (changedbits & (1 << PIND5)){  //pin is changed pin
    if (digitalRead(PIN_BT_1) == false){
      changed = true;
      toggle = !toggle;
      digitalWrite(PIN_LED_1, toggle);
    } //else button release, should be ignored
  }
  if (changedbits & (1 << PIND6)){
    if (digitalRead(PIN_BT_2) == false){
      payload = (cmd_payload){this_node, other_node, CMD::GENERIC, CMD::SET_LED, toggle};  //switch other led depending on own status
    }
  }
}

void sleep(){
  // Serial -> going to sleep

  set_sleep_mode(SLEEP_MODE_PWR_DOWN);    // stand by possible for fast responses

  sleep_enable();
  sleep_bod_disable();
  //## turn off ADC ...
  //sei();
  sleep_mode();   //goes to sleep here

  // [SLEEPING HERE]

  sleep_disable();    //wakes up here
}

void print_payload(volatile cmd_payload &pl){
    Serial.println(F("-- Payload is: --"));
    Serial.print(F("\n From Node: "));
    Serial.print(pl.from_node);
    Serial.print(F("\n To Node: "));
    Serial.print(pl.to_node);
    Serial.print(F("\n category: "));
    Serial.print(pl.category);
    Serial.print(F("\n function: "));
    Serial.print(pl.function);
    Serial.print(F("\n parameter: "));
    Serial.print(pl.parameter);
    Serial.println(F("\n-----------------"));
}

void setup(){
  noInterrupts();

  // pullup unused pins (consumes less power)
  for (byte i=0; i<20; i++) {    //make all pins inputs with pullups enabled
      pinMode(i, INPUT_PULLUP);
  }

  Serial.begin(9600);
  Serial.println(F("RF_Debug_Board starting..."));

  pinMode(PIN_BT_1, INPUT_PULLUP);    // switch pulls down
  pinMode(PIN_BT_2, INPUT_PULLUP);
  pinMode(PIN_LED_1, OUTPUT);
  pinMode(PIN_LED_2, OUTPUT);

  SPI.begin();    //start for radio ##needed?

  if (!radio.begin()){    //failed init, continued use will do weird stuff to network code
    Serial.println(F("No response from module"));
    //delay(1000);  // wait for... stuff...
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    cli();  //disable interrupts
    Serial.flush();
    sleep_mode();
  }

  //## modify radio if possible
  // lower power & range

  network.begin(90, this_node);   // ## no idea where the 90 is from

  enable_interrupts();

  // -- done --

  Serial.println(F("Setup finished."));
}


void loop(){
  network.update();   //call often to enable network features

  if (payload.to_node != (uint16_t)-1) {   //interrupt routine modified; easier than 2^16 - 1
    Serial.println(F("Payload was modified, transmitting again"));
    print_payload(payload);

    RF24NetworkHeader header(payload.to_node);
    bool success = network.write(header, &payload, sizeof(payload));

    if (success == true) Serial.println("Transmission was successful");
    else Serial.println("Transmission failed");

    //reset variables

    payload.to_node = (uint16_t)-1; //payload is invalid again; easier than 2^16 - 1

    Serial.println(F("Payload was reset"));
    print_payload(payload);
  }
  // transmit (BROADCAST, GENERIC, BLINK, 3)
}

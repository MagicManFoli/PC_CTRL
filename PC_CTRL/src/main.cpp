
#include <Arduino.h>

#include <avr/interrupt.h>
#include <avr/sleep.h>

#include "HomeNet.h"

// http://tmrh20.github.io/RF24/
#include <RF24Network.h>
#include <RF24.h>
#include <SPI.h>

/** TODO
- LED not always responding to received payload
- manual activation via button triggers on both flanks, should be one

**/

// -------- globals & startup -------------------

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
IRQ -> 2
*/

// Setup for RF24 Codes
const uint8_t PIN_CE = 7;
const uint8_t PIN_CS = 8;

// test battery/start PC (?)
const uint8_t PIN_IRQ = 2;
const uint8_t PIN_BT = 3;   //inverted

// blink when transmission was received
const uint8_t PIN_LED = A4; //inverted
const uint8_t PIN_PC = A5;  //inverted


// SPI +
RF24 radio(PIN_CE, PIN_CS);
RF24Network network(radio);

const uint16_t this_node = static_cast<uint16_t>(HomeNet::NODE::PC_NODE);    //first child, this node
const uint16_t main_node = static_cast<uint16_t>(HomeNet::NODE::TESTER);    //master Node

//volatile cmd_payload payload{(uint16_t) -1,(uint16_t) -1, 0, 0, 0};
HomeNet home;

const uint16_t sleep_delay = 30000; //10 s

uint32_t next_reset = 0;
uint32_t last_action = 0;

// ---------------- functions --------------

void PC_CTRL_SET(uint8_t duration){
  digitalWrite(PIN_LED, false);
  digitalWrite(PIN_PC, false);

  next_reset = millis() + (duration*100);

  // ... wait for reset
}

void PC_CTRL_RESET(){
  // .. after set

  next_reset = 0;

  digitalWrite(PIN_PC, true);
  digitalWrite(PIN_LED, true);
}

void SET_LED(uint8_t status){
  digitalWrite(PIN_LED, !status);
}

void sleep(){
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);    // stand by possible for fast responses

  sleep_enable();         // allow power-down
  sleep_bod_disable();    //turn off voltage monitoring
  //## turn off ADC ...
  // ## switch radio to low-power

  sei();
  sleep_mode();   //goes to sleep here

  // [SLEEPING HERE]

  sleep_disable();    //forbid sleep
}

void enable_interrupts(){ //EDIT
  //cli();
  //PCMSK2 |= bit (PCINT18);  // want pin 2, IRQ
  //PCMSK2 |= bit (PCINT19);  // want pin 3, BT

  EICRA |= bit (ISC11) | bit (ISC01);  // falling edge  ### seems to trigger on both
  EIMSK |= bit (INT0) | bit(INT1);

  sei();
}

// ISR to wake up from NRF
// ...

ISR(INT0_vect){
  // woke up from sleep, nothing to do
}

ISR(INT1_vect){
  PC_CTRL_SET(10);
}

// ------------------- main ----------------

void setup(){
  cli();

  // pullup unused pins (consumes less power)
  for (byte i=0; i<20; i++) {    //make all pins inputs with pullups enabled
      pinMode(i, INPUT_PULLUP);
  }

  Serial.begin(9600);
  Serial.println(F("PC_CTRL starting..."));

  pinMode(PIN_BT, INPUT_PULLUP);    // switch pulls down
  pinMode(PIN_IRQ, INPUT_PULLUP);    // interrupt from NRF

  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_PC, OUTPUT);

  digitalWrite(PIN_LED, false);   // set to on
  digitalWrite(PIN_PC, true);    // set to off

  SPI.begin();    //start for radio ##needed?

  if (!radio.begin()){    //failed init, continued use will do weird stuff to network code
    Serial.println(F("No response from module"));
    //delay(1000);  // wait for... stuff...
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    cli();  //disable interrupts
    Serial.flush();
    sleep_mode();
  }

  // ## SETUP radio
  // low power
  // IRQ only on received


  // add entries for dictionary
  home.add(static_cast<uint8_t>(HomeNet::CATEGORY::GENERIC),
    static_cast<uint8_t>(HomeNet::GENERIC::SET_LED), SET_LED);
  home.add(static_cast<uint8_t>(HomeNet::CATEGORY::PC_CTRL),
    static_cast<uint8_t>(HomeNet::PC_CTRL::SET), PC_CTRL_SET);


  //-- network --
  network.begin(90, this_node);   // no idea where the 90 is from

  Serial.println(F("Setup finished."));
  Serial.flush();

  digitalWrite(PIN_LED, true);

  enable_interrupts();
}

void loop(){

  uint32_t now = millis();

  // network handling
  network.update();   //call often to enable network features

  while (network.available()){
    Serial.println(F("Message available"));
    HomeNet::payload load;
    RF24NetworkHeader header;

    network.read(header, &load, sizeof(load));

    HomeNet::print_payload(load);

    home.translate(load);
    last_action = now;
  }

  // RESET logic for PC_CTRL
  if ( next_reset != 0 && now >= next_reset){
    Serial.println(F("reset PC_CTRL"));
    PC_CTRL_RESET();
    last_action = now;
  }

  //sleep logic

  if ((last_action + sleep_delay) <= now){
    Serial.println(F("Going to sleep"));
    Serial.flush();

    sleep();

    // [SLEEPING]

    Serial.println(F("Woke up"));
    last_action = now;
  }

}


#include <Arduino.h>

#include <avr/interrupt.h>
#include <avr/sleep.h>

#include "HomeNet.h"

// http://tmrh20.github.io/RF24/
#include <RF24Network.h>
#include <RF24.h>
#include <SPI.h>

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
IRQ -> ??
*/

// Setup for RF24 Codes
const uint8_t PIN_CE = 7;
const uint8_t PIN_CS = 8;

// test battery/start PC (?)
const uint8_t PIN_BT = 3;

// blink when transmission was received
const uint8_t PIN_LED = A4;
const uint8_t PIN_PC = A5;


// SPI +
RF24 radio(PIN_CE, PIN_CS);
RF24Network network(radio);

const uint16_t this_node = static_cast<uint16_t>(HomeNet::NODE::PC_NODE);    //first child, this node
const uint16_t main_node = static_cast<uint16_t>(HomeNet::NODE::TESTER);    //master Node

//volatile cmd_payload payload{(uint16_t) -1,(uint16_t) -1, 0, 0, 0};


// ---------------- functions --------------

void translate(HomeNet::payload load){
  if (load.category == static_cast<uint8_t>(HomeNet::CATEGORY::GENERIC)){
    if (load.function == static_cast<uint8_t>(HomeNet::GENERIC::SET_LED)){
      Serial.println(F("Writing new LED status"));
      digitalWrite(PIN_LED, (bool)load.parameter);
      return;
    }
  }

  if (load.category == static_cast<uint8_t>(HomeNet::CATEGORY::PC_CTRL)){
    if (load.function == static_cast<uint8_t>(HomeNet::PC_CTRL::SET)){
      Serial.println(F("Writing new CTRL status"));
      digitalWrite(PIN_PC, (bool)load.parameter);
      return;
    }
  }
}

// ISR to wake up from NRF
// ...

void sleep(){
  // Serial -> going to sleep

  set_sleep_mode(SLEEP_MODE_PWR_DOWN);    // stand by possible for fast responses

  sleep_enable();         // allow power-down
  sleep_bod_disable();    //turn off voltage monitoring
  //## turn off ADC ...
  // ## switch radio to low-power

  //sei();
  sleep_mode();   //goes to sleep here

  // [SLEEPING HERE]

  sleep_disable();    //forbid sleep
}

void enable_interrupts(){ //EDIT
  cli();
  // pin change interrupt (example for D4)
  PCMSK2 |= bit (PCINT21);  // want pin 5
  PCMSK2 |= bit (PCINT22);  // want pin 6
  PCIFR  |= bit (PCIF2);    // clear any outstanding interrupts
  PCICR  |= bit (PCIE2);    // enable pin change interrupts for D0 to D7

  sei();
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
  pinMode(PIN_LED, OUTPUT);

  digitalWrite(PIN_LED, false);

  SPI.begin();    //start for radio ##needed?

  if (!radio.begin()){    //failed init, continued use will do weird stuff to network code
    Serial.println(F("No response from module"));
    //delay(1000);  // wait for... stuff...
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    cli();  //disable interrupts
    Serial.flush();
    sleep_mode();
  }
  //-- network --
  network.begin(90, this_node);   // no idea where the 90 is from

  Serial.println(F("Setup finished."));
  Serial.flush();

  digitalWrite(PIN_LED, true);


  sei();
  //enable_interrupts(); FIT TO CODE FIRST
}

uint32_t last_blink = 0;
uint32_t now;
const uint32_t interval = 2000; //ms

void loop(){
  network.update();   //call often to enable network features

  now = millis();

  while (network.available()){
    HomeNet::payload load;
    RF24NetworkHeader header;

    network.read(header, &load, sizeof(load));

    HomeNet::print_payload(load);


    translate(load);

  }


/*
  if ( (now - last_blink) >= interval){
    last_blink = now;
    Serial.println(F("Toggle"));

    digitalWrite(PIN_LED, !digitalRead(PIN_LED));
  }
*/
}

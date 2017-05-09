#include <Arduino.h>
#include <stdint.h>

#include <avr/interrupt.h>

//#include <EnableInterrupt.h>

const uint8_t PIN_LED_1 = 3;
const uint8_t PIN_LED_2 = 4;  //not used
const uint8_t PIN_BT_1 = 5;
const uint8_t PIN_BT_2 = 6;  //not used

const uint32_t t_deb = 50;    //ms

volatile bool changed = false;
volatile bool toggle = false;
volatile uint32_t last;

void enable_interrupts(){

  // pin change interrupt (example for D4)
  PCMSK2 |= bit (PCINT21);  // want pin 5
  PCMSK2 |= bit (PCINT22);  // want pin 6
  PCIFR  |= bit (PCIF2);    // clear any outstanding interrupts
  PCICR  |= bit (PCIE2);    // enable pin change interrupts for D0 to D7

  pinMode(PIN_BT_1, INPUT_PULLUP);    //pullup or external pulldown
  pinMode(PIN_BT_2, INPUT_PULLUP);

  // enable interrupts?
}

//any pin change detected, possible start of bouncing
ISR (PCINT2_vect){
  // ## check for type/pin
  // u16 = & 0FFF possible, top word irrelevant for small timing
  uint32_t now = millis();    //won't increment in function
  if (((now - last) > t_deb) && (digitalRead(PIN_BT_1) == false)){  //ignore degitalRead?
    changed = true;   //only for loop to check for changes
    toggle = !toggle;
  }
  last = now;
}


void setup(){
  Serial.begin(9600);
  pinMode(PIN_BT_1, INPUT_PULLUP);
  pinMode(PIN_LED_1, OUTPUT);

  //enableInterrupt(PIN_BT_1, ISR_toggle, FALLING);
  enable_interrupts();
}

void loop(){
  if (changed){
    changed = false;
    Serial.println(F("ISR_toggle"));
    digitalWrite(PIN_LED_1, toggle);
  }
}

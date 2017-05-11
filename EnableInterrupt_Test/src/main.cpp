/* ------------------------------- *\
Low power interrupt sleep test for 328P

button press to toggle LED1

clock: internal RC osz 8MHz
BOD: disabled

Current Tests:
 - #TODO# -

\* ------------------------------- */

#include <Arduino.h>
#include <stdint.h>

#include <avr/interrupt.h>
#include <avr/power.h>
#include <avr/sleep.h>

#include <avr/io.h>


const uint8_t PIN_LED_1 = 3;
const uint8_t PIN_LED_2 = 4;  //not used
const uint8_t PIN_BT_1 = 5;
const uint8_t PIN_BT_2 = 6;  //not used

//wait this long for additional input before sleeping again
uint32_t t_wait = 10000;  //ms

// -- ISR
const uint32_t t_deb = 50;    //ms
volatile uint32_t last;
volatile uint8_t PORTD_HIST = 0xFF;
// --

volatile bool changed = false;
volatile bool toggle = false;

void enable_interrupts(){
  cli();
  // pin change interrupt (example for D4)
  PCMSK2 |= bit (PCINT21);  // want pin 5
  PCMSK2 |= bit (PCINT22);  // want pin 6
  PCIFR  |= bit (PCIF2);    // clear any outstanding interrupts
  PCICR  |= bit (PCIE2);    // enable pin change interrupts for D0 to D7

  pinMode(PIN_BT_1, INPUT_PULLUP);    //pullup or external pulldown
  pinMode(PIN_BT_2, INPUT_PULLUP);

  sei();
}

//any pin change detected, possible start of bouncing
ISR (PCINT2_vect){  //PORTD

  // - decode pin that changed
  uint8_t changedbits;

  changedbits = PIND ^ PORTD_HIST;    //pins that are different
  PORTD_HIST = PIND;    //update changed pins

  // - check if still bouncing

  // u16 = & 0FFF possible, top word irrelevant for small timing  (but overflow after 65s possible)
  uint32_t now = millis();    //won't increment in function but current is ok
  if ((now - last) < t_deb){  //quick changes, still bouncing
    last = now;
    return;
  }

  // - should be stable now, can start analysis

  if (changedbits & (1 << PIND5)){  //pin is changed pin
    if (digitalRead(PIN_BT_1) == false){
      changed = true;
      toggle = !toggle;
    }
  }
  if (changedbits & (1 << PIND6)){
    // second button
  }
}

void sleep(){
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);    // stand by possible for fast responses
  //cli();    //optional, prevents problems with BOD setup
  ADCSRA = 0;   //apperently turns off ADC (already done below?)

  /*  // Test these
  power_adc_disable(); // ADC converter
  power_spi_disable(); // SPI
  power_usart0_disable();// Serial (USART)
  power_timer0_disable();// Timer 0
  power_timer1_disable();// Timer 1
  power_timer2_disable();// Timer 2
  power_twi_disable(); // TWI (I2C)
  */

  sleep_enable();
  sleep_bod_disable();
  //## turn off ADC ...
  //sei();
  sleep_mode();

  sleep_disable();    //wakes up here
}


void setup(){
  // only executed once, waking up does not run setup

  // pullup unused pins (consumes less power)
  for (byte i=0; i<20; i++) {    //make all pins inputs with pullups enabled
      pinMode(i, INPUT_PULLUP);
  }

  Serial.begin(9600);
  pinMode(PIN_BT_1, INPUT_PULLUP);
  pinMode(PIN_LED_1, OUTPUT);

  digitalWrite(PIN_LED_1, HIGH);  //inverted logic
  //enableInterrupt(PIN_BT_1, ISR_toggle, FALLING);
  enable_interrupts();

  Serial.println(F("Setup done"));
}

uint32_t last_activity = 0;

void loop(){
  uint32_t now = millis();


  if (changed){   //Serial is not allowed in ISR, so values should be checked externally
    changed = false;
    Serial.println(F("ISR_toggle"));
    digitalWrite(PIN_LED_1, toggle);

    last_activity = now;
  }

  // - sleep if nothing happened

  if ( (now - last_activity) > t_wait){
    Serial.print(F("Waited for "));
    Serial.print(String(t_wait));
    Serial.print(F(" ms, now going to sleep \n"));

    Serial.flush();   //wait for data to be send

    sleep();

    // -------------
    // sleeping until external interrupt fires
    // interupt code after waking up
    // --------------

    Serial.println(F("Awake again"));
    last_activity = now;
  }
}

#include "userinput.h"

#include "Arduino.h"

// PCINT20
#define USERINPUT_PIN_A       PD4
// PCINT8
#define USERINPUT_PIN_B       A0
// PCINT0
#define USERINPUT_PIN_CLICK   8

#define USERINPUT_BIT_LEFT    0
#define USERINPUT_BIT_RIGHT   1
#define USERINPUT_BIT_CLICK   2

volatile uint8_t userinput_flags = 0;

// A
ISR(PCINT2_vect) {
  if (digitalRead(USERINPUT_PIN_A) == LOW) {
    userinput_flags |= _BV(USERINPUT_BIT_LEFT);
  }
}

// B
ISR(PCINT1_vect) {
  if (digitalRead(USERINPUT_PIN_B) == LOW) {
    userinput_flags |= _BV(USERINPUT_BIT_RIGHT);
  }
}

// CLK
ISR(PCINT0_vect) {
  if (digitalRead(USERINPUT_PIN_CLICK) == LOW) {
    userinput_flags |= _BV(USERINPUT_BIT_CLICK);
  }
}

void userinput_init() {
  pinMode(USERINPUT_PIN_A, INPUT_PULLUP);
  pinMode(USERINPUT_PIN_B, INPUT_PULLUP);
  pinMode(USERINPUT_PIN_CLICK, INPUT_PULLUP);

  PCICR = _BV(PCIE0) | _BV(PCIE1) | _BV(PCIE2);
  PCMSK2 |= _BV(PCINT20);
  PCMSK1 |= _BV(PCINT8);
  PCMSK0 |= _BV(PCINT0);
}

bool userinput_is_move_left() {
  if (userinput_flags & _BV(USERINPUT_BIT_LEFT)) {
    userinput_flags &= ~(_BV(USERINPUT_BIT_LEFT));
    return true;
  }

  return false;
}

bool userinput_is_move_right() {
  if (userinput_flags & _BV(USERINPUT_BIT_RIGHT)) {
    userinput_flags &= ~(_BV(USERINPUT_BIT_RIGHT));
    return true;
  }

  return false;
}

bool userinput_is_click() {
  if (userinput_flags & _BV(USERINPUT_BIT_CLICK)) {
    userinput_flags &= ~(_BV(USERINPUT_BIT_CLICK));
    return true;
  }

  return false;
}

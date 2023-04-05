#include "userinput.h"

#include "Arduino.h"
#include "charger_control.h"
#include "powersave.h"

// This code based on Alex Gyver samples.
// Thanks to him for the code.
// ref: https://alexgyver.ru/encoder

// PCINT20
#define USERINPUT_PIN_A       PD4
// PCINT8
#define USERINPUT_PIN_B       A0
// PCINT0
#define USERINPUT_PIN_CLICK   8
// PCINT23
#define USERINPUT_PIN_WAKEUP  PD7

#define USERINPUT_BIT_LEFT    0
#define USERINPUT_BIT_RIGHT   1
#define USERINPUT_BIT_CLICK   2
#define USERINPUT_BIT_WAKEUP  3
#define USERINPUT_BIT_USBINT  4

volatile uint8_t userinput_flags = 0;
volatile uint8_t userinput_lastStateA = LOW;
volatile bool userinput_turnFlag = false;

// A & wakeup
ISR(PCINT2_vect) {
  uint8_t stateA = digitalRead(USERINPUT_PIN_A);
  if (stateA != userinput_lastStateA) {
    userinput_lastStateA = stateA;
    userinput_turnFlag   = !userinput_turnFlag;
    if (userinput_turnFlag) {
      if (digitalRead(USERINPUT_PIN_B) != stateA) {
        userinput_flags |= _BV(USERINPUT_BIT_LEFT);
      } else {
        userinput_flags |= _BV(USERINPUT_BIT_RIGHT);
      }
    }
  }
  if (digitalRead(USERINPUT_PIN_WAKEUP) == LOW) {
    userinput_flags |= _BV(USERINPUT_BIT_WAKEUP);
    isrcall_powersave_onwakeup();
  }
}

// CLK & usbint
ISR(PCINT0_vect) {
  if (digitalRead(USERINPUT_PIN_CLICK) == LOW) {
    userinput_flags |= _BV(USERINPUT_BIT_CLICK);
  }

  isrcall_charger_control_onusbint();
}

void userinput_init() {
  pinMode(USERINPUT_PIN_A,      INPUT_PULLUP);
  pinMode(USERINPUT_PIN_B,      INPUT_PULLUP);
  pinMode(USERINPUT_PIN_CLICK,  INPUT_PULLUP);
  pinMode(USERINPUT_PIN_WAKEUP, INPUT_PULLUP);

  PCICR   = _BV(PCIE0)   | _BV(PCIE2);
  PCMSK2 |= _BV(PCINT20) | _BV(PCINT23);
  PCMSK0 |= _BV(PCINT0)  | _BV(PCINT1);
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

bool userinput_is_wakeup() {
  if (userinput_flags & _BV(USERINPUT_BIT_WAKEUP)) {
    userinput_flags &= ~(_BV(USERINPUT_BIT_WAKEUP));
    return true;
  }

  return false;
}

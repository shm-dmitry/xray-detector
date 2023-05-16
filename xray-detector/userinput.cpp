#include "userinput.h"

#include "Arduino.h"
#include "charger_control.h"
#include "powersave.h"
#include "clock.h"
#include "config.h"

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

#define USERINPUT_BIT_LEFT     (_BV(0))
#define USERINPUT_BIT_RIGHT    (_BV(1))
#define USERINPUT_BIT_CLICK    (_BV(2))
#define USERINPUT_BIT_WAKEUP   (_BV(3))
#define USERINPUT_BIT_USBINT   (_BV(4))
#define USERINPUT_BIT_FASTMOVE (_BV(5))

volatile uint8_t userinput_flags = 0;
volatile uint8_t userinput_lastStateA = LOW;
volatile bool userinput_turnFlag = false;

#define USERINPUT_SETFLAG(flag)   userinput_flags |= flag
#define USERINPUT_RESETFLAG(flag) userinput_flags &= ~(flag)

// A & wakeup
ISR(PCINT2_vect) {
  uint8_t stateA = digitalRead(USERINPUT_PIN_A);
#if USERINPUT_ENCODER_SIMUL
  if (stateA) {
    USERINPUT_SETFLAG(USERINPUT_BIT_LEFT);
    USERINPUT_RESETFLAG(USERINPUT_BIT_RIGHT & USERINPUT_BIT_FASTMOVE);
  }
#else
  if (stateA != userinput_lastStateA) {
    userinput_lastStateA = stateA;
    userinput_turnFlag   = !userinput_turnFlag;
    if (userinput_turnFlag) {
      if (digitalRead(USERINPUT_PIN_B) != stateA) {
        if (userinput_flags & USERINPUT_BIT_LEFT) {
          USERINPUT_SETFLAG(USERINPUT_BIT_FASTMOVE);
        } else {
          USERINPUT_SETFLAG(USERINPUT_BIT_LEFT);
          USERINPUT_RESETFLAG(USERINPUT_BIT_RIGHT & USERINPUT_BIT_FASTMOVE);
        }
      } else {
        if (userinput_flags & USERINPUT_BIT_RIGHT) {
          USERINPUT_SETFLAG(USERINPUT_BIT_FASTMOVE);
        } else {
          USERINPUT_SETFLAG(USERINPUT_BIT_RIGHT);
          USERINPUT_RESETFLAG(USERINPUT_BIT_LEFT & USERINPUT_BIT_FASTMOVE);
        }
      }
    }
  }
#endif

  if (digitalRead(USERINPUT_PIN_WAKEUP) == LOW) {
    USERINPUT_SETFLAG(USERINPUT_BIT_WAKEUP);
    isrcall_powersave_leave_standby();
  }
}

#if USERINPUT_ENCODER_SIMUL
// B
ISR(PCINT1_vect) {
  if (digitalRead(USERINPUT_PIN_B)) {
    USERINPUT_SETFLAG(USERINPUT_BIT_RIGHT);
    USERINPUT_RESETFLAG(USERINPUT_BIT_LEFT & USERINPUT_BIT_FASTMOVE);
  }
}
#endif

// CLK & usbint
ISR(PCINT0_vect) {
  if (digitalRead(USERINPUT_PIN_CLICK) == LOW) {
    USERINPUT_SETFLAG(USERINPUT_BIT_CLICK);
  }

  isrcall_charger_control_onusbint();
}

void userinput_init() {
  pinMode(USERINPUT_PIN_A,      INPUT_PULLUP);
  pinMode(USERINPUT_PIN_B,      INPUT_PULLUP);
  pinMode(USERINPUT_PIN_CLICK,  INPUT_PULLUP);
  pinMode(USERINPUT_PIN_WAKEUP, INPUT);

  clock_delay(5); // await for a filter capacitors before attach interrupts

#if USERINPUT_ENCODER_SIMUL
  PCICR   = _BV(PCIE0)   | _BV(PCIE1) | _BV(PCIE2);
#else
  PCICR   = _BV(PCIE0)   | _BV(PCIE2);
#endif

  userinput_on_stop_sleep();
}

uint8_t userinput_get_move() {
  uint8_t result = USERINPUT_MOVE_NONE;

  uint8_t oldSREG = SREG;
  cli();

  if (userinput_flags & USERINPUT_BIT_LEFT) {
    if (userinput_flags & USERINPUT_BIT_FASTMOVE) {
      USERINPUT_RESETFLAG(USERINPUT_BIT_FASTMOVE);
      result = USERINPUT_MOVE_LEFTFAST;
    } else {
      USERINPUT_RESETFLAG(USERINPUT_BIT_LEFT);
      result = USERINPUT_MOVE_LEFT;
    }
  } else if (userinput_flags & USERINPUT_BIT_RIGHT) {
    if (userinput_flags & USERINPUT_BIT_FASTMOVE) {
      USERINPUT_RESETFLAG(USERINPUT_BIT_FASTMOVE);
      result = USERINPUT_MOVE_RIGHTFAST;
    } else {
      USERINPUT_RESETFLAG(USERINPUT_BIT_RIGHT);
      result = USERINPUT_MOVE_RIGHT;
    }
  }

  SREG = oldSREG;

  return result;
}

bool userinput_is_click() {
  if (userinput_flags & USERINPUT_BIT_CLICK) {
    USERINPUT_RESETFLAG(USERINPUT_BIT_CLICK);
    return true;
  }

  return false;
}

bool userinput_is_wakeup() {
  if (userinput_flags & USERINPUT_BIT_WAKEUP) {
    USERINPUT_RESETFLAG(USERINPUT_BIT_WAKEUP);
    return true;
  }

  return false;
}

void userinput_on_start_sleep() {
  PCMSK2 &= ~(_BV(PCINT20));
#if USERINPUT_ENCODER_SIMUL
  PCMSK1 &= ~(_BV(PCINT8));
#endif
  PCMSK0 &= ~(_BV(PCINT0));
}

void userinput_on_stop_sleep() {
  PCMSK2 |= _BV(PCINT20) | _BV(PCINT23);
#if USERINPUT_ENCODER_SIMUL
  PCMSK1 |= _BV(PCINT8);
#endif
  PCMSK0 |= _BV(PCINT0)  | _BV(PCINT1);

  uint8_t oldSREG = SREG;
  cli();

  if (userinput_flags & USERINPUT_BIT_WAKEUP) {
    userinput_flags = USERINPUT_BIT_WAKEUP;
  } else {
    userinput_flags = 0;
  }

  SREG = oldSREG;
}

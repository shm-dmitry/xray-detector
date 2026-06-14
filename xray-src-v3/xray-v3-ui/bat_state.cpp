#include "bat_state.h"

#include "Arduino.h"

void bat_state_init() {
  pinMode(PIN_PB4, INPUT);
  pinMode(PIN_PB5, INPUT);
}

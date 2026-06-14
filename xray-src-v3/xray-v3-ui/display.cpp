#include "display.h"

#include "Arduino.h"

void display_init() {
  pinMode(PIN_PC0, OUTPUT);
  digitalWrite(PIN_PC0, HIGH);

  pinMode(PIN_PC3, OUTPUT);
  digitalWrite(PIN_PC3, LOW);
}

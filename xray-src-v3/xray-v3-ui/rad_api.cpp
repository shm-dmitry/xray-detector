#include "rad_api.h"

#include "Arduino.h"

void isrcall__on_high_rad();

void rad_api_init() {
  pinMode(PIN_PA4, INPUT);
  attachInterrupt(digitalPinToInterrupt(PIN_PA4), isrcall__on_high_rad, FALLING);

  pinMode(PIN_PA7, OUTPUT);
  digitalWrite(PIN_PA7, LOW);
}

void isrcall__on_high_rad() {

}

#include "user_input.h"

#include "Arduino.h"

void isrcall__on_user_input();

void user_input_init() {
  pinMode(PIN_PA5, INPUT);
  attachInterrupt(digitalPinToInterrupt(PIN_PA5), isrcall__on_user_input, FALLING);
}

void isrcall__on_user_input() {

}

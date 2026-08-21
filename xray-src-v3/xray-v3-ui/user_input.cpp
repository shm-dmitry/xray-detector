#include "user_input.h"

#include "Arduino.h"
#include "ext_clock_ctrl.h"

#define USER_INPUT_SLEEP_DELAY (20*1000L)

volatile bool user_input_button_pressed = false;
volatile uint32_t user_input_next_allow_sleep = 0; 

void isrcall__on_user_input();

void user_input_init() {
  pinMode(PIN_PA5, INPUT);
  attachInterrupt(digitalPinToInterrupt(PIN_PA5), isrcall__on_user_input, CHANGE);

  user_input_next_allow_sleep = ext_clock_millis() + USER_INPUT_SLEEP_DELAY;
}

void isrcall__on_user_input() {
  user_input_button_pressed = (digitalRead(PIN_PA5) == LOW);
  user_input_next_allow_sleep = isrcall__ext_clock_millis() + USER_INPUT_SLEEP_DELAY;
}

bool user_input_can_sleep() {
  return !user_input_button_pressed && user_input_next_allow_sleep < ext_clock_millis();
}

bool user_input_pressed() {
  bool v = user_input_button_pressed;
  user_input_button_pressed = false;
  return v;
}

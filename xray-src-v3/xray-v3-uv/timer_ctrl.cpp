#include "rad_data.h"
#include "timer_ctrl.h"

#include "Arduino.h"
#include "uv_gen.h"
#include "rad_control.h"

void isrcall_on_timer() {
  isrcall__rad_data_on_one_second();
  isrcall__rad_control_on_one_second();
}

void timerctrl_init() {
  pinMode(PIN_PA1, INPUT);
  attachInterrupt(digitalPinToInterrupt(PIN_PA1), isrcall_on_timer, FALLING);
}

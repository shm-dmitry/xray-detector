#include "rad_data.h"
#include "timer_ctrl.h"

#include "Arduino.h"
#include "uv_gen.h"
#include "rad_control.h"
#include "geigerlog.h"

volatile bool timerctrl_alive = false;

void isrcall__on_timer() {
  isrcall__rad_data_on_one_second();
  isrcall__rad_control_on_one_second();

#if GEIGERLOG_ENABLED
  isrcall__geigerlog_on_one_second();
#endif

  timerctrl_alive = true;
}

void timerctrl_init() {
  pinMode(PIN_PA1, INPUT);
  attachInterrupt(digitalPinToInterrupt(PIN_PA1), isrcall__on_timer, FALLING);
}

bool isrcall__timerctrl_is_alive() {
  return timerctrl_alive;
}

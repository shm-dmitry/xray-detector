#include "timer_ctrl.h"

#include "Arduino.h"
#include "uv_gen.h"
#include "rad_control.h"

#define TIMERCTRL_UV_START_EVERY_SECONDS (4*60)
#define TIMERCTRL_UV_START_AWAIT_TIMER (TIMERCTRL_UV_START_EVERY_SECONDS - 1)

volatile uint8_t timerctrl_uv_start_delay = TIMERCTRL_UV_START_AWAIT_TIMER;

void isrcall_on_timer() {
  timerctrl_uv_start_delay++;

  isrcall__rad_control_on_one_second();
}

void timerctrl_init() {
  pinMode(PIN_PA1, INPUT);
  attachInterrupt(digitalPinToInterrupt(PIN_PA1), isrcall_on_timer, CHANGE);
}

void timerctrl_on_uv_runned() {
  timerctrl_uv_start_delay = 0;
}

void timerctrl_on_main_loop() {
  if (timerctrl_uv_start_delay >= TIMERCTRL_UV_START_EVERY_SECONDS) {
    timerctrl_uv_start_delay = 0;
    uv_gen_start();
  }
}

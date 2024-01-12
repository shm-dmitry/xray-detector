#include "powersave.h"

#include "userinput.h"
#include "display.h"
#include "charger_control.h"
#include "svf_control.h"
#include "gui_manager.h"
#include "config.h"
#include "clock.h"
#include "uv_control.h"

#include "Arduino.h"
#include <avr/sleep.h>

#define POWERSAVE_ALLOW_STANDBY           true

#define POWERSAVE_WORK_MODES_NORMAL       0
#define POWERSAVE_WORK_MODES_LIGHTSLEEP   1
#define POWERSAVE_WORK_MODES_STANDBY      2
#define POWERSAVE_WORK_MODES_LEAVESTANDBY 3

#define POWERSAVE_STANDBY_SLEEP_COUNT     10

#define POWERSAVE_WORK_MODES_UV_MIN_SECONDS_TO_SLEEP 10
#define POWERSAVE_AUTO_SLEEP_TIMEOUT      ((uint32_t)5*60*1000)

uint8_t powersave_mode = POWERSAVE_WORK_MODES_NORMAL;
uint32_t powersave_normal_mode_activated = 0;

void powersave_enter_light_sleep();
void powersave_enter_extended_standby();
void powersave_leave_light_sleep();
void powersave_leave_extended_standby();
bool powersave_check_can_go_standby();

bool powersave_on_main_loop() {
  if (powersave_mode == POWERSAVE_WORK_MODES_NORMAL) {
    if (clock_is_elapsed(powersave_normal_mode_activated, POWERSAVE_AUTO_SLEEP_TIMEOUT)) {
      powersave_enter_light_sleep();
    }
  }

  if (userinput_is_wakeup()) {
    if (powersave_mode == POWERSAVE_WORK_MODES_NORMAL) {
      powersave_enter_light_sleep();
    } else {
      powersave_wakeup();
    }
  }
  
  if (powersave_mode == POWERSAVE_WORK_MODES_LIGHTSLEEP) {
    if (powersave_check_can_go_standby()) {
      powersave_enter_extended_standby();
    }
  }

  if (powersave_mode == POWERSAVE_WORK_MODES_LEAVESTANDBY) {
    powersave_leave_extended_standby();
  }

  if (powersave_mode == POWERSAVE_WORK_MODES_STANDBY) {
    if (!powersave_check_can_go_standby()) {
      powersave_mode = POWERSAVE_WORK_MODES_LEAVESTANDBY;
    } else {
      for (uint8_t i = 0; i<POWERSAVE_STANDBY_SLEEP_COUNT; i++) {
        if (powersave_mode != POWERSAVE_WORK_MODES_STANDBY) {
          break;
        }

        sleep_cpu();
      }
    }
  }
  
  if (powersave_mode == POWERSAVE_WORK_MODES_LEAVESTANDBY) {
    powersave_leave_extended_standby();
  }

  return powersave_is_on();
}

void powersave_enter_light_sleep() {
  if (powersave_mode == POWERSAVE_WORK_MODES_LIGHTSLEEP) {
    return;
  }

  if (powersave_mode == POWERSAVE_WORK_MODES_NORMAL) {
    display_off();
    charger_control_enter_sleep_mode();
    svf_control_stop();
    userinput_on_start_sleep();

    pinMode(MOSI, INPUT);

#if POWERSAVE_LIGHTSLEEP_DISABLE_IO
    PRR = _BV(PRTWI) |
          _BV(PRTIM2) |
          _BV(PRSPI)  |
          _BV(PRADC)  |
          _BV(PRUSART0);
#endif

    powersave_mode = POWERSAVE_WORK_MODES_LIGHTSLEEP;
  } else {
    powersave_leave_extended_standby();
  }
}

void powersave_leave_light_sleep() {
#if POWERSAVE_LIGHTSLEEP_DISABLE_IO
#if SYSTEM_SERIAL_ENABLED
  PRR = 0;
  Serial.begin(SYSTEM_SERIAL_FREQ);
#else
  PRR = _BV(PRUSART0);
#endif
#endif

  charger_control_leave_sleep_mode();
  display_on();
  gui_manager_on_wakeup();
  userinput_on_stop_sleep();

  powersave_mode = POWERSAVE_WORK_MODES_NORMAL;
  powersave_normal_mode_activated = clock_millis();
}

void powersave_enter_extended_standby() {
  if (powersave_mode == POWERSAVE_WORK_MODES_STANDBY || powersave_mode == POWERSAVE_WORK_MODES_LEAVESTANDBY) {
    return;
  }

  if (powersave_mode == POWERSAVE_WORK_MODES_NORMAL) {
    powersave_enter_light_sleep();
    return;
  }

  PRR &= ~(_BV(PRTIM2));

  clock_enter_sleep_mode();

  set_sleep_mode(SLEEP_MODE_EXT_STANDBY);
  sleep_enable();

  powersave_mode = POWERSAVE_WORK_MODES_STANDBY;
}

void powersave_leave_extended_standby() {
  clock_leave_sleep_mode();
  PRR |= _BV(PRTIM2);

  sleep_disable();

  powersave_mode = POWERSAVE_WORK_MODES_LIGHTSLEEP;
}

bool powersave_is_on() {
  return powersave_mode == POWERSAVE_WORK_MODES_NORMAL;
}

void powersave_wakeup() {
  if (powersave_mode == POWERSAVE_WORK_MODES_NORMAL) {
    return;
  }

  if (powersave_mode == POWERSAVE_WORK_MODES_STANDBY || powersave_mode == POWERSAVE_WORK_MODES_LEAVESTANDBY) {
    powersave_leave_extended_standby();
  }

  if (powersave_mode == POWERSAVE_WORK_MODES_LIGHTSLEEP) {
    powersave_leave_light_sleep();
  }
}

bool powersave_check_can_go_standby() {
  if (uv_control_seconds_between_uv_on() < POWERSAVE_WORK_MODES_UV_MIN_SECONDS_TO_SLEEP) {
    return false;
  }

  if (charger_control_is_active()) {
    return false;
  }

  if (uv_control_is_on()) {
    return false;
  }

  return POWERSAVE_ALLOW_STANDBY;
}

void isrcall_powersave_leave_standby() {
  if (powersave_mode == POWERSAVE_WORK_MODES_STANDBY) {
    powersave_mode = POWERSAVE_WORK_MODES_LEAVESTANDBY;
  }
}

#include "powersave.h"

#include "userinput.h"
#include "display.h"
#include "charger_control.h"
#include "svf_control.h"
#include "gui_manager.h"

#include "Arduino.h"

#define POWERSAVE_WORK_MODES_NORMAL     0
#define POWERSAVE_WORK_MODES_LIGHTSLEEP 1
#define POWERSAVE_WORK_MODES_STANDBY    2

uint8_t powersave_mode = POWERSAVE_WORK_MODES_NORMAL;

void powersave_enter_light_sleep();
void powersave_enter_extended_standby();
void powersave_leave_light_sleep();
void powersave_leave_extended_standby();
bool powersave_check_can_go_standby();

bool powersave_on_main_loop() {
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

    powersave_mode = POWERSAVE_WORK_MODES_LIGHTSLEEP;
  } else {
    powersave_leave_extended_standby();
  }
}

void powersave_enter_extended_standby() {
  if (powersave_mode == POWERSAVE_WORK_MODES_STANDBY) {
    return;
  }

  if (powersave_mode == POWERSAVE_WORK_MODES_NORMAL) {
    powersave_enter_light_sleep();
    return;
  }

  // TODO enter extended standby
}

void powersave_leave_extended_standby() {
  // TODO: go from extended standby to light sleep
}
bool powersave_is_on() {
  return powersave_mode == POWERSAVE_WORK_MODES_NORMAL;
}

void powersave_wakeup() {
  if (powersave_mode == POWERSAVE_WORK_MODES_NORMAL) {
    return;
  }

  if (powersave_mode == POWERSAVE_WORK_MODES_STANDBY) {
    powersave_leave_extended_standby();
  }

  if (powersave_mode == POWERSAVE_WORK_MODES_LIGHTSLEEP) {
    powersave_leave_light_sleep();
  }
}

void powersave_leave_light_sleep() {
  display_on();
  gui_manager_on_wakeup();

  powersave_mode = POWERSAVE_WORK_MODES_NORMAL;
}

bool powersave_check_can_go_standby() {
  return false;
}

void isrcall_powersave_onwakeup() {
  if (!powersave_is_on()) {
    isrcall_userinput_force_wakeup();
  }
}

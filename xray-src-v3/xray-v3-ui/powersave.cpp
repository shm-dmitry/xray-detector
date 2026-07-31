#include "uart_ctrl.h"
#include "powersave.h"

#include "stdint.h"

#include "user_input.h"
#include "uart_ctrl.h"
#include "rad_api.h"
#include "ui_manager.h"
#include "i2c_ctrl.h"

#define POWERSAVE_ALLOW_SLEEP (false)

void powersave_wakeup();
void powersave_prepare_sleep();
bool powersave_can_sleep();
void powersave_execute_sleep();

bool powersave_is_sleep = true;

void powersave_on_main_loop() {
  if (powersave_can_sleep()) {
    if (!powersave_is_sleep) {
      powersave_prepare_sleep();
      powersave_is_sleep = true;
    }

    powersave_execute_sleep();      
  } else {
    if (powersave_is_sleep) {
      powersave_wakeup();
      powersave_is_sleep = false;
    }
  }
}

bool powersave_can_sleep() {
  if (!user_input_can_sleep()) {
    return false;
  }

  if (!uart_ctrl_can_go_sleep()) {
    return false;
  }

  if (!rad_api_can_go_sleep()) {
    return false;
  }

  return POWERSAVE_ALLOW_SLEEP;
}

void powersave_wakeup() {
  i2c_ctrl_on_wake_up();
  rad_api_wakeup();
  ui_manager_wakeup();
}

void powersave_prepare_sleep() {
  i2c_ctrl_on_go_sleep();
  rad_api_go_sleep();
  ui_manager_go_sleep();
}

void powersave_execute_sleep() {
  // TODO: power off and sleep
}



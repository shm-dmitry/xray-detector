#include "powersave.h"

#include "userinput.h"
#include "display.h"
#include "charger_control.h"

void powersave_init() {
}

bool powersave_on_main_loop() {
  if (userinput_is_wakeup()) {
    if (powersave_is_on()) {
      display_off();
      charger_control_enter_sleep_mode();

      // TODO: enter to Extended Standby mode
  
      return false;
    } else {
      display_on();
    }
  }
  
  // TODO: work on Extended Standby mode if need
  
  return powersave_is_on();
}

bool powersave_is_on() {
  return display_is_on();
}

void powersave_wakeup() {
  // TODO: leave Extended Standby
  display_on();
}

void isrcall_powersave_onwakeup() {
  if (!powersave_is_on()) {
    powersave_wakeup();
  }
}

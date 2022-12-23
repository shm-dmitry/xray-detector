#include "alarm_manager.h"

#include "Arduino.h"

#include "eeprom_control.h"
#include "powersave.h"
#include "rad_control.h"
#include "svf_control.h"
#include "pages_control.h"
#include "clock.h"

#define ALARM_MANAGER_OPEN_RAD_PAGE_EVERY   30000

uint16_t alarm_manager_level1 = 0xFFFF;
uint16_t alarm_manager_level2 = 0xFFFF;

volatile bool alarm_manager_wasimpulse = false;
volatile uint32_t alarm_manager_next_open_rad = 0;

void alarm_manager_init() {
  alarm_manager_refresh_levels();
}

void alarm_manager_refresh_levels() {
  eeprom_control_get_alarm_levels(alarm_manager_level1, alarm_manager_level2);
}

uint16_t alarm_manager_getlevel(uint8_t level) {
  return level == 1 ? alarm_manager_level1 : alarm_manager_level2;
}

bool alarm_manager_open_rad_page() {
  uint32_t now = clock_millis();
  if (alarm_manager_next_open_rad < now) {
    Serial.println("alarm - open rad");
    pages_control_openrad();

    if (now + ALARM_MANAGER_OPEN_RAD_PAGE_EVERY < now) {
      alarm_manager_next_open_rad = 0xFFFFFFFF; // isrcall will restart this variable
    } else {
      alarm_manager_next_open_rad = now + ALARM_MANAGER_OPEN_RAD_PAGE_EVERY;
    }

    return true;
  } else {
    return false;
  }
}

void alarm_manager_on_main_loop() {
  if (alarm_manager_wasimpulse) {
    alarm_manager_wasimpulse = false;
    uint32_t dose = rad_control_dose();
    if (dose >= alarm_manager_level2) {
      if (alarm_manager_open_rad_page()) {
        svf_control_play_sound__alarm2();
        svf_control_play_vibro__alarm2();
      }
    } else if (dose >= alarm_manager_level1) {
      if (alarm_manager_open_rad_page()) {
        svf_control_play_sound__alarm1();
        svf_control_play_vibro__alarm1();
      }
    } else {
      svf_control_play_sound__impuls();
      svf_control_play_vibro__impuls();
    }
  }
}

void alarm_manager_mute_alarm() {
  svf_control_sound_mute();
  svf_control_vibro_mute();
}

void isrcall_alarm_manager_onimpulse() {
  alarm_manager_wasimpulse = true;
  if (rad_control_dose(true) >= alarm_manager_level2) {
    if (!powersave_is_on()) {
      powersave_wakeup();
    }
  }
}

void isrcall_alarm_manager_onresetmillis() {
  alarm_manager_next_open_rad = 0;
}

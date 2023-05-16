#include "alarm_manager.h"

#include "Arduino.h"

#include "eeprom_control.h"
#include "powersave.h"
#include "rad_control.h"
#include "svf_control.h"
#include "gui_manager.h"
#include "clock.h"

#define ALARM_MANAGER_OPEN_RAD_PAGE_EVERY   30000
#define ALARM_MANAGER_CHECK_MINDOSE2_EVERY  2000

uint8_t alarm_manager_level1 = 0xFF;
uint8_t alarm_manager_level2 = 0xFF;
uint8_t alarm_manager_onimpulse_voice_type = ALARM_MANAGER_ONIMPULSE_VOICE_VIBRO_AND_VOICE;

volatile bool alarm_manager_wasimpulse = false;
volatile uint32_t alarm_manager_next_open_rad = 0;
volatile uint32_t alarm_manager_last_impulse = 0;
volatile uint8_t alarm_manager_no_impulse_seconds = 0;

void alarm_manager_init() {
  alarm_manager_refresh_levels();
}

void alarm_manager_refresh_levels() {
  eeprom_control_get_alarm_levels(alarm_manager_level1, alarm_manager_level2);
  alarm_manager_no_impulse_seconds = eeprom_control_get_noimpulse_seconds();
  alarm_manager_onimpulse_voice_type = eeprom_control_get_onimpulse_voice();
}

uint8_t alarm_manager_getlevel(uint8_t level) {
  if (level == 1) {
    return alarm_manager_level1;
  } else if (level == 2) {
    return alarm_manager_level2;
  } else if (level == ALARM_MANAGER_NI_LEVEL) {
    return alarm_manager_no_impulse_seconds;
  } else {
    return 0;
  }
}

uint8_t alarm_manager_dose2level(uint32_t dose) {
  if (dose >= (uint32_t)alarm_manager_level2 * 1000) {
    return 2;
  } else if (dose >= alarm_manager_level1) {
    return 1;
  } else {
    return 0;
  }
}

bool alarm_manager_open_rad_page() {
  uint32_t now = clock_millis();
  if (alarm_manager_next_open_rad < now) {
    gui_manager_openrad();

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
    uint8_t level = alarm_manager_dose2level(rad_control_dose());
    if (level > 0 && !powersave_is_on()) {
      powersave_wakeup();
    }

    if (level == 2) {
      if (alarm_manager_open_rad_page()) {
        svf_control_play_sound__alarm2();
        svf_control_play_vibro__alarm2();
      }
    } else if (level == 1) {
      if (alarm_manager_open_rad_page()) {
        svf_control_play_sound__alarm1();
        svf_control_play_vibro__alarm1();
      }
    } else if (powersave_is_on()) {
      if (alarm_manager_onimpulse_voice_type == ALARM_MANAGER_ONIMPULSE_VOICE_VOICE || 
          alarm_manager_onimpulse_voice_type == ALARM_MANAGER_ONIMPULSE_VOICE_VIBRO_AND_VOICE) {
        svf_control_play_sound__impuls();
      }
      if (alarm_manager_onimpulse_voice_type == ALARM_MANAGER_ONIMPULSE_VOICE_VIBRO || 
          alarm_manager_onimpulse_voice_type == ALARM_MANAGER_ONIMPULSE_VOICE_VIBRO_AND_VOICE) {
        svf_control_play_vibro__impuls();
      }
    }
  }
}

void alarm_manager_mute_alarm() {
  svf_control_sound_mute();
  svf_control_vibro_mute();
}

void isrcall_alarm_manager_onimpulse() {
  alarm_manager_wasimpulse = true;
  if (!clock_is_elapsed(alarm_manager_last_impulse, ALARM_MANAGER_CHECK_MINDOSE2_EVERY, true)) {
    return;
  }  

  alarm_manager_last_impulse = clock_millis(true);
  if (isrcall_rad_control_check_dose_alarm((uint16_t)alarm_manager_level2 * 1000, alarm_manager_last_impulse)) {
    if (!powersave_is_on()) {
      powersave_wakeup();
    }
  }
}

void isrcall_alarm_manager_onminute() {
  if (clock_millis(true) - alarm_manager_last_impulse > alarm_manager_no_impulse_seconds * 1000) {
    svf_control_play_sound__alarm3();
    svf_control_play_vibro__alarm3();
  }
}

void isrcall_alarm_manager_onresetmillis() {
  alarm_manager_next_open_rad = 0;
  alarm_manager_last_impulse = 0;
}

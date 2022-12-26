#include "rad_control.h"
#include "Arduino.h"
#include "uv_control.h"
#include "clock.h"
#include "svf_control.h"
#include "alarm_manager.h"

#define RAD_CONTROL_PIN               2

#define RAD_CONTROL_STORE_POINTS      12
#define RAD_CONTROL_STORE_PER_S       10

// RAD_CONTROL_PRIORITY_VALUE - be carefull with overflow!
#define RAD_CONTROL_PRIORITY_VALUE    2
#define RAD_CONTROL_PRIORITY_POINTS   3
#define RAD_CONTROL_MINIMPL_TO_RESULT 40
#define RAD_CONTROL_IMPL_PER_UR       100

volatile uint32_t rad_control_counters[RAD_CONTROL_STORE_POINTS] = { 0 };

void isr_rad_control_one_event() {
  if (rad_control_counters[0] == 0 && !isrcall_uv_control_is_initialized()) {
    return;
  }
  
  rad_control_counters[0]++;

  isrcall_uv_control_on_impulse();
  isrcall_alarm_manager_onimpulse();
}

void rad_control_init() {
  pinMode(RAD_CONTROL_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(RAD_CONTROL_PIN), isr_rad_control_one_event, FALLING);
}

void isrcall_rad_control_on_timer(uint8_t seconds) {
  if (seconds % RAD_CONTROL_STORE_PER_S == 0) {
    for (uint8_t i = RAD_CONTROL_STORE_POINTS - 1; i>0; i--) {
      rad_control_counters[i] = rad_control_counters[i - 1];
    }
    rad_control_counters[0] = 0;
  }
}

bool isrcall_rad_control_calc_impl(uint32_t & impulses, uint8_t & pertime, bool fast) {
  for (uint8_t i = 0; i<RAD_CONTROL_STORE_POINTS; i++) {
    impulses += rad_control_counters[i];
    if (i == 0) {
      uint8_t secs = clock_get_time(CLOCK_TIME_SECOND);
      pertime += secs - ((secs / RAD_CONTROL_STORE_PER_S) * RAD_CONTROL_STORE_PER_S);
    } else {
      pertime += RAD_CONTROL_STORE_PER_S;
    }

    if (impulses >= RAD_CONTROL_MINIMPL_TO_RESULT) {
      return true;
    }

    if (i == RAD_CONTROL_PRIORITY_POINTS) {
      if (fast) {
        return false;
      }
      
      impulses *= RAD_CONTROL_PRIORITY_VALUE;
      pertime  *= RAD_CONTROL_PRIORITY_VALUE;
    }
  }

  return false;
}

bool rad_control_calc_impl(uint32_t & impulses, uint8_t & pertime, bool isrcall = false) {
  if (isrcall) {
    uint8_t oldSREG = SREG;
    cli();
    bool res = isrcall_rad_control_calc_impl(impulses, pertime, true);
    SREG = oldSREG;
    return res;
  } else {
    return isrcall_rad_control_calc_impl(impulses, pertime, false);
  }
}

uint32_t rad_control_dose(bool isrcall = false) {
  uint32_t impulses = 0;
  uint8_t  time = 0;

  if (!rad_control_calc_impl(impulses, time, isrcall)) {
    return 0; // undef
  }

  return ((impulses * 60 * 60) / (uint32_t) time) / RAD_CONTROL_IMPL_PER_UR;
}

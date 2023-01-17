#include "rad_control.h"
#include "Arduino.h"
#include "uv_control.h"
#include "clock.h"
#include "svf_control.h"
#include "alarm_manager.h"

#define RAD_CONTROL_PIN               2

#define RAD_CONTROL_STORE_POINTS      12
#define RAD_CONTROL_MAX_SECONDS_IN_PT 30
#define RAD_CONTROL_MIN_SECONDS_IN_PT 5
#define RAD_CONTROL_MAX_IMPULS_PER_PT 100

// RAD_CONTROL_PRIORITY_VALUE - be carefull with overflow!
#define RAD_CONTROL_PRIORITY_VALUE    2
#define RAD_CONTROL_PRIORITY_POINTS   3
#define RAD_CONTROL_MINIMPL_TO_RESULT 40
#define RAD_CONTROL_IMPL_PER_UR       100

volatile uint16_t rad_control_counters[RAD_CONTROL_STORE_POINTS] = { 0 };
volatile uint8_t  rad_control_timers[RAD_CONTROL_STORE_POINTS] = { 0 };

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
  rad_control_timers[0]++;
  
  if (rad_control_timers[0] > RAD_CONTROL_MAX_SECONDS_IN_PT || 
      (
          rad_control_counters[0] > RAD_CONTROL_MAX_IMPULS_PER_PT && 
          rad_control_timers[0] > RAD_CONTROL_MIN_SECONDS_IN_PT
      )
     ) {
    for (uint8_t i = RAD_CONTROL_STORE_POINTS - 1; i>0; i--) {
      rad_control_counters[i] = rad_control_counters[i - 1];
      rad_control_timers[i]   = rad_control_timers[i - 1];
    }
    rad_control_counters[0] = 0;
    rad_control_timers[0] = 0;
  }
}

bool isrcall_rad_control_calc_impl(uint32_t & impulses, uint16_t & pertime, bool fast) {
  for (uint8_t i = 0; i<RAD_CONTROL_STORE_POINTS; i++) {
    impulses += rad_control_counters[i];
    pertime  += rad_control_timers[i];

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

bool rad_control_calc_impl(uint32_t & impulses, uint16_t & pertime, bool isrcall = false) {
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
  uint16_t  time = 0;

  if (!rad_control_calc_impl(impulses, time, isrcall)) {
    return 0; // undef
  }

  return ((impulses * 60 * 60) / (uint32_t) time) / RAD_CONTROL_IMPL_PER_UR;
}

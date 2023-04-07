#include "rad_control.h"
#include "Arduino.h"
#include "uv_control.h"
#include "clock.h"
#include "svf_control.h"
#include "alarm_manager.h"

#define RAD_CONTROL_PIN               2

#define RAD_CONTROL_STORE_POINTS      12
#define RAD_CONTROL_MAX_SECONDS_IN_PT 30
#define RAD_CONTROL_MIN_SECONDS_IN_PT 2
#define RAD_CONTROL_MAX_IMPULS_PER_PT 100

// RAD_CONTROL_PRIORITY_VALUE - be carefull with overflow!
#define RAD_CONTROL_PRIORITY_VALUE    2
#define RAD_CONTROL_PRIORITY_POINTS   3
#define RAD_CONTROL_MINIMPL_TO_RESULT 40
#define RAD_CONTROL_IMPL_PER_UR       (4 * 50)
#define RAD_CONTROL_IMPL2UR_MULT ((uint32_t)60 * (uint32_t)60 * (uint32_t)1000 / (uint32_t)RAD_CONTROL_IMPL_PER_UR)

#define RAD_CONTROL_DUMP_CALC         false

volatile uint32_t rad_control_counters[RAD_CONTROL_STORE_POINTS] = { 0 };
volatile uint8_t  rad_control_timers[RAD_CONTROL_STORE_POINTS] = { 0 };

volatile uint32_t rad_control_cached_value = 0;

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
  rad_control_cached_value = 0;

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

bool isrcall_rad_control_calc_impl(uint32_t & impulses, uint16_t & pertime) {
  for (uint8_t i = 0; i<RAD_CONTROL_STORE_POINTS; i++) {
    impulses += rad_control_counters[i];
    pertime  += rad_control_timers[i];

    if (impulses >= RAD_CONTROL_MINIMPL_TO_RESULT) {
      return true;
    }

    if (i == RAD_CONTROL_PRIORITY_POINTS) {
      impulses *= RAD_CONTROL_PRIORITY_VALUE;
      pertime  *= RAD_CONTROL_PRIORITY_VALUE;
    }
  }

  return false;
}

bool isrcall_rad_control_check_dose_alarm(uint32_t mindose, uint32_t millis) {
  if (rad_control_timers[0] == 0) {
    return false;
  }

  return (rad_control_counters[0] * RAD_CONTROL_IMPL2UR_MULT) / (((uint32_t)rad_control_timers[0]) * 1000 + millis % 1000) >= mindose;
}

uint32_t rad_control_dose() {
  uint32_t impulses = 0;
  uint16_t  time = 0;

  uint8_t oldSREG = SREG;
  cli();

  uint32_t mils = clock_millis(true);

  if (rad_control_cached_value > 0) {
    uint32_t result = rad_control_cached_value;
    SREG = oldSREG;
    return result;
  }

  bool res = isrcall_rad_control_calc_impl(impulses, time);
  uint32_t result = 0;
  if (res) {
    result = (impulses * RAD_CONTROL_IMPL2UR_MULT) / (((uint32_t)time) * 1000 + mils % 1000);
    rad_control_cached_value = result;
  }
  SREG = oldSREG;

#if RAD_CONTROL_DUMP_CALC
  Serial.print("impulses = ");
  Serial.print(impulses);
  Serial.print("; time = ");
  Serial.print(time);
  Serial.print("; mils = ");
  Serial.print(mils);
  Serial.print("; result = ");
  Serial.println(result);
#endif

  return result;
}

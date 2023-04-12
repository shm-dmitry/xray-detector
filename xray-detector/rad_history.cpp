#include "rad_history.h"
#include "rad_control.h"
#include "clock.h"
#include "rad_accum_history.h"

#include "Arduino.h"

#define RAD_HISTORY_TIMER_NOEVENT    0x00
#define RAD_HISTORY_TIMER_EVENT      0x01
#define RAD_HISTORY_TIMER_STOREEVENT 0x02

volatile uint8_t rad_history_on_timer = RAD_HISTORY_TIMER_NOEVENT;
uint32_t rad_history_totaldose = 0;
uint8_t rad_history_totaldose_count = 0;

t_rad_history_minute_points_buffer rad_history_minute_points = { 0 };
uint32_t rad_history_minute_currentdate = 0;
bool rad_history_minute_buffer_changed = false;

void rad_history_on_store_one_event(uint32_t dose);
void rad_history_on_store_last_point(uint32_t dose);

void isrcall_rad_history_on_second(uint8_t seconds) {
  rad_history_on_timer = (seconds % 5 == 0) ? RAD_HISTORY_TIMER_STOREEVENT : RAD_HISTORY_TIMER_EVENT;
}

uint8_t rad_history_get_event() {
  uint8_t tmp;

  uint8_t oldSREG = SREG;
  cli();

  tmp = rad_history_on_timer;
  rad_history_on_timer = RAD_HISTORY_TIMER_NOEVENT;

  SREG = oldSREG;

  return tmp;
}

void rad_history_on_main_loop() {
  uint8_t event = rad_history_get_event();
  if (event == RAD_HISTORY_TIMER_NOEVENT) {
    return;
  }

  if (event == RAD_HISTORY_TIMER_STOREEVENT && rad_history_totaldose_count > 0) {
    uint32_t dose = rad_history_totaldose / rad_history_totaldose_count;
    rad_history_on_store_one_event(dose);

    rad_history_totaldose = 0;
    rad_history_totaldose_count = 0;
    rad_history_minute_currentdate = 0;
  } else if (event == RAD_HISTORY_TIMER_EVENT) {
    if (rad_history_minute_currentdate == 0) {
      rad_history_minute_currentdate = clock_get_packed();
    }

    uint32_t temp = rad_control_dose();

    rad_accum_history_ondose(temp);

    if (temp > 0 && rad_history_totaldose + temp > rad_history_totaldose) {
      rad_history_totaldose += temp;
      rad_history_totaldose_count++;

      rad_history_on_store_last_point(rad_history_totaldose / rad_history_totaldose_count);
    }
  }
}

void rad_history_on_store_last_point(uint32_t dose) {
  if (rad_history_minute_points[0].dose != dose || rad_history_minute_points[0].datepacked == 0) {
    if (rad_history_minute_points[0].datepacked == 0) {
      rad_history_minute_points[0].datepacked = rad_history_minute_currentdate;
    }

    rad_history_minute_points[0].dose = dose;

    rad_history_minute_buffer_changed = true;
  }
}

void rad_history_on_store_one_event(uint32_t dose) {
  for (int8_t i = RAD_HISTORY_STORE_MINUTE_POINTS - 2; i>=1; i--) {
    rad_history_minute_points[i + 1].dose = rad_history_minute_points[i].dose;
    rad_history_minute_points[i + 1].datepacked = rad_history_minute_points[i].datepacked;
  }

  rad_history_minute_points[1].datepacked = rad_history_minute_currentdate;
  rad_history_minute_points[1].dose = dose;

  rad_history_minute_points[0].datepacked = 0;
  rad_history_minute_points[0].dose = dose;

  rad_history_minute_buffer_changed = true;
}

const t_rad_history_minute_points_buffer * rad_history_get_minute_points_buffer() {
  return &rad_history_minute_points;
}

bool rad_history_was_minute_buffer_changed() {
  if (rad_history_minute_buffer_changed) {
    rad_history_minute_buffer_changed = false;
    return true;
  } else {
    return false;
  }
}

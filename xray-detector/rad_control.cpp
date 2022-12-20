#include "rad_control.h"
#include "Arduino.h"
#include "uv_control.h"
#include "clock.h"
#include "svf_control.h"

#define RAD_CONTROL_PIN           2
#define RAD_CONTROL_STORE_POINTS  10
#define RAD_CONTROL_DOSE_ALARM    20

volatile uint32_t rad_control_counters[RAD_CONTROL_STORE_POINTS] = { 0 };
volatile bool rad_control_wasimpulse = false;

void isr_rad_control_one_event() {
  rad_control_counters[0]++;
  rad_control_wasimpulse = true;

  isrcall_uv_control_on_impulse();
}

void rad_control_init() {
  pinMode(RAD_CONTROL_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(RAD_CONTROL_PIN), isr_rad_control_one_event, FALLING);
}

void isrcall_rad_control_on_timer(uint8_t seconds) {
  if (seconds == 0) {
    for (uint8_t i = RAD_CONTROL_STORE_POINTS - 1; i>0; i--) {
      rad_control_counters[i] = rad_control_counters[i - 1];
    }
    rad_control_counters[0] = 0;
  }
}

uint32_t rad_control_impulses_last_minute(uint8_t minute) {
  uint8_t oldSREG = SREG;
  cli();
  uint32_t val = rad_control_counters[minute];
  SREG = oldSREG;
  return val;
}

uint32_t rad_control_dose() {
  uint32_t checkmin = rad_control_impulses_last_minute(2);
  uint32_t prevmin  = rad_control_impulses_last_minute(1);
  uint32_t curmin   = rad_control_impulses_last_minute(0);
  uint8_t seconds_passed = clock_get_time(CLOCK_TIME_SECOND);

  uint32_t total   = 0;
  uint8_t  seconds = 0;

  if (checkmin != 0) {
    // есть данные более чем за 2 минуты, считаем
    total   = prevmin + curmin;
    seconds = 60 + seconds_passed;
  } else if (prevmin == 0 && curmin > 30) {
    // уже чтото успели накопить, считаем
    total   = curmin;
    seconds = seconds_passed;
  }

  if (seconds == 0) {
    return 0; // undef
  }

  return ((total * 60 * 60) / (uint32_t) seconds) / 50;
}

void rad_control_on_main_loop() {
  if (rad_control_wasimpulse) {
    rad_control_wasimpulse = false;

    if (rad_control_dose() > RAD_CONTROL_DOSE_ALARM) {
      svf_control_play_sound__alarm();
      svf_control_play_vibro__alarm();
    } else {
      svf_control_play_sound__impuls();
      svf_control_play_vibro__impuls();
    }
  }
}

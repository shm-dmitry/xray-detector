#include "clock.h"
#include "Arduino.h"

#include "uv_control.h"
#include "rad_control.h"
#include "svf_control.h"
#include "alarm_manager.h"
#include "eeprom_control.h"
#include "rad_history.h"
#include "datepacker.h"

typedef void (*t_wd_timer_impl)();

#define CLOCK_MAX_UINT32_T    0xFFFFFFFF
#define CLOCK_OVF_MILLIS_FIX         296

#define CLOCK_ON_ONE_SECOND_CALLBACKS \
  isrcall_rad_control_on_timer(clock_seconds); \
  isrcall_uv_control_on_timer(); \
  if (clock_seconds == 0) { \
    isrcall_alarm_manager_onminute(); \
  } \
  isrcall_rad_history_on_second(clock_seconds);

#define CLOCK_ON_ONE_MILLISECOND_CALLBACKS \
  isrcall_svf_control_on_millis();

#define CLOCK_ON_RESET_MILLISECOND_CALLBACKS \
  isrcall_alarm_manager_onresetmillis();

#define CLOCK_START_TIMER_COUNTER0  \
  TCCR0B  = _BV(WGM02) | _BV(CS00) | _BV(CS01)

volatile uint32_t clock_millis_value = 0;
volatile uint8_t clock_year    = 22;
volatile uint8_t clock_month   = 12;
volatile uint8_t clock_day     = 7;
volatile uint8_t clock_hour    = 0;
volatile uint8_t clock_minutes = 0;
volatile uint8_t clock_seconds = 0;

volatile uint16_t clock_wd_timer_value_ms =  0;
volatile uint16_t clock_wd_timer_calibrate_start_ms = 0;
volatile t_wd_timer_impl wd_timer_impl;

void clock_on_one_second();
void clock_on_one_millis();
void clock_on_reset_millis();
void isr_clock_wd_timer_calibrate();
void isr_clock_wd_timer_execute();

ISR(TIMER0_COMPA_vect) {
  clock_millis_value++; 
  if (clock_millis_value == 0) {
    clock_millis_value = CLOCK_OVF_MILLIS_FIX; 
    clock_on_reset_millis();
  }
  
  if (clock_millis_value % 1000 == 0) {
    clock_on_one_second();
    clock_on_one_millis();
  } else {
    clock_on_one_millis();
  }
}

ISR(WDT_vect) {
  if (wd_timer_impl) {
    wd_timer_impl();
  }
}

void isr_clock_wd_timer_calibrate() {
  if (clock_wd_timer_calibrate_start_ms > 0) {
    clock_wd_timer_value_ms = clock_millis_value - clock_wd_timer_calibrate_start_ms;
    clock_wd_timer_calibrate_start_ms = clock_millis_value;
  } else {
    clock_wd_timer_calibrate_start_ms = clock_millis_value;
  }
}

void isr_clock_wd_timer_execute() {
  uint32_t prevms = clock_millis_value;
  clock_millis_value += clock_wd_timer_value_ms;
  if (clock_millis_value < prevms) {
    clock_millis_value += CLOCK_OVF_MILLIS_FIX;
    clock_on_reset_millis();
  }

  uint16_t ms_to_next_second = 1000 - prevms % 1000;
  if (clock_wd_timer_value_ms <= ms_to_next_second) {
    clock_on_one_second();
  }  
}

uint8_t clock_days_in_month() {
  const uint8_t ds_in_month[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  
  if (clock_month == 2) {
    uint16_t year = (uint16_t) clock_year + 2000;
    
    if (((year % 4 == 0) && (!(year % 100 == 0))) || (year % 400 == 0)) {
      return 29;
    } else { 
      return 28;
    }
  } else {
    return ds_in_month[(clock_month - 1)];
  }
}

void clock_on_reset_millis() {
  CLOCK_ON_RESET_MILLISECOND_CALLBACKS;
}

void clock_on_one_millis() {
  CLOCK_ON_ONE_MILLISECOND_CALLBACKS;
}

void clock_on_one_second() {  
  clock_seconds++;
  
  if (clock_seconds >= 60) {
    clock_seconds = 0;
    clock_minutes++;
  }

  if (clock_minutes >= 60) {
    clock_minutes = 0;
    clock_hour++;
  }

  if (clock_hour >= 24) {
    clock_hour = 0;
    clock_day++;
  }

  if (clock_day > clock_days_in_month()) {
    clock_day = 1;
    clock_month++;
  }

  if (clock_month > 12) {
    clock_month = 1;
    clock_year++;
  }

  if (clock_seconds == 0 && clock_minutes == 0) { // save date-time hourly
    eeprom_control_save_date_time(clock_year, clock_month, clock_day, clock_hour, clock_minutes);
  }

  CLOCK_ON_ONE_SECOND_CALLBACKS;
}

void clock_init() {
  eeprom_control_get_date_time(clock_year, clock_month, clock_day, clock_hour, clock_minutes);

  TCCR0A  = _BV(WGM01) | _BV(WGM00);
  CLOCK_START_TIMER_COUNTER0;

  OCR0A = F_CPU / 64 / 1000 - 1;
  
  TIMSK0 |= _BV(OCIE0A);   

  // start wd timer in calibrate mode
  wd_timer_impl = &isr_clock_wd_timer_calibrate;

  uint8_t oldSREG = SREG;
  cli();
  WDTCSR = (1 << WDCE) | (1 << WDE); 
  WDTCSR = (0 << WDP3) | (0 << WDP2) | (1 << WDP1) | (1 << WDP0);
  WDTCSR |= (1 << WDIE);
  SREG = oldSREG;
}

uint16_t clock_get_component(uint8_t component) {
  uint16_t year;
  uint8_t month;
  uint8_t day;
  uint8_t hour;
  uint8_t minute;

  clock_get_time(year, month, day, hour, minute);

  switch (component) {
  case CLOCK_COMPONENT_YEAR:
  return year;
  case CLOCK_COMPONENT_MONTH:
  return month;
  case CLOCK_COMPONENT_DAY:
  return day;
  case CLOCK_COMPONENT_HOUR:
  return hour;
  case CLOCK_COMPONENT_MINUTE:
  return minute;
  default: 
  return 0;
  }
}

void clock_set_component(uint8_t component, uint16_t value) {
  uint8_t oldSREG = SREG;
  cli();

  switch (component) {
  case CLOCK_COMPONENT_YEAR:
  clock_year = value - 2000; 
  break;
  
  case CLOCK_COMPONENT_MONTH:
  clock_month = value;
  if (clock_month > 12) {
    clock_month = 12;
  } else if (clock_month == 0) {
    clock_month = 1;
  }
  if (clock_day > clock_days_in_month()) {
    clock_day = clock_days_in_month();
  }
  break;
  
  case CLOCK_COMPONENT_DAY:
  clock_day = value;
  if (clock_day > clock_days_in_month()) {
    clock_day = clock_days_in_month();
  } else if (clock_day == 0) {
    clock_day = 1;
  }
  break;

  case CLOCK_COMPONENT_HOUR:
  clock_hour = value;
  if (clock_hour > 23) {
    clock_hour = 23;
  }
  break;
  
  case CLOCK_COMPONENT_MINUTE:
  clock_minutes = value;
  if (clock_minutes > 59) {
    clock_minutes = 59;
  }
  break;
 }

  SREG = oldSREG;

  eeprom_control_save_date_time(clock_year, clock_month, clock_day, clock_hour, clock_minutes);
}

void clock_get_time(uint16_t & year, uint8_t & month, uint8_t & day, uint8_t & hour, uint8_t & minute) {
  uint8_t oldSREG = SREG;
  cli();

  year = 2000 + clock_year;
  month = clock_month;
  day = clock_day;
  hour = clock_hour;
  minute = clock_minutes;

  SREG = oldSREG;
}

uint32_t clock_get_packed(bool inisr = false) {
  uint8_t oldSREG;
  if (!inisr) {
    oldSREG = SREG;
    cli();
  }

  uint32_t packed;
  DATE_PACK(packed, 2000 + clock_year, clock_month, clock_day, clock_hour, clock_minutes);

  if (!inisr) {
    SREG = oldSREG;
  }

  return packed;
}

uint32_t clock_millis(bool inisr = false) {
  if (inisr) {
    return clock_millis_value;
  }
  
  uint8_t oldSREG = SREG;
  cli();
  uint32_t val = clock_millis_value;
  SREG = oldSREG;
  return val;
}

void clock_delay(uint32_t mils) {
  uint32_t next = clock_millis();
  
  if (next + mils < next) {
    while (clock_millis() >= next);
    next = CLOCK_OVF_MILLIS_FIX + mils - (CLOCK_MAX_UINT32_T - next);
  } else {
    next += mils;
  }

  while (clock_millis() < next);
}

uint32_t clock_calc_delay(uint32_t base, uint32_t delta, bool & ovf) {
  if (base + delta < base) {
    ovf = true;
    return (base + delta) + CLOCK_OVF_MILLIS_FIX;
  } else {
    ovf = false;
    return base + delta;
  }
}

bool clock_is_elapsed(uint32_t base, uint32_t delta, bool inisr = false) {
  uint32_t now = clock_millis(inisr);
  bool ovf = false;
  uint32_t untill = clock_calc_delay(base, delta, ovf);

  if (ovf) {
    return now < base && now >= untill;
  } else {
    return now >= untill;
  }
}

bool clock_enter_sleep_mode() {
  uint8_t oldSREG = SREG;
  cli();

  bool success = (clock_wd_timer_value_ms > 0);
  if (success) {
    wd_timer_impl = &isr_clock_wd_timer_execute;
    TCCR0B = 0;
  }

  SREG = oldSREG;

  return success;
}

void clock_leave_sleep_mode() {
  uint32_t current = clock_millis();
  // await one WDT cicle
  while (current == clock_millis());

  // switch to standart timer
  uint8_t oldSREG = SREG;
  cli();

  wd_timer_impl = &isr_clock_wd_timer_calibrate;
  clock_wd_timer_calibrate_start_ms = 0;
  CLOCK_START_TIMER_COUNTER0;

  SREG = oldSREG;
}


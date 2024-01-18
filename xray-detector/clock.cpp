#include "clock.h"
#include "Arduino.h"

#include "uv_control.h"
#include "rad_control.h"
#include "svf_control.h"
#include "alarm_manager.h"
#include "eeprom_control.h"
#include "rad_history.h"
#include "datepacker.h"

#define CLOCK_MAX_UINT32_T    0xFFFFFFFF
#define CLOCK_OVF_MILLIS_FIX         296
#define CLOCK_TIMER2_TOP             (250 - 1)
#define CLOCK_TIMER2_EXEC_EVERY_MS   1000/((uint32_t)F_CPU / (uint32_t)(CLOCK_TIMER2_TOP + 1) / (uint32_t)1024)

#define CLOCK_ON_ONE_SECOND_CALLBACKS \
  isrcall_rad_control_on_timer(); \
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

void clock_on_one_second();
void clock_on_one_millis();
void clock_on_reset_millis();

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

ISR(TIMER2_COMPA_vect) {
  volatile uint32_t temp = clock_millis_value;
  clock_millis_value += CLOCK_TIMER2_EXEC_EVERY_MS;

  if (clock_millis_value < temp) {
    clock_millis_value += CLOCK_OVF_MILLIS_FIX; 
  }

  if ((clock_millis_value / 1000) != (temp / 1000)) {
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

  Serial.print("CLOCK_TIMER2_EXEC_EVERY_MS = ");
  Serial.println(CLOCK_TIMER2_EXEC_EVERY_MS);
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

uint32_t clock_get_packed(bool inisr) {
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

uint32_t clock_millis(bool inisr) {
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

bool clock_is_elapsed(uint32_t base, uint32_t delta, bool inisr) {
  uint32_t now = clock_millis(inisr);
  bool ovf = false;
  uint32_t untill = clock_calc_delay(base, delta, ovf);

  if (ovf) {
    return now < base && now >= untill;
  } else {
    return now >= untill;
  }
}

void clock_enter_sleep_mode() {
  uint8_t oldSREG = SREG;
  cli();

  TCCR2A = 0;
  TCCR2B = 0;
  TCNT2  = 0;
  TCCR2A = _BV(WGM21) | _BV(WGM20);
  TCCR2B = _BV(CS22) | _BV(CS21) | _BV(CS20) | _BV(WGM22);
  OCR2A  = CLOCK_TIMER2_TOP;
  TIMSK2 = _BV(OCIE2A);

  SREG = oldSREG;
}

void clock_leave_sleep_mode() {
  uint32_t current = clock_millis();
  // await one T2 cicle
  while (current == clock_millis());

  // switch to standart timer
  uint8_t oldSREG = SREG;
  cli();

  TCCR2A = 0;
  TCCR2B = 0;
  OCR2A  = 0;
  TCNT2  = 0;
  TIMSK2 = 0;
  
  CLOCK_START_TIMER_COUNTER0;

  SREG = oldSREG;
}

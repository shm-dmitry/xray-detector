#include "clock.h"
#include "Arduino.h"

#include "uv_control.h"
#include "rad_control.h"
#include "svf_control.h"

#define CLOCK_MAX_UINT32_T    4294967295
#define CLOCK_OVF_MILLIS_FIX         296

#define CLOCK_ON_ONE_SECOND_CALLBACKS \
  isrcall_rad_control_on_timer(clock_seconds); \
  isrcall_uv_control_on_timer();

#define CLOCK_ON_ONE_MILLISECOND_CALLBACKS \
  isrcall_svf_control_on_millis();

volatile uint32_t clock_millis_value = 0;
volatile uint8_t clock_year    = 22;
volatile uint8_t clock_month   = 12;
volatile uint8_t clock_day     = 7;
volatile uint8_t clock_hour    = 0;
volatile uint8_t clock_minutes = 0;
volatile uint8_t clock_seconds = 0;

void clock_on_one_second();
void clock_on_one_millis();

ISR(TIMER0_COMPA_vect) {
  clock_millis_value++; 
  if (clock_millis_value == 0) {
    clock_millis_value = CLOCK_OVF_MILLIS_FIX; 
  }
  
  if (clock_millis_value % 1000 == 0) {
    clock_on_one_second();
    clock_on_one_millis();
  } else {
    clock_on_one_millis();
  }
}

uint8_t clock_days_in_month()
{
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

  CLOCK_ON_ONE_SECOND_CALLBACKS;
}

void clock_init() {
  TCCR0A  = _BV(WGM01);
  TCCR0B  = _BV(CS00) | _BV(CS01);

  OCR0A = F_CPU / 64 / 1000 - 1;
  
  TIMSK0 |= _BV(OCIE0A);   
}

uint16_t clock_get_time(uint8_t what) {
  switch (what) {
    case CLOCK_TIME_SECOND: return clock_seconds;
    case CLOCK_TIME_MINUTE: return clock_minutes;
    case CLOCK_TIME_HOUR:   return clock_hour;
    case CLOCK_TIME_DAY:    return clock_day;
    case CLOCK_TIME_MONTH:  return clock_month;
    case CLOCK_TIME_YEAR:   return clock_year + 2000;
    default:                return 0;
  }
}

uint32_t clock_millis() {
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

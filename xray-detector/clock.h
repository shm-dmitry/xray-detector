#ifndef CLOCK_H_
#define CLOCK_H_

#include "stdint.h"

#define CLOCK_COMPONENT_YEAR   0
#define CLOCK_COMPONENT_MONTH  1
#define CLOCK_COMPONENT_DAY    2
#define CLOCK_COMPONENT_HOUR   3
#define CLOCK_COMPONENT_MINUTE 4

void clock_init();

void clock_get_time(uint16_t & year, uint8_t & month, uint8_t & day, uint8_t & hour, uint8_t & minute);
uint16_t clock_get_component(uint8_t component);
uint8_t clock_days_in_month();
void clock_set_component(uint8_t component, uint16_t value);

void clock_delay(uint32_t mils);

uint32_t clock_get_packed(bool inisr = false);
uint32_t clock_millis(bool inisr = false);
uint32_t clock_calc_delay(uint32_t base, uint32_t delta, bool & ovf);
bool clock_is_elapsed(uint32_t base, uint32_t delta, bool inisr = false);

bool clock_enter_sleep_mode();
void clock_leave_sleep_mode();

#endif /* CLOCK_H_ */

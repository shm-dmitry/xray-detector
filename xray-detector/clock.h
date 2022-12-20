#ifndef CLOCK_H_
#define CLOCK_H_

#include "stdint.h"

#define CLOCK_TIME_SECOND 1
#define CLOCK_TIME_MINUTE 2
#define CLOCK_TIME_HOUR   3
#define CLOCK_TIME_DAY    4
#define CLOCK_TIME_MONTH  5
#define CLOCK_TIME_YEAR   6

void clock_init();

uint16_t clock_get_time(uint8_t what);

void clock_delay(uint32_t mils);

#endif /* CLOCK_H_ */

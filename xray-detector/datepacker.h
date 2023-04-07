#ifndef DATEPACKER_H_
#define DATEPACKER_H_

#include "stdbool.h"
#include "stdint.h"

#define DATE_UNPACK(packed, year, month, day, hour, minute) \
  minute = packed & 0b111111; \
  packed = packed >> 6; \
  hour = packed & 0b11111; \
  packed = packed >> 5; \
  day = packed & 0b11111; \
  packed = packed >> 5; \
  month = packed & 0b1111; \
  year = 20 + (packed >> 4);

#define DATE_PACK(packed, year, month, day, hour, minute) \
  packed = year - 20; \
  packed = packed << 4; \
  packed += month; \
  packed = packed << 5; \
  packed += day; \
  packed = packed << 5; \
  packed += hour; \
  packed = packed << 6; \
  packed += minute;

#endif /* DATEPACKER_H_ */


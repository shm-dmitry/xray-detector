#ifndef DATEPACKER_H_
#define DATEPACKER_H_

#include "stdbool.h"
#include "stdint.h"

#define DATE_PACKER_MASK_MM       0b00000000000011110000000000000000
#define DATE_PACKER_MASK_YY       0b11111111111100000000000000000000
#define DATE_PACKER_MASK_YYMM     0b11111111111111110000000000000000
#define DATE_PACKER_MASK_YYMMDD   0b11111111111111111111100000000000
#define DATE_PACKER_MASK_YYMMDDHH 0b11111111111111111111111111000000
#define DATE_PACKER_MASK_MINUTES  0b00000000000000000000000000111111

#define DATE_PACKER__YY(x)      ((uint16_t)(((x) & DATE_PACKER_MASK_YY) >> 20) + 2000) 
#define DATE_PACKER__MM(x)      (((x) & DATE_PACKER_MASK_MM) >> 16) 
#define DATE_PACKER__YYMM(x)    ((x) & DATE_PACKER_MASK_YYMM) 
#define DATE_PACKER__DATE(x)    ((x) & DATE_PACKER_MASK_YYMMDD) 
#define DATE_PACKER__DATE_HH(x) ((x) & DATE_PACKER_MASK_YYMMDDHH) 
#define DATE_PACKER__MINUTES(x) ((x) & DATE_PACKER_MASK_MINUTES) 

#define DATE_UNPACK(packed, year, month, day, hour, minute) \
  minute = packed & 0b111111; \
  packed = packed >> 6; \
  hour = packed & 0b11111; \
  packed = packed >> 5; \
  day = packed & 0b11111; \
  packed = packed >> 5; \
  month = packed & 0b1111; \
  year = 2000 + (packed >> 4);

#define DATE_PACK(packed, year, month, day, hour, minute) \
  packed = year - 2000; \
  packed = packed << 4; \
  packed += month; \
  packed = packed << 5; \
  packed += day; \
  packed = packed << 5; \
  packed += hour; \
  packed = packed << 6; \
  packed += minute;

#endif /* DATEPACKER_H_ */


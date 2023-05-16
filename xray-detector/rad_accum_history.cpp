#include "rad_accum_history.h"
#include "clock.h"
#include "datepacker.h"
#include "eeprom_control.h"
#include "Arduino.h"

#define RAD_ACCUM__R2SV(x) ((x) / 102) // rengen to sievert : 102 rengen == 1 sievert
#define RAD_ACCUM_MAXVALUE 0xFFFFFFFFFFFFFFFF

// all counters in rengen!
uint64_t rad_accum_hour_dose = 0;
uint8_t rad_accum_hour_dose_count = 0;
uint32_t rad_accum_date_packed = 0;

uint32_t rad_accum_daily_dose = 0;

void rad_accum_on_save_date(uint32_t packed);
void rad_accum_on_next_hour();

void rad_accum_history_ondose(uint32_t dose) {
  if (rad_accum_daily_dose == 0xFFFFFFFF) {
    return;
  }
  if (dose == 0) {
    return;
  }

  uint32_t packed = clock_get_packed();
  if (packed == rad_accum_date_packed) {
    return; 
  }

  if (packed != 0) {
    if (DATE_PACKER__DATE(packed) != DATE_PACKER__DATE(rad_accum_date_packed)) {
      rad_accum_on_next_hour();
      rad_accum_on_save_date(packed);
    } else if (DATE_PACKER__DATE_HH(packed) != DATE_PACKER__DATE_HH(rad_accum_date_packed)) {
      rad_accum_on_next_hour();
    }
  }

  rad_accum_date_packed = packed;

  if (rad_accum_hour_dose + (uint64_t)dose > rad_accum_hour_dose) {
    rad_accum_hour_dose += (uint64_t)dose;
    rad_accum_hour_dose_count++;
  } else {
    rad_accum_hour_dose = RAD_ACCUM_MAXVALUE;
  }
}

uint32_t rad_accum_get_daily_dose_usv() {
  if (rad_accum_hour_dose_count == 0) {
    return RAD_ACCUM__R2SV(rad_accum_daily_dose);
  }

  uint8_t minutes = DATE_PACKER__MINUTES(clock_get_packed());
  if (minutes == 0) {
    return RAD_ACCUM__R2SV(rad_accum_daily_dose);
  }

  uint64_t value = (rad_accum_hour_dose / (uint64_t) rad_accum_hour_dose_count / 60) * (uint64_t) minutes;
  if (value > 0xFFFFFFFF) {
    return 0xFFFFFFFF;
  }

  if (rad_accum_daily_dose == 0 && value == 0) {
    return RAD_ACCUM__R2SV(0);
  }

  if (rad_accum_daily_dose + (uint32_t) value > rad_accum_daily_dose) {
    return RAD_ACCUM__R2SV(rad_accum_daily_dose + (uint32_t) value);
  } else {
    return 0xFFFFFFFF;    
  }
}

void rad_accum_on_next_hour() {
  if (rad_accum_hour_dose_count == 0) {
    return;
  }

  if (rad_accum_daily_dose == 0xFFFFFFFF) {
    rad_accum_hour_dose = 0;
    rad_accum_hour_dose_count = 0;
    return;
  }

  uint64_t value = rad_accum_hour_dose / (uint64_t) rad_accum_hour_dose_count;
  if (value >= 0xFFFFFFFF) {
    rad_accum_daily_dose = 0xFFFFFFFF;
  } else if ((uint32_t)rad_accum_daily_dose + (uint32_t)value > (uint32_t)rad_accum_daily_dose) {
    rad_accum_daily_dose += (uint32_t)value;
  } else {
    rad_accum_daily_dose = 0xFFFFFFFF;
  }

  rad_accum_hour_dose = 0;
  rad_accum_hour_dose_count = 0;
}

uint8_t rad_accum_history_points_count() {
  return EEPROM_CONTROL_ACCUMULATED_DOSE_STORE_POINTS;
}

void rad_accum_on_save_date(uint32_t packed) {
  eeprom_control_on_save_rad_accum(packed, rad_accum_daily_dose);
  rad_accum_daily_dose = 0;
}

bool rad_accum_history_read(uint8_t sortedindex, uint32_t & datepacked, uint32_t & value) {
  bool res = eeprom_control_read_accumulated(sortedindex, datepacked, value);
  if (res && value != 0xFFFFFFFF) {
    value = RAD_ACCUM__R2SV(value);
  }
  return res;
}


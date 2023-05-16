#include "eeprom_control.h"

#include "EEPROM.h"
#include "datepacker.h"
#include "config.h"
#include "Arduino.h"
#include "alarm_manager.h"

#define EEPROM_VERSION_ACTUAL 0x01
#define EEPROM_VERSION_UNINIT 0xFF

#define EEPROM_ADDR_VERSION   0x00
#define EEPROM_ADDR_UV        0x01
#define EEPROM_ADDR_ALARM     0x0F
#define EEPROM_ADDR_DATE      0x16
#define EEPROM_ADDR_ACCUM     (1024 - EEPROM_CONTROL_ACCUMULATED_DOSE_STORE_POINTS * 8 - 10)

#define EEPROM_DEFAULT_UV_FREQ  100
#define EEPROM_DEFAULT_UV_DUTY  13
#define EEPROM_DEFAULT_IMPL_PER_UR (4*50)

#define EEPROM_DEFAULT_ALRM_L1    100
#define EEPROM_DEFAULT_ALRM_L2    1
#define EEPROM_DEFAULT_ALRM_NOIMP (60*3)
#define EEPROM_DEFAULT_ALRM_ONIMP ALARM_MANAGER_ONIMPULSE_VOICE_VIBRO_AND_VOICE


#define EEPROM_ADDR_ACCUM_DATEFORCELL(x) (EEPROM_ADDR_ACCUM + (x) * 8)
#define EEPROM_ADDR_ACCUM_VALUEFORCELL(x) (EEPROM_ADDR_ACCUM + (x) * 8 + 4)

void eeprom_control_migrate();

#if EEPROM_CONTROL_INIT_FAKE_BUFFER
void eeprom_control_init_fake_accum_buffer();
#endif

void eeprom_control_init() {
  uint8_t ver = EEPROM.read(EEPROM_ADDR_VERSION);
  if (ver == EEPROM_VERSION_UNINIT) {
    eeprom_control_init_default();
  } else if (ver != EEPROM_VERSION_ACTUAL) {
    eeprom_control_migrate();
  }

#if EEPROM_CONTROL_INIT_FAKE_BUFFER
  eeprom_control_init_fake_accum_buffer();
#endif
}

void eeprom_control_migrate() {
  eeprom_control_init_default();
}

void eeprom_control_init_default() {
  uint8_t value = EEPROM_VERSION_ACTUAL;
  EEPROM.update(EEPROM_ADDR_VERSION, value);
  
  eeprom_control_save_freq(EEPROM_DEFAULT_UV_FREQ);
  eeprom_control_save_duty(EEPROM_DEFAULT_UV_DUTY);

  eeprom_control_save_alarm_levels(EEPROM_DEFAULT_ALRM_L1, EEPROM_DEFAULT_ALRM_L2);
  eeprom_control_save_onimpulse_voice(EEPROM_DEFAULT_ALRM_ONIMP);

  eeprom_control_save_impl_per_ur(EEPROM_DEFAULT_IMPL_PER_UR);
}

void eeprom_control_save_freq(uint8_t freq) {
  EEPROM.put(EEPROM_ADDR_UV, freq);
}

void eeprom_control_save_duty(uint8_t duty) {
  EEPROM.put(EEPROM_ADDR_UV + 1, duty);
}

uint8_t eeprom_control_get_freq() {
  uint8_t val;
  EEPROM.get(EEPROM_ADDR_UV, val);
  if (val < 50 || val > 150) {
    val = EEPROM_DEFAULT_UV_FREQ;
  }
  return val;
}

uint8_t eeprom_control_get_duty() {
  uint8_t val;
  EEPROM.get(EEPROM_ADDR_UV + 1, val);
  if (val == 0 || val > 30) {
    val = EEPROM_DEFAULT_UV_DUTY;
  }
  return val;
}

void eeprom_control_save_impl_per_ur(uint16_t value) {
  EEPROM.put(EEPROM_ADDR_UV + 6, value);
}

uint16_t eeprom_control_get_impl_per_ur() {
  uint16_t b = 0;
  EEPROM.get(EEPROM_ADDR_UV + 6, b);
  if (b == 0xFFFF || b == 0x0000) {
    b = EEPROM_DEFAULT_IMPL_PER_UR;
  }

  return b;
}

void eeprom_control_get_alarm_levels(uint8_t & level1, uint8_t & level2) {
  EEPROM.get(EEPROM_ADDR_ALARM,     level1);
  EEPROM.get(EEPROM_ADDR_ALARM + 1, level2);

  if (level1 == 0x00 || level1 == 0xFF) {
    level1 = EEPROM_DEFAULT_ALRM_L1;
  }

  if (level2 == 0x00 || level2 == 0xFF) {
    level2 = EEPROM_DEFAULT_ALRM_L2;
  }
}

void eeprom_control_save_alarm_levels(uint8_t level1, uint8_t level2) {
  EEPROM.put(EEPROM_ADDR_ALARM,     level1);
  EEPROM.put(EEPROM_ADDR_ALARM + 1, level2);
}

uint8_t eeprom_control_get_noimpulse_seconds() {
  uint8_t res = 0;
  EEPROM.get(EEPROM_ADDR_ALARM + 2, res);
  if (res == 0x00 || res == 0xFF) {
    res = EEPROM_DEFAULT_ALRM_NOIMP;
  }
  return res;
}

void eeprom_control_set_noimpulse_seconds(uint8_t value) {
  EEPROM.put(EEPROM_ADDR_ALARM + 2, value);
}

void eeprom_control_save_onimpulse_voice(uint8_t value) {
  if (value > ALARM_MANAGER_ONIMPULSE_VOICE_MAXVALUE) {
    value = EEPROM_DEFAULT_ALRM_ONIMP;
  }

  EEPROM.put(EEPROM_ADDR_ALARM + 3, value);
}

uint8_t eeprom_control_get_onimpulse_voice() {
  uint8_t res = 0;
  EEPROM.get(EEPROM_ADDR_ALARM + 3, res);
  if (res == 0xFF || res > ALARM_MANAGER_ONIMPULSE_VOICE_MAXVALUE) {
    res = EEPROM_DEFAULT_ALRM_ONIMP;
  }
  return res;
}

void eeprom_control_get_date_time(volatile uint8_t & year, volatile uint8_t & month, volatile uint8_t & day, volatile uint8_t & hour, volatile uint8_t & minute) {
  uint32_t packed = 0;
  EEPROM.get(EEPROM_ADDR_DATE, packed);
  if (packed == 0 || packed == 0xFFFFFFFF) {
    return; // leave unchanged
  }

  DATE_UNPACK(packed, year, month, day, hour, minute);
}

void eeprom_control_save_date_time(uint8_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute) {
  uint32_t packed = 0;

  DATE_PACK(packed, year, month, day, hour, minute);

  EEPROM.put(EEPROM_ADDR_DATE, packed);
}

void eeprom_control_accum_resetbuffer(uint32_t datepacked) {
  uint32_t temp = 0xFFFFFFFF;
  for (uint8_t i = 1; i<EEPROM_CONTROL_ACCUMULATED_DOSE_STORE_POINTS; i++) {
    EEPROM.put(EEPROM_ADDR_ACCUM_DATEFORCELL(i), temp);
    EEPROM.put(EEPROM_ADDR_ACCUM_VALUEFORCELL(i), temp);
  }

  EEPROM.put(EEPROM_ADDR_ACCUM_DATEFORCELL(0), datepacked);

  temp = 0;
  EEPROM.put(EEPROM_ADDR_ACCUM_VALUEFORCELL(0), temp);
}

uint16_t eeprom_control_find_accum_for_date(uint32_t datepacked) {
  // memory is a ring-buffer for a EEPROM_CONTROL_ACCUMULATED_DOSE_STORE_POINTS cells.

  uint32_t temp = 0;

  // found cell for current month
  for (uint8_t i = 0; i<EEPROM_CONTROL_ACCUMULATED_DOSE_STORE_POINTS; i++) {
    EEPROM.get(EEPROM_ADDR_ACCUM_DATEFORCELL(i), temp);
    if (temp == datepacked) {
      return i;
    }
  }

  // check consistency - we cant store dates less than any
  EEPROM.get(EEPROM_ADDR_ACCUM_DATEFORCELL(0), temp);
  if (temp != 0xFFFFFFFF && temp > datepacked) {
    eeprom_control_accum_resetbuffer(datepacked);
    return 0;
  }

  // try to find next cell in ring buffer
  uint32_t prev = 0;
  EEPROM.get(EEPROM_ADDR_ACCUM_DATEFORCELL(0), prev);
  if (prev == 0xFFFFFFFF) {
    // buffer is empty, use first cell
    EEPROM.put(EEPROM_ADDR_ACCUM_DATEFORCELL(0), datepacked);
    temp = 0;
    EEPROM.put(EEPROM_ADDR_ACCUM_VALUEFORCELL(0), temp);
    return 0;
  }

  for (uint8_t i = 1; i<EEPROM_CONTROL_ACCUMULATED_DOSE_STORE_POINTS; i++) {
    EEPROM.get(EEPROM_ADDR_ACCUM_DATEFORCELL(i), temp);
    if (temp == 0xFFFFFFFF) {
      // found empty cell
      EEPROM.put(EEPROM_ADDR_ACCUM_DATEFORCELL(i), datepacked);
      temp = 0;
      EEPROM.put(EEPROM_ADDR_ACCUM_VALUEFORCELL(i), temp);
      return i;
    }

    if (temp > datepacked) {
      // inconsistency, reset buffer
      eeprom_control_accum_resetbuffer(datepacked);
      return 0;
    }

    if (temp > prev) {
      prev = temp;
    } else {
      // this is end of buffer, so  - we can write data here
      EEPROM.put(EEPROM_ADDR_ACCUM_DATEFORCELL(i), datepacked);
      temp = 0;
      EEPROM.put(EEPROM_ADDR_ACCUM_VALUEFORCELL(i), temp);
      return i;
    }
  }

  // end of buffer, this case means - we start from beginning
  EEPROM.put(EEPROM_ADDR_ACCUM_DATEFORCELL(0), datepacked);
  temp = 0;
  EEPROM.put(EEPROM_ADDR_ACCUM_VALUEFORCELL(0), temp);
  return 0;
}

void eeprom_control_on_save_rad_accum(uint32_t datepacked, uint32_t dose) {
  if (dose == 0) {
    return;
  }

  datepacked = DATE_PACKER__YYMM(datepacked);

  uint8_t cell = eeprom_control_find_accum_for_date(datepacked);
  uint32_t current = 0;
  EEPROM.get(EEPROM_ADDR_ACCUM_VALUEFORCELL(cell), current);
  if (current != 0xFFFFFFFF && current + dose > current) {
    current += dose;
    EEPROM.put(EEPROM_ADDR_ACCUM_VALUEFORCELL(cell), current);
  } else if (current != 0xFFFFFFFF) {
    current = 0xFFFFFFFF;
    EEPROM.put(EEPROM_ADDR_ACCUM_VALUEFORCELL(cell), current);
  }
}

bool eeprom_control_read_accumulated(uint8_t sortedindex, uint32_t & datepacked, uint32_t & value) {
  if (sortedindex > EEPROM_CONTROL_ACCUMULATED_DOSE_STORE_POINTS) {
    return false;    
  }

  // 1. find begin-of-ring-buffer
  uint32_t prev = 0;
  EEPROM.get(EEPROM_ADDR_ACCUM_DATEFORCELL(0), prev);
  if (prev == 0xFFFFFFFF || prev == 0) {
    return false;
  }

  uint8_t begin = 1;
  for (;begin < EEPROM_CONTROL_ACCUMULATED_DOSE_STORE_POINTS; begin++) {
    uint32_t current = 0;
    EEPROM.get(EEPROM_ADDR_ACCUM_DATEFORCELL(begin), current);
    if (current == 0xFFFFFFFF) {
      begin = 0;
      break; // end of buffer reached
    }

    if (current > prev) {
      prev = current;      
    } else {
      break;
    }
  }

  if (begin == EEPROM_CONTROL_ACCUMULATED_DOSE_STORE_POINTS) {
    // restart buffer from beginning
    begin = 0;
  }

  // get requested position
  begin += sortedindex;
  if (begin >= EEPROM_CONTROL_ACCUMULATED_DOSE_STORE_POINTS) {
    begin -= EEPROM_CONTROL_ACCUMULATED_DOSE_STORE_POINTS;
  }

  // try to read cell
  EEPROM.get(EEPROM_ADDR_ACCUM_DATEFORCELL(begin), prev);
  if (prev == 0xFFFFFFFF) {
    return false; // nodata in cell
  }

  datepacked = prev;
  EEPROM.get(EEPROM_ADDR_ACCUM_VALUEFORCELL(begin), value);
  return true;
}

#if EEPROM_CONTROL_INIT_FAKE_BUFFER
void eeprom_control_init_fake_accum_buffer() {
  eeprom_control_accum_resetbuffer(0xFFFFFFFF);

  for (uint8_t i = 0; i<7; i++) {
    uint32_t packed = 0;
    DATE_PACK(packed, 2023, i + 1, 0, 0, 0);
    eeprom_control_on_save_rad_accum(packed, i * 100 + 4444);
  }
}
#endif

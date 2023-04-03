#include "eeprom_control.h"

#include "EEPROM.h"

#define EEPROM_VERSION_ACTUAL 0x01
#define EEPROM_VERSION_UNINIT 0xFF

#define EEPROM_ADDR_VERSION   0x00
#define EEPROM_ADDR_UV        0x01
#define EEPROM_ADDR_ALARM     0x0F

#define EEPROM_DEFAULT_UV_FREQ 82000
#define EEPROM_DEFAULT_UV_DUTY 13

#define EEPROM_DEFAULT_ALRM_L1    100
#define EEPROM_DEFAULT_ALRM_L2    1000
#define EEPROM_DEFAULT_ALRM_NOIMP (60*3)

void eeprom_control_migrate(uint8_t from_ver);

void eeprom_control_init() {
  uint8_t ver = EEPROM.read(EEPROM_ADDR_VERSION);
  if (ver == EEPROM_VERSION_UNINIT) {
    eeprom_control_init_default();
  } else if (ver != EEPROM_VERSION_ACTUAL) {
    eeprom_control_migrate(ver);
  }
}

void eeprom_control_migrate(uint8_t from_ver) {
}

void eeprom_control_init_default() {
  EEPROM.update(EEPROM_ADDR_VERSION, EEPROM_VERSION_ACTUAL);
  
  eeprom_control_uv defuv = {
    .freq = EEPROM_DEFAULT_UV_FREQ,
    .duty = EEPROM_DEFAULT_UV_DUTY
  };
  eeprom_control_save_uv_freq(defuv);

  eeprom_control_save_alarm_levels(EEPROM_DEFAULT_ALRM_L1, EEPROM_DEFAULT_ALRM_L2);
}

void eeprom_control_get_uv(eeprom_control_uv & to) {
  EEPROM.get(EEPROM_ADDR_UV, to);
  if (to.freq == 0 || to.freq == 0xFFFFFFFF) {
    to.freq = EEPROM_DEFAULT_UV_FREQ;
  }
  if (to.duty == 0 || to.duty == 0xFF) {
    to.duty = EEPROM_DEFAULT_UV_DUTY;
  }
}

void eeprom_control_save_uv_freq(const eeprom_control_uv & data) {
  EEPROM.put(EEPROM_ADDR_UV, data);
}

void eeprom_control_get_alarm_levels(uint16_t & level1, uint16_t & level2) {
  EEPROM.get(EEPROM_ADDR_ALARM,     level1);
  EEPROM.get(EEPROM_ADDR_ALARM + 2, level2);

  if (level1 == 0x00 || level1 == 0xFFFF) {
    level1 = EEPROM_DEFAULT_ALRM_L1;
  }

  if (level2 == 0x00 || level2 == 0xFFFF) {
    level2 = EEPROM_DEFAULT_ALRM_L2;
  }
}

void eeprom_control_save_alarm_levels(uint16_t level1, uint16_t level2) {
  EEPROM.put(EEPROM_ADDR_ALARM,     level1);
  EEPROM.put(EEPROM_ADDR_ALARM + 2, level2);
}

uint16_t eeprom_control_get_noimpulse_seconds() {
  uint16_t res = 0;
  EEPROM.get(EEPROM_ADDR_ALARM + 4, res);
  if (res == 0x00 || res == 0xFFFF) {
    res = EEPROM_DEFAULT_ALRM_NOIMP;
  }
  return res;
}

void eeprom_control_set_noimpulse_seconds(uint16_t value) {
  EEPROM.put(EEPROM_ADDR_ALARM + 4, value);
}

#ifndef EEPROM_CONTROL_H_
#define EEPROM_CONTROL_H_

#include "stdint.h"

typedef struct {
  uint32_t freq;
  uint8_t duty;
} eeprom_control_uv;

void eeprom_control_init();
void eeprom_control_init_default();

void eeprom_control_get_uv(eeprom_control_uv & to);
void eeprom_control_save_uv_freq(const eeprom_control_uv & data);

void eeprom_control_get_alarm_levels(uint16_t & level1, uint16_t & level2);
void eeprom_control_save_alarm_levels(uint16_t level1, uint16_t level2);
uint16_t eeprom_control_get_noimpulse_seconds();
void eeprom_control_set_noimpulse_seconds(uint16_t value);

#endif /* EEPROM_CONTROL_H_ */

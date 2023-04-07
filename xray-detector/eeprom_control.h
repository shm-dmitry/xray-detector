#ifndef EEPROM_CONTROL_H_
#define EEPROM_CONTROL_H_

#include "stdint.h"

void eeprom_control_init();
void eeprom_control_init_default();

void eeprom_control_save_freq(uint8_t freq);
void eeprom_control_save_duty(uint8_t duty);
uint8_t eeprom_control_get_freq();
uint8_t eeprom_control_get_duty();

void eeprom_control_get_alarm_levels(uint8_t & level1, uint8_t & level2);
void eeprom_control_save_alarm_levels(uint8_t level1, uint8_t level2);
uint8_t eeprom_control_get_noimpulse_seconds();
void eeprom_control_set_noimpulse_seconds(uint8_t value);

void eeprom_control_get_date_time(volatile uint8_t & year, volatile uint8_t & month, volatile uint8_t & day, volatile uint8_t & hour, volatile uint8_t & minute);
void eeprom_control_save_date_time(uint8_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute);

#endif /* EEPROM_CONTROL_H_ */

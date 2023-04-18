#ifndef EEPROM_CONTROL_H_
#define EEPROM_CONTROL_H_

#include "stdint.h"

#define EEPROM_CONTROL_ACCUMULATED_DOSE_STORE_POINTS  12

void eeprom_control_init();
void eeprom_control_init_default();

void eeprom_control_save_freq(uint8_t freq);
uint8_t eeprom_control_get_freq();
void eeprom_control_save_duty(uint8_t duty);
uint8_t eeprom_control_get_duty();
uint16_t eeprom_control_get_uv_A();
void eeprom_control_save_uv_A(uint16_t a);
uint16_t eeprom_control_get_uv_B();
void eeprom_control_save_uv_B(uint16_t b);

void eeprom_control_get_alarm_levels(uint8_t & level1, uint8_t & level2);
void eeprom_control_save_alarm_levels(uint8_t level1, uint8_t level2);
uint8_t eeprom_control_get_noimpulse_seconds();
void eeprom_control_set_noimpulse_seconds(uint8_t value);

void eeprom_control_get_date_time(volatile uint8_t & year, volatile uint8_t & month, volatile uint8_t & day, volatile uint8_t & hour, volatile uint8_t & minute);
void eeprom_control_save_date_time(uint8_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute);

void eeprom_control_on_save_rad_accum(uint32_t datepacked, uint32_t dose);
bool eeprom_control_read_accumulated(uint8_t sortedindex, uint32_t & datepacked, uint32_t & value);

#endif /* EEPROM_CONTROL_H_ */

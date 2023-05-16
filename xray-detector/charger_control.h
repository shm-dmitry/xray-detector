#ifndef CHARGER_CONTROL_H_
#define CHARGER_CONTROL_H_

#include "stdint.h"

// PB1 / PCINT1
#define CHARGER_INT_PIN    9

typedef struct {
  uint16_t bat_voltage_x100;
  uint8_t bat_current_x10;
  bool i2c_in_use;

  // ?..
} t_charger_data;

void charger_control_init();
void charger_control_on_main_loop();
void charger_control_enter_sleep_mode();
void charger_control_leave_sleep_mode();

bool charger_control_get_data(t_charger_data & data);
bool charger_control_read_adc(t_charger_data & data);

uint8_t charger_control_get_voltage_pc();
bool charger_control_is_active();

void isrcall_charger_control_onusbint();

#endif /* CHARGER_CONTROL_H_ */

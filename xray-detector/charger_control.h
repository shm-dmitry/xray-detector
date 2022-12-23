#ifndef CHARGER_CONTROL_H_
#define CHARGER_CONTROL_H_

#include "stdint.h"

// PB1 / PCINT1
#define CHARGER_INT_PIN    9

typedef struct {
  uint8_t bat_voltage_x10;
  // ?..
} t_charger_data;

void charger_control_init();
void charger_control_on_main_loop();
void charger_control_enter_sleep_mode();

bool charger_control_get_data(t_charger_data & data);

void isrcall_charger_control_onusbint();

#endif /* CHARGER_CONTROL_H_ */

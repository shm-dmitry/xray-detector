#pragma once

#include "stdint.h"

#define RAD_DATA_MASK__SBT9_CURRENT_UPDATED  0b00000001
#define RAD_DATA_MASK__SBT9_HIST_UPDATED     0b00000010
#define RAD_DATA_MASK__SBM20_CURRENT_UPDATED 0b00000100
#define RAD_DATA_MASK__SBM20_HIST_UPDATED    0b00001000
#define RAD_DATA_MASK__ALERT                 0b01000000
#define RAD_DATA_MASK                        0b00001111

void rad_data_init();
void rad_data_on_main_loop();

void isrcall__rad_data_on_one_second();
bool isrcall__rad_data_is_alert();
uint8_t isrcall__rad_data_decimals();
uint8_t isrcall__rad_data_buffer_pointer(bool sbt9, uint8_t index, uint8_t maxelements, uint32_t ** ptr);
uint8_t isrcall__rad_data_get_mask();
void isrcall__rad_data_set_mask(uint8_t mask);
bool isrcall__rad_data_allow_sleep();

void rad_data_getlast(uint32_t * sbt9, uint32_t * sbm20, uint8_t * decimals);


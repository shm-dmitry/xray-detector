#pragma once

#include "stdint.h"

void alert_manager_on_wakeup();
uint8_t alert_manager_getlevel(uint32_t value);
void alert_manager_set_level_value(uint32_t level1, uint32_t level2);
void alert_manager_get_level_value(uint32_t * level1, uint32_t * level2);

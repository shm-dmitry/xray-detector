#pragma once

#include "stdint.h"

void ext_clock_ctrl_init();
uint32_t ext_clock_millis();
uint32_t isrcall__ext_clock_millis();

void ext_clock_go_sleep();
void ext_clock_wake_up();

#pragma once

#include "stdint.h"

#define NRAD_COMMAND_ACTIVE true

void rad_control_init();
void rad_control_on_main_loop();

void isrcall__rad_control_on_one_second();
void isrcall__rad_control_get_data(uint32_t * sbt9, uint32_t * sbm20, bool reset);

#if NRAD_COMMAND_ACTIVE
void rad_control_get_total_counters(uint32_t * sbt9, uint32_t * sbm20, bool reset);
#endif
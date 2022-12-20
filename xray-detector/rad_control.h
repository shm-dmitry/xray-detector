#ifndef RAD_CONTROL_H_
#define RAD_CONTROL_H_

#include "stdint.h"

void rad_control_init();

void isrcall_rad_control_on_timer(uint8_t seconds);

uint32_t rad_control_impulses_last_minute(uint8_t minute);

uint32_t rad_control_dose();

void rad_control_on_main_loop();

#endif /* RAD_CONTROL_H_ */

#ifndef RAD_CONTROL_H_
#define RAD_CONTROL_H_

#include "stdint.h"

void rad_control_init();

uint32_t rad_control_dose(bool isrcall = false);

void isrcall_rad_control_on_timer(uint8_t seconds);

#endif /* RAD_CONTROL_H_ */

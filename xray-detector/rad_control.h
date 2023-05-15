#ifndef RAD_CONTROL_H_
#define RAD_CONTROL_H_

#include "stdint.h"

#define RAD_CONTROL_USER_COUNTER_DISABLED  0xFFFF
#define RAD_CONTROL_USER_COUNTER_MAXVAL    0xFFFE

void rad_control_init();

void rad_control_refresh_impl_per_ur();

uint32_t rad_control_dose();

bool isrcall_rad_control_check_dose_alarm(uint32_t mindose, uint32_t millis);
void isrcall_rad_control_on_timer(uint8_t seconds);

uint16_t rad_control_user_counter_getvalue();
void rad_control_user_counter_startstop(bool start);

#endif /* RAD_CONTROL_H_ */

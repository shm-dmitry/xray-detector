#ifndef UV_CONTROL_H_
#define UV_CONTROL_H_

#include "stddef.h"
#include "stdint.h"

void uv_control_init();

// commands
bool uv_control_update_pwm(uint8_t freq, uint8_t duty);
void uv_control_enable_pwm();
void uv_control_disable_pwm();
bool uv_control_is_on();

void uv_control_change_pwm_with_testrun(uint8_t freq, uint8_t duty);

// callbacks

void isrcall_uv_control_on_impulse();
void isrcall_uv_control_on_timer();
bool isrcall_uv_control_is_initialized();

#endif /* UV_CONTROL_H_ */

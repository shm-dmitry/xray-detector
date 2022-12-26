#ifndef UV_CONTROL_H_
#define UV_CONTROL_H_

#include "stddef.h"
#include "stdint.h"

void uv_control_init();

// commands

inline void uv_control_enable_pwm();
inline void uv_control_disable_pwm();

void uv_control_change_pwm_with_testrun(uint32_t freq, uint8_t duty);

// callbacks

void isrcall_uv_control_on_impulse();
void isrcall_uv_control_on_timer();
bool isrcall_uv_control_is_initialized();

#endif /* UV_CONTROL_H_ */

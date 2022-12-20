#ifndef UV_CONTROL_H_
#define UV_CONTROL_H_

#include "stddef.h"
#include "stdint.h"

#define UV_CONTROL_TEST_MANUAL  true

void uv_control_init();

// commands

inline void uv_control_enable_pwm();
inline void uv_control_disable_pwm();

#if UV_CONTROL_TEST_MANUAL
void uv_control_manualpwm_correct();
#endif

// callbacks

void isrcall_uv_control_on_impulse();
void isrcall_uv_control_on_timer();

#endif /* UV_CONTROL_H_ */

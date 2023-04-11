#ifndef RAD_HISTORY_H_
#define RAD_HISTORY_H_

#include "stdint.h"

#define RAD_HISTORY_STORE_MINUTE_POINTS  20

typedef struct t_rad_history_minute_points {
  uint32_t dose;
  uint32_t datepacked;
} t_rad_history_minute_points;

typedef t_rad_history_minute_points t_rad_history_minute_points_buffer[RAD_HISTORY_STORE_MINUTE_POINTS];

void isrcall_rad_history_on_second(uint8_t seconds);

void rad_history_on_main_loop();

const t_rad_history_minute_points_buffer * rad_history_get_minute_points_buffer();
bool rad_history_was_minute_buffer_changed();

#endif /* RAD_HISTORY_H_ */

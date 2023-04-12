#ifndef RAD_ACCUM_HISTORY_H_
#define RAD_ACCUM_HISTORY_H_

#include "stdint.h"

void rad_accum_history_ondose(uint32_t dose);

uint32_t rad_accum_get_daily_dose_usv();
bool rad_accum_history_read(uint8_t sortedindex, uint32_t & datepacked, uint32_t & value);
uint8_t rad_accum_history_points_count();

#endif /* RAD_ACCUM_HISTORY_H_ */

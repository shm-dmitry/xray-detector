#ifndef SVF_CONTROL_H_
#define SVF_CONTROL_H_

#include "stdint.h"

// svf == speaker, vibro, flash

void svf_control_init();

void svf_control_play_sound__impuls();
void svf_control_play_sound__alarm();

void svf_control_play_vibro__impuls();
void svf_control_play_vibro__alarm();

void svf_control_flash(uint8_t percent);

void isrcall_svf_control_on_millis();
void svf_control_on_main_loop();

#endif /* NVF_CONTROL_ */

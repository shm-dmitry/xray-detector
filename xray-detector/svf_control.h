#ifndef SVF_CONTROL_H_
#define SVF_CONTROL_H_

#include "stdint.h"

// svf == speaker, vibro, flash

void svf_control_init();

void svf_control_play_sound__impuls();
void svf_control_play_sound__alarm1();
void svf_control_play_sound__alarm2();
void svf_control_play_sound__alarm3();

void svf_control_play_vibro__impuls();
void svf_control_play_vibro__alarm1();
void svf_control_play_vibro__alarm2();
void svf_control_play_vibro__alarm3();

void svf_control_flash(uint8_t percent);

void svf_control_sound_mute();
void svf_control_vibro_mute();

void isrcall_svf_control_on_millis();
void svf_control_on_main_loop();

#endif /* NVF_CONTROL_ */

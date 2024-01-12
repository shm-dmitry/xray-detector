#include "svf_control.h"

#include "Arduino.h"

#include "svf_control_vibro_models.h"
#include "svf_control_speaker_models.h"

// 3 == PD3
#define PIN_SPEAKER   3
#define PIN_VIBRO     A1
#define PIN_FLASH     5

#define SVF_CONTROL_SPEAKER_VOLUME 30

#define SVF_SPEAKER_SINGLE_IMPL   1
#define SVF_SPEAKER_ALARM1        2
#define SVF_SPEAKER_ALARM2        3
#define SVF_SPEAKER_ALARM3        4
#define SVF_VIBRO_SINGLE_IMPL     1
#define SVF_VIBRO_ALARM1          2
#define SVF_VIBRO_ALARM2          3
#define SVF_VIBRO_ALARM3          4

typedef struct {
  uint16_t samples_count;
  uint16_t current_sample;
  uint16_t play_time_millis;
  uint8_t  id;
} t_data_sample;

inline void svf_control_timer_stop();
void svf_control_mute(volatile t_data_sample & obj);

volatile t_data_sample svf_speaker = { 
  .samples_count = 0,
  .current_sample = 0,
  .play_time_millis = 0,
  .id = 0
};
volatile t_data_sample svf_vibro   = { 
  .samples_count = 0,
  .current_sample = 0,
  .play_time_millis = 0,
  .id = 0
};

void svf_control_init() {
  pinMode(PIN_SPEAKER, OUTPUT);
  digitalWrite(PIN_SPEAKER, LOW);
  
  pinMode(PIN_VIBRO, OUTPUT);
  digitalWrite(PIN_VIBRO, LOW);
  
  pinMode(PIN_FLASH, OUTPUT);
  digitalWrite(PIN_FLASH, LOW);

  svf_control_timer_stop();
}

inline void svf_control_timer_stop() {
  TCCR2A = 0;
  TCCR2B = 0;
  OCR2A  = 0;
  OCR2B  = 0;
}

inline void svf_control_timer_start() {
  TCCR2A = _BV(WGM20);
  TCCR2B = _BV(WGM22) | _BV(CS22) | _BV(CS21);
  TCNT2  = 0;
}

inline void svf_control_vibro_stop() {
  digitalWrite(PIN_VIBRO, LOW);
}

inline void svf_control_vibro_play() {
  digitalWrite(PIN_VIBRO, HIGH);
}

inline void svf_control_speaker_stop() {
  TCCR2A &= ~(_BV(COM2B1));
  digitalWrite(PIN_SPEAKER, LOW);
  svf_control_timer_stop();
}

inline void svf_control_speaker_freq(uint16_t freq) {
  svf_control_speaker_stop();
  if (freq > 0) {
    OCR2A = F_CPU / 2 / 256 / freq;
    OCR2B = ((uint32_t)OCR2A * (uint32_t)SVF_CONTROL_SPEAKER_VOLUME) / 100;
    
    TCCR2A |= _BV(COM2B1);
    svf_control_timer_start();
  }
}

void svf_control_vibro_next_frame() {
  svf_vibro.current_sample++;
  if (svf_vibro.current_sample > svf_vibro.samples_count) {
    svf_control_vibro_stop();
    svf_vibro.id = 0;
  } else {
    svf_vibro.play_time_millis = pgm_read_word(&SVF_CONTROL_MODELS_VIBRO[svf_vibro.id - 1][(svf_vibro.current_sample - 1) * 2 + 2]);
    if (pgm_read_word(&SVF_CONTROL_MODELS_VIBRO[svf_vibro.id - 1][(svf_vibro.current_sample - 1) * 2 + 1])) {
      svf_control_vibro_play();
    } else {
      svf_control_vibro_stop();
    }
  }
}

void svf_control_speaker_next_frame() {
  svf_speaker.current_sample++;
  if (svf_speaker.current_sample > svf_speaker.samples_count) {
    svf_control_speaker_stop();
    svf_speaker.id = 0;
  } else {
    svf_speaker.play_time_millis = pgm_read_word(&SVF_CONTROL_MODELS_SPEAKER[svf_speaker.id - 1][(svf_speaker.current_sample - 1) * 2 + 2]);
    svf_control_speaker_freq(pgm_read_word(&SVF_CONTROL_MODELS_SPEAKER[svf_speaker.id - 1][(svf_speaker.current_sample - 1) * 2 + 1]));
  }
}

void isrcall_svf_control_on_millis() {
  if (svf_speaker.id > 0 && svf_speaker.play_time_millis > 0) {
    svf_speaker.play_time_millis--;
  }

  if (svf_vibro.id > 0 && svf_vibro.play_time_millis > 0) {
    svf_vibro.play_time_millis--;
  }
}

bool svf_control_play(volatile t_data_sample & obj, uint8_t id, const uint16_t * const* progmem_data) {
  // priority
  if (obj.id < id) {
    svf_control_mute(obj);

    obj.current_sample   = 0;
    obj.samples_count    = pgm_read_word(&progmem_data[id - 1][0]);
    obj.play_time_millis = 0;
    obj.id = id; // must be last call in initialization!

    return true;
  } else {
    return false;
  }
}

void svf_control_play_sound(uint8_t id) {
  if (svf_control_play(svf_speaker, id, SVF_CONTROL_MODELS_SPEAKER)) {
    svf_control_speaker_next_frame();
  }
}

void svf_control_play_vibro(uint8_t id) {
  if (svf_control_play(svf_vibro, id, SVF_CONTROL_MODELS_VIBRO)) {
    svf_control_vibro_next_frame();
  }
}

void svf_control_play_sound__impuls() {
  svf_control_play_sound(SVF_SPEAKER_SINGLE_IMPL);
}

void svf_control_play_sound__alarm1() {
  svf_control_play_sound(SVF_SPEAKER_ALARM1);
}

void svf_control_play_sound__alarm2() {
  svf_control_play_sound(SVF_SPEAKER_ALARM2);
}

void svf_control_play_sound__alarm3() {
  svf_control_play_sound(SVF_SPEAKER_ALARM3);
}

void svf_control_play_vibro__impuls() {
  svf_control_play_vibro(SVF_VIBRO_SINGLE_IMPL);
}

void svf_control_play_vibro__alarm1() {
  svf_control_play_vibro(SVF_VIBRO_ALARM1);
}

void svf_control_play_vibro__alarm2() {
  svf_control_play_vibro(SVF_VIBRO_ALARM2);
}

void svf_control_play_vibro__alarm3() {
  svf_control_play_vibro(SVF_VIBRO_ALARM3);
}

void svf_control_mute(volatile t_data_sample & obj) {
  if (obj.id > 0) {
    obj.current_sample   = obj.samples_count;
    obj.play_time_millis = 1;
    while (obj.play_time_millis != 0);
  }
}

void svf_control_sound_mute() {
  svf_control_mute(svf_speaker);
}

void svf_control_vibro_mute() {
  svf_control_mute(svf_vibro);
}

uint16_t svf_control_read_play_time(volatile t_data_sample & obj) {
  uint8_t oldSREG = SREG;
  cli();
  uint16_t val = obj.play_time_millis;
  SREG = oldSREG;
  return val;
}

void svf_control_on_main_loop() {
  if (svf_speaker.id > 0 && svf_control_read_play_time(svf_speaker) == 0) {
    svf_control_speaker_next_frame();
  }

  if (svf_vibro.id > 0 && svf_control_read_play_time(svf_vibro) == 0) {
    svf_control_vibro_next_frame();
  }
}

void svf_control_flash(uint8_t percent) {
  if (percent == 0) {
    digitalWrite(PIN_FLASH, LOW);
    TCCR0A &= ~(_BV(COM0B1));
  } else if (percent >= 100) {
    TCCR0A &= ~(_BV(COM0B1));
    digitalWrite(PIN_FLASH, HIGH);
  } else {
    digitalWrite(PIN_FLASH, LOW);
    TCCR0A &= ~(_BV(COM0B1));
    
    OCR0B = (OCR0A * percent) / 100;

    if (OCR0B == 0) {
      // nothing to do
    } else if (OCR0B < OCR0A) {
      TCCR0A |= _BV(COM0B1);
    } else {
      digitalWrite(PIN_FLASH, HIGH);
    }
  }
}

void svf_control_stop() {
  svf_control_flash(0); 

  svf_control_sound_mute();
  svf_control_vibro_mute();

  svf_control_vibro_stop();
  svf_control_speaker_stop();
}

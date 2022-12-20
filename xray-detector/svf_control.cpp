#include "svf_control.h"

#include "Arduino.h"

#include "svf_control_vibro_models.h"
#include "svf_control_speaker_models.h"

#define PIN_SPEAKER   10
#define PIN_VIBRO     PD6
#define PIN_FLASH     PD5

#define SVF_CONTROL_SPEAKER_VOLUME 30

#define SVF_SPEAKER_SINGLE_IMPL   0
#define SVF_SPEAKER_ALARM         1
#define SVF_VIBRO_SINGLE_IMPL     0
#define SVF_VIBRO_ALARM           1

inline void svf_control_timer_stop();

typedef struct {
  uint16_t samples_count;
  uint16_t current_sample;
  uint16_t play_time_millis;
  uint8_t  id;
} t_data_sample;

volatile t_data_sample svf_speaker = { 0 };
volatile t_data_sample svf_vibro   = { 0 };

void svf_control_init() {
  pinMode(PIN_SPEAKER, OUTPUT);
  digitalWrite(PIN_SPEAKER, LOW);
  
  pinMode(PIN_VIBRO, OUTPUT);
  digitalWrite(PIN_VIBRO, LOW);
  
  pinMode(PIN_FLASH, OUTPUT);
  digitalWrite(PIN_FLASH, LOW);

  TCCR1A = _BV(WGM11) | _BV(WGM10);
  svf_control_timer_stop();
  OCR1A  = 0;
  OCR1B  = 0;
}

inline void svf_control_timer_stop() {
  TCCR1B = 0;
}

inline void svf_control_timer_start() {
  TCCR1B = _BV(WGM13) | _BV(CS10); // F_CPU / 256
}

inline void svf_control_vibro_stop() {
  digitalWrite(PIN_VIBRO, LOW);
}

inline void svf_control_vibro_play() {
  digitalWrite(PIN_VIBRO, HIGH);
}

inline void svf_control_speaker_stop() {
  TCCR1A &= ~(_BV(COM1B1));
  svf_control_timer_stop();
}

inline void svf_control_speaker_freq(uint16_t freq) {
  svf_control_speaker_stop();
  if (freq > 0) {
    OCR1A = F_CPU / 2 / freq;
    OCR1B = (OCR1A * SVF_CONTROL_SPEAKER_VOLUME) / 100;
    TCCR1A |= _BV(COM1B1);
    svf_control_timer_start();
  }
}

void svf_control_vibro_next_frame() {
  svf_vibro.current_sample++;
  if (svf_vibro.current_sample == svf_vibro.samples_count) {
    svf_control_vibro_stop();
    svf_vibro.id = 0;
  } else {
    svf_vibro.play_time_millis = pgm_read_word(&SVF_CONTROL_MODELS_VIBRO[svf_vibro.id - 1][(svf_vibro.current_sample - 1) * 2 + 2]);
    if (pgm_read_word(&SVF_CONTROL_MODELS_VIBRO[svf_vibro.id - 1][(svf_vibro.current_sample - 1) * 2 + 1])) {
      svf_control_vibro_stop();
    } else {
      svf_control_vibro_play();
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

bool svf_control_play(volatile t_data_sample & obj, uint8_t id, const uint16_t ** progmem_data) {
  // priority
  if (obj.id < id) {
    if (obj.id > 0) {
      // request for a stop!
      obj.current_sample   = obj.samples_count;
      obj.play_time_millis = 1;
      while (obj.id != 0);
    }

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

void svf_control_play_sound__alarm() {
  svf_control_play_sound(SVF_SPEAKER_ALARM);
}

void svf_control_play_vibro__impuls() {
  svf_control_play_vibro(SVF_VIBRO_SINGLE_IMPL);
}

void svf_control_play_vibro__alarm() {
  svf_control_play_vibro(SVF_VIBRO_ALARM);
}

uint32_t svf_control_read_play_time(volatile t_data_sample & obj) {
  uint8_t oldSREG = SREG;
  cli();
  uint32_t val = obj.play_time_millis;
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

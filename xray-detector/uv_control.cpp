#include "uv_control.h"

#include "Arduino.h"

#define UV_CONTROL_PIN_PWM    PD3

#define UV_CONTROL_INITIAL_FREQ 82000
#define UV_CONTROL_INITIAL_DUTY 13

#define UV_CONTROL_REFILL_EVERY 50
#define UV_CONTROL_REFILL_REQ   250
#define UV_CONTROL_REFILL_DUR   1
#define UV_CONTROL_REFILL_INIT  10

static volatile uint8_t uv_control_refill_sec = UV_CONTROL_REFILL_INIT;
static volatile uint8_t uv_control_impulses   = UV_CONTROL_REFILL_REQ;

bool uv_control_update_pwm(uint32_t freq, uint8_t duty);
inline void uv_control_enable_pwm();
inline void uv_control_disable_pwm();

void uv_control_init() {
  pinMode(UV_CONTROL_PIN_PWM,      OUTPUT);
  digitalWrite(UV_CONTROL_PIN_PWM, LOW);

  // configure timer on UV_CONTROL_PIN_PWM
  TCCR2A = _BV(WGM20);
  uv_control_disable_pwm();

  // configure PWM
  uv_control_update_pwm(UV_CONTROL_INITIAL_FREQ, UV_CONTROL_INITIAL_DUTY);

#if UV_CONTROL_TEST_MANUAL
  pinMode(A0, OUTPUT);
  pinMode(A2, INPUT);
  pinMode(A4, OUTPUT);

  digitalWrite(A0, LOW);
  digitalWrite(A4, HIGH);

  uv_control_manualpwm_correct();
#endif

  uv_control_enable_pwm();
}

#if UV_CONTROL_TEST_MANUAL
void uv_control_manualpwm_correct() {
  long value = analogRead(A2);
  value = (value * 100) / 1024;

  long nextOCR2B = OCR2A;
  nextOCR2B = (nextOCR2B * value) / 100;
  if (nextOCR2B < 0) {
    nextOCR2B = 1;
  }
  if (nextOCR2B >= OCR2A) {
    nextOCR2B = OCR2A - 1;
  }

  if (nextOCR2B != OCR2B) {
    Serial.print("Change PWM duty to ");
    Serial.println(value);
    OCR2B = nextOCR2B;
  }
}
#endif

inline void uv_control_enable_pwm() {
  TCCR2A |= _BV(COM2B1);
  TCCR2B = _BV(WGM22) | _BV(CS20);
}

inline void uv_control_disable_pwm() {
  TCCR2A &= ~(_BV(COM2B1));
  TCCR2B = 0;
}

bool uv_control_update_pwm(uint32_t freq, uint8_t duty) {
  Serial.print("uv_control_update_pwm : ");
  Serial.print(freq);
  Serial.print(" / ");
  Serial.println(duty);
  
  OCR2A  = F_CPU / 2 / freq;

  uint8_t prev = OCR2B;
  OCR2B = (OCR2A * duty) / 100;
  return prev != OCR2B;
}

void isrcall_uv_control_on_impulse() {
  if (uv_control_impulses < UV_CONTROL_REFILL_REQ) {
    if (uv_control_impulses++ >= UV_CONTROL_REFILL_EVERY) {
      uv_control_impulses = UV_CONTROL_REFILL_REQ;
    }
  }
}

void isrcall_uv_control_on_timer() {
  if (uv_control_impulses >= UV_CONTROL_REFILL_REQ) {
    if (uv_control_refill_sec == 0) {
      uv_control_refill_sec = UV_CONTROL_REFILL_DUR;

      uv_control_enable_pwm();
    } else {
      if (uv_control_refill_sec == 1) {
        uv_control_disable_pwm();
        uv_control_impulses = 0;
        uv_control_refill_sec = 0;
      } else {
        uv_control_refill_sec--;
      }
    }
  }
}

#include "uv_control.h"
#include "eeprom_control.h"
#include "rad_control.h"

#include "Arduino.h"

#define UV_HIGH_VOLTAGE_OUTPUT_ENABLED false

#define UV_CONTROL_PIN_PWM    PD3

#define UV_CONTROL_REFILL_EVERY 50
#define UV_CONTROL_REFILL_REQ   250
#define UV_CONTROL_REFILL_REQRS 251
#define UV_CONTROL_REFILL_DUR   1
#define UV_CONTROL_REFILL_INIT  10

#define UV_CONTROL_TESTRUN_DUR  5

static volatile uint8_t uv_control_refill_sec = UV_CONTROL_REFILL_INIT;
static volatile uint8_t uv_control_impulses   = UV_CONTROL_REFILL_REQRS;


void uv_control_enable_pwm();
void uv_control_disable_pwm();

void uv_control_init() {
  pinMode(UV_CONTROL_PIN_PWM,      OUTPUT);
  digitalWrite(UV_CONTROL_PIN_PWM, LOW);

  // configure timer on UV_CONTROL_PIN_PWM
  TCCR2A = _BV(WGM20);
  uv_control_disable_pwm();

  // configure PWM
  eeprom_control_uv def = { 0 };
  eeprom_control_get_uv(def);
  uv_control_update_pwm(def.freq, def.duty);
  uv_control_enable_pwm();
}

void uv_control_enable_pwm() {
  #if UV_HIGH_VOLTAGE_OUTPUT_ENABLED
  TCCR2A |= _BV(COM2B1);
  TCCR2B = _BV(WGM22) | _BV(CS20);
  #endif
}

void uv_control_disable_pwm() {
  TCCR2A &= ~(_BV(COM2B1));
  TCCR2B = 0;
  digitalWrite(UV_CONTROL_PIN_PWM, LOW);
}

bool uv_control_is_on() {
  return TCCR2B != 0;
}

void uv_control_change_pwm_with_testrun(uint32_t freq, uint8_t duty) {
  uv_control_refill_sec = 1;
  uv_control_impulses = 0;

  uv_control_disable_pwm();
  uv_control_update_pwm(freq, duty);
  uv_control_enable_pwm();

  uv_control_refill_sec = UV_CONTROL_TESTRUN_DUR;
  uv_control_impulses = UV_CONTROL_REFILL_REQ;
}


bool uv_control_update_pwm(uint32_t freq, uint8_t duty) {
  Serial.print("Set FREQ = ");
  Serial.print(freq);
  Serial.print("; DUTY = ");
  Serial.println(duty);
  OCR2A  = F_CPU / 2 / freq;

  uint8_t prev = OCR2B;
  OCR2B = (OCR2A * duty) / 100;
  return prev != OCR2B;
}

bool isrcall_uv_control_is_initialized() {
  return uv_control_impulses != UV_CONTROL_REFILL_REQRS;
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

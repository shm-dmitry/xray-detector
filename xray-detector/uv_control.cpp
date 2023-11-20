#include "uv_control.h"
#include "eeprom_control.h"
#include "rad_control.h"
#include "config.h"
#include "charger_control.h"
#include "clock.h"

#include "Arduino.h"

#define UV_CONTROL_PIN_PWM    PD3
#define UV_CONTROL_ENABLE     A2

#define UV_CONTROL_REFILL_EVERY 50
#define UV_CONTROL_REFILL_REQ   250
#define UV_CONTROL_REFILL_REQRS 251
#define UV_CONTROL_REFILL_DUR   1
#define UV_CONTROL_REFILL_INIT  10

#define UV_CONTROL_TESTRUN_DUR  5

static volatile uint8_t uv_control_refill_sec   = UV_CONTROL_REFILL_INIT;
static volatile uint8_t uv_control_impulses     = UV_CONTROL_REFILL_REQRS;
static volatile uint8_t uv_control_duty         = 10;
static volatile uint32_t uv_control_last_run_uv = 0;
static volatile uint8_t uv_control_seconds_between_uv = 0;

void uv_control_enable_pwm();
void uv_control_disable_pwm();

void uv_control_init() {
  pinMode(UV_CONTROL_PIN_PWM,      OUTPUT);
  digitalWrite(UV_CONTROL_PIN_PWM, LOW);

  pinMode(UV_CONTROL_ENABLE, OUTPUT);
  digitalWrite(UV_CONTROL_ENABLE, HIGH);

  // configure timer on UV_CONTROL_PIN_PWM
  TCCR1A = _BV(WGM10) | _BV(WGM11);
  uv_control_disable_pwm();

  // configure PWM
  uv_control_update_pwm(eeprom_control_get_freq(), eeprom_control_get_duty());
  uv_control_enable_pwm();
}

void uv_control_enable_pwm() {
  #if UV_HIGH_VOLTAGE_OUTPUT_ENABLED
  digitalWrite(UV_CONTROL_ENABLE, LOW);

  TCCR1A |= _BV(COM1B1);
  TCCR1B = _BV(WGM13) | _BV(CS10);
  #endif
}

void uv_control_disable_pwm() {
  digitalWrite(UV_CONTROL_ENABLE, HIGH);

  TCCR1A &= ~(_BV(COM1B1));
  TCCR1B = 0;
  digitalWrite(UV_CONTROL_PIN_PWM, LOW);
}

bool uv_control_is_on() {
  return TCCR1B != 0;
}

void uv_control_change_pwm_with_testrun_freq(uint8_t freq, uint8_t duty) {
  uv_control_refill_sec = 1;
  uv_control_impulses = 0;

  uv_control_disable_pwm();
  uv_control_update_pwm(freq, duty);
  uv_control_enable_pwm();

  uv_control_refill_sec = UV_CONTROL_TESTRUN_DUR;
  uv_control_impulses = UV_CONTROL_REFILL_REQ;
}

bool uv_control_update_pwm(uint8_t freq, uint8_t duty) {
  OCR1A  = F_CPU / 2 / freq / 1000;

  uv_control_duty = duty;

  uint16_t prev = OCR1B;
  OCR1B = (OCR1A * duty) / 100;
  return prev != OCR1B;
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

      uint32_t now = clock_millis(true);
      if (uv_control_last_run_uv > 0 && now > uv_control_last_run_uv) {
        uint32_t temp = (now - uv_control_last_run_uv) / 1000;
        if (temp > 0xFF) {
          temp = 0xFF;
        }
        
        uv_control_seconds_between_uv = temp;
      }

      uv_control_last_run_uv = now;

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

uint8_t uv_control_seconds_between_uv_on() {
  return uv_control_seconds_between_uv;
}

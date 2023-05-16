#include "uv_control.h"
#include "eeprom_control.h"
#include "rad_control.h"
#include "config.h"
#include "charger_control.h"
#include "clock.h"

#include "Arduino.h"

#define UV_CONTROL_PIN_PWM    PD3

#define UV_CONTROL_REFILL_EVERY 50
#define UV_CONTROL_REFILL_REQ   250
#define UV_CONTROL_REFILL_REQRS 251
#define UV_CONTROL_REFILL_DUR   1
#define UV_CONTROL_REFILL_INIT  10

#define UV_CONTROL_APX_A 5463
#define UV_CONTROL_APX_B 1471

#define UV_CONTROL_APPROX_MINFREQ 80000
#define UV_CONTROL_APPROX_MAXFREQ 150000

#define UV_CONTROL_TESTRUN_DUR  5

#define UV_CONTROL_RECALC_APPROX_FREQ_EVERY 1000

static volatile uint8_t uv_control_refill_sec   = UV_CONTROL_REFILL_INIT;
static volatile uint8_t uv_control_impulses     = UV_CONTROL_REFILL_REQRS;
static volatile uint8_t uv_control_duty         = 10;
static volatile uint32_t uv_control_recalc_freq = 0;
static volatile uint32_t uv_control_last_run_uv = 0;
static volatile uint8_t uv_control_seconds_between_uv = 0;

void uv_control_enable_pwm();
void uv_control_disable_pwm();
bool uv_control_update_pwm_approx_linear();

void uv_control_init() {
  pinMode(UV_CONTROL_PIN_PWM,      OUTPUT);
  digitalWrite(UV_CONTROL_PIN_PWM, LOW);

  // configure timer on UV_CONTROL_PIN_PWM
  TCCR2A = _BV(WGM20);
  uv_control_disable_pwm();

  // configure PWM
  #if UV_LINEAR_APPROX_FREQ_ENABLED
  uv_control_update_pwm_approx_linear();
  #else
  uv_control_update_pwm(eeprom_control_get_freq(), eeprom_control_get_duty());
  #endif

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

void uv_control_change_pwm_with_testrun_freq(uint8_t freq, uint8_t duty) {
  uv_control_refill_sec = 1;
  uv_control_impulses = 0;

  uv_control_disable_pwm();
  uv_control_update_pwm(freq, duty);
  uv_control_enable_pwm();

  uv_control_refill_sec = UV_CONTROL_TESTRUN_DUR;
  uv_control_impulses = UV_CONTROL_REFILL_REQ;
}

void uv_control_change_pwm_with_testrun_approx() {
  uv_control_refill_sec = 1;
  uv_control_impulses = 0;

  uv_control_disable_pwm();
  uv_control_update_pwm_approx_linear();
  uv_control_enable_pwm();

  uv_control_refill_sec = UV_CONTROL_TESTRUN_DUR;
  uv_control_impulses = UV_CONTROL_REFILL_REQ;
}

bool uv_control_update_pwm(uint8_t freq, uint8_t duty) {
  OCR2A  = F_CPU / 2 / freq / 1000;

  uv_control_duty = duty;

  uint8_t prev = OCR2B;
  OCR2B = (OCR2A * duty) / 100;
  return prev != OCR2B;
}

bool uv_control_update_pwm_approx_linear() {
  #if !UV_LINEAR_APPROX_FREQ_ENABLED
  return;
  #endif

  t_charger_data data = {0};
  if (!charger_control_read_adc(data)) {
    uv_control_update_pwm(eeprom_control_get_freq(), uv_control_duty);
    return;
  }

  uint32_t freq = (uint32_t) UV_CONTROL_APX_A * (uint32_t) 10 + ((uint32_t) UV_CONTROL_APX_B * (uint32_t) data.bat_voltage_x100) / (uint32_t) 10;  

  if (freq < UV_CONTROL_APPROX_MINFREQ || freq > UV_CONTROL_APPROX_MAXFREQ) {
    freq = eeprom_control_get_freq();
  }

  OCR2A  = F_CPU / 2 / freq;

  uint8_t prev = OCR2B;
  OCR2B = (OCR2A * uv_control_duty) / 100;
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

void uv_control_on_main_loop() {
  if (!clock_is_elapsed(uv_control_recalc_freq, UV_CONTROL_RECALC_APPROX_FREQ_EVERY)) {
    return;
  }

  uv_control_recalc_freq = clock_millis();
  uv_control_update_pwm_approx_linear();
}

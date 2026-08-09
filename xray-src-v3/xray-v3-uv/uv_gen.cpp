#include "uv_gen.h"

#include "timer_ctrl.h"

#include "Arduino.h"
#include <avr/interrupt.h>
#include <avr/io.h>

#define PWM_FREQ_VALUE 210U
#define FB_MAX_COUNTER_BEFORE_OK 5
#define DUTY_USE_TABLE 0xFF
#define UV_CONTROL_TIMER_PERIODS 200U

#define UV_AWAIT_FB_MS 1000
#define UV_AWAIT_FB_COUNTER      (((uint32_t)UV_AWAIT_FB_MS * (F_CPU / 1000UL)) / ((uint32_t)PWM_FREQ_VALUE * UV_CONTROL_TIMER_PERIODS))


volatile uint8_t uv_gen_status = UV_GEN_STATUS_IDLE;
volatile uint8_t uv_gen_fb_counter = 0;
volatile uint16_t uv_charging_counter = 0;
uint16_t uv_supply = 0;

//  voltage                3.0 3.1 3.2 3.3 3.4 3.5 3.6 3.7 3.8 3.9 4.0 4.1 4.2 4.3 4.4 4.5
uint8_t uv_duty_table[] = { 35, 34, 33, 33, 32, 31, 31, 30, 30, 29, 29, 28, 27, 25, 24, 24 };

void isrcall__uv_gen_stop();
void uv_gen_raw_start(uint8_t tostatus, uint8_t initialduty);
void uv_detect_current_supply();

ISR(TCB0_INT_vect) {
  TCB0.INTFLAGS = TCB_CAPT_bm;

  if (uv_gen_status == UV_GEN_STATUS_MANUAL) {
    TCB0.CTRLA = 0;
    return;
  }

  if (uv_charging_counter++ > UV_AWAIT_FB_COUNTER) {
      isrcall__uv_gen_stop();
      uv_gen_status = UV_GEN_STATUS_NOFB;
  }
}

ISR(AC0_AC_vect) {
  AC0.STATUS = AC_CMP_bm;

  uint8_t v = uv_gen_fb_counter + 1;
  if (v > 100) {
    AC0.INTCTRL = 0; 
  } else if (v > FB_MAX_COUNTER_BEFORE_OK) {
    if (uv_gen_status == UV_GEN_STATUS_CHARGING) {
      isrcall__uv_gen_stop();
      uv_gen_status = UV_GEN_STATUS_IDLE;
    } else if (uv_gen_status == UV_GEN_STATUS_CHARGING_CONTINUUS || uv_gen_status == UV_GEN_STATUS_MANUAL_CONTINUUS || uv_gen_status == UV_GEN_STATUS_MANUAL) {
      // disable interrupts
      AC0.INTCTRL = 0; 
      TCB0.CTRLA = 0;
    }
  } else {
    uv_gen_fb_counter = v;
  }
}

void isrcall__uv_gen_stop() {
  // stop PWM
  TCB0.CTRLA = 0;
  TCA0.SPLIT.INTCTRL = 0;  
  TCA0.SPLIT.CTRLB = 0;
  TCA0.SPLIT.CTRLA = 0;

  digitalWrite(PIN_PA4, LOW);

  TCB0.CNT = 0;
  TCB0.INTCTRL = 0;
  TCB0.CTRLA = 0;

  // stop FB
  AC0.CTRLA = 0;
  AC0.INTCTRL = 0;
  VREF.CTRLB = 0;
}

void uv_gen_init() {
  // init PWM
  pinMode(PIN_PA4, OUTPUT);
  digitalWrite(PIN_PA4, LOW);

  TCA0.SPLIT.CTRLD = TCA_SPLIT_SPLITM_bm;
  TCA0.SPLIT.HPER = PWM_FREQ_VALUE;
  TCA0.SPLIT.HCMP1 = uv_duty_table[sizeof(uv_duty_table) - 1];
  TCA0.SPLIT.INTCTRL = 0;
  TCA0.SPLIT.CTRLB = 0;
  TCA0.SPLIT.CTRLA = 0;
  
  TCB0.CTRLA = 0;
  TCB0.CNT = 0;  
  TCB0.CCMP = PWM_FREQ_VALUE * UV_CONTROL_TIMER_PERIODS;
  TCB0.CTRLB = 0;
  TCB0.INTCTRL = 0;
  TCB0.CTRLB = TCB_CNTMODE_INT_gc;

  isrcall__uv_gen_stop();

  // init FB
  VREF.CTRLA = VREF_DAC0REFSEL_1V5_gc | VREF_ADC0REFSEL_1V1_gc;
  VREF.CTRLB = 0;
  PORTA.PIN7CTRL = PORT_ISC_INPUT_DISABLE_gc;
  AC0.CTRLA = 0;
  AC0.INTCTRL = 0;
  AC0.MUXCTRLA = AC_MUXPOS_PIN0_gc | AC_MUXNEG_VREF_gc;
}

void uv_gen_start_on(uint8_t timerlimit) {
  uv_gen_raw_start(UV_GEN_STATUS_MANUAL, timerlimit);
}

bool uv_gen_recharge(bool continuus) {
  uint8_t status = uv_gen_status;
  if (status == UV_GEN_STATUS_IDLE || status == UV_GEN_STATUS_BAD_VOLTAGE || status == UV_GEN_STATUS_NOFB) {
    uv_gen_raw_start(continuus ? UV_GEN_STATUS_CHARGING_CONTINUUS : UV_GEN_STATUS_CHARGING, DUTY_USE_TABLE);
    return true;
  } else if (status == UV_GEN_STATUS_CHARGING_CONTINUUS || status == UV_GEN_STATUS_CHARGING) {
    if (!continuus && status == UV_GEN_STATUS_CHARGING_CONTINUUS) {
      uv_gen_raw_start(UV_GEN_STATUS_CHARGING, DUTY_USE_TABLE);
    }

    return true;
  } else {
    return false;
  }
}

void uv_gen_start_run() {
  uv_gen_raw_start(UV_GEN_STATUS_MANUAL_CONTINUUS, DUTY_USE_TABLE);
}

void uv_gen_raw_start(uint8_t tostatus, uint8_t initialduty) {
  isrcall__uv_gen_stop();

  uv_gen_status = tostatus;
  uv_gen_fb_counter = 0;
  uv_charging_counter = 0;

  uv_detect_current_supply();

  if (uv_supply == 0xFFFF && initialduty == DUTY_USE_TABLE) {
    uv_gen_status = UV_GEN_STATUS_NOFB;
    return;
  }

  if (initialduty == DUTY_USE_TABLE) {
    if (uv_supply < 3000 || uv_supply > 4500) {
      uv_gen_status = UV_GEN_STATUS_BAD_VOLTAGE;
      return;
    }

    int8_t index = (uint8_t)(uv_supply / 100) - 30;

    if (index < 0 || ((uint8_t)index) >= sizeof(uv_duty_table)) {
      uv_gen_status = UV_GEN_STATUS_BAD_VOLTAGE;
      return;
    }

    initialduty = uv_duty_table[index];
  }

  // start FB
  VREF.CTRLB = VREF_DAC0REFEN_bm;
  delayMicroseconds(100);

  AC0.CTRLA = AC_ENABLE_bm | AC_HYSMODE_10mV_gc | AC_INTMODE_POSEDGE_gc;
  delayMicroseconds(100);

  AC0.STATUS = AC_CMP_bm;
  AC0.INTCTRL = AC_CMP_bm;

  TCA0.SPLIT.HCMP1 = initialduty;
  TCA0.SPLIT.INTCTRL = 0;  
  TCA0.SPLIT.CTRLB = TCA_SPLIT_HCMP1EN_bm;

  if (tostatus == UV_GEN_STATUS_CHARGING || tostatus == UV_GEN_STATUS_MANUAL_CONTINUUS) {
    TCB0.CNT = 0;
    TCB0.INTCTRL = TCB_CAPT_bm;
    TCB0.CTRLA = TCB_CLKSEL_CLKDIV1_gc | TCB_ENABLE_bm;
  } else {
    TCB0.CNT = 0;
    TCB0.INTCTRL = 0;
    TCB0.CTRLA = 0;
  }

  TCA0.SPLIT.CTRLA = TCA_SPLIT_CLKSEL_DIV1_gc | TCA_SPLIT_ENABLE_bm;
}

void uv_gen_stop() {
  isrcall__uv_gen_stop();

  uv_gen_status = UV_GEN_STATUS_STOPPED;
}

void uv_gen_start() {
  if (uv_gen_status == UV_GEN_STATUS_STOPPED) {
    uv_gen_status = UV_GEN_STATUS_IDLE;
  }
}

uint8_t isrcall__uv_get_getstatus() {
  return uv_gen_status;
}

void uv_detect_current_supply() {
  ADC0.CTRLC = ADC_REFSEL_VDDREF_gc |
                 ADC_PRESC_DIV16_gc;

  ADC0.MUXPOS = ADC_MUXPOS_INTREF_gc;

  ADC0.CTRLA = ADC_ENABLE_bm;
  delayMicroseconds(100);

  ADC0.INTFLAGS = ADC_RESRDY_bm;
  ADC0.COMMAND = ADC_STCONV_bm;

  while (!(ADC0.INTFLAGS & ADC_RESRDY_bm)) {
  }

  uint16_t adc = ADC0.RES;
  ADC0.INTFLAGS = ADC_RESRDY_bm;

  if (adc > 0) {
    uv_supply = (uint16_t)((1100UL * 1024UL + adc / 2U) / adc);
  } else {
    uv_supply = 0xFFFF;
  }

  ADC0.CTRLA = 0;
}

void uv_get_getstatus(t_uv_status_struct * status) {
  status->status = uv_gen_status;
  status->duty = TCA0.SPLIT.HCMP1;
  status->fb_counter = uv_gen_fb_counter;
  status->charge_counter = uv_charging_counter;
  status->supply = uv_supply;
}

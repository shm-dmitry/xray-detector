#include "uv_gen.h"

#include "timer_ctrl.h"

#include "Arduino.h"
#include <avr/interrupt.h>

#define INCREASE_DUTY_PERIODS 5
#define MAX_DUTY 50

volatile uint8_t uv_gen_status = UV_GEN_STATUS_IDLE;
volatile uint8_t uv_gen_cicle_counter = 0;

void isrcall__uv_gen_stop();
/*
ISR(TCA0_HUNF_vect) {
  TCA0.SPLIT.INTFLAGS = TCA_SPLIT_HUNF_bm;
  
  uv_gen_cicle_counter++;
  if (uv_gen_cicle_counter >= INCREASE_DUTY_PERIODS) {
    uv_gen_cicle_counter = 0;

    uint16_t next_duty = TCA0.SPLIT.HCMP1 + 1;

    if (next_duty > MAX_DUTY) {
        isrcall__uv_gen_stop();
        uv_gen_status = UV_GEN_STATUS_NOFB;
    } else {
        TCA0.SPLIT.HCMP1 = next_duty;
    }
  }
}

ISR(AC0_AC_vect) {
    if (AC0.STATUS & AC_STATE_bm) {
      isrcall__uv_gen_stop();
    }

    AC0.STATUS = AC_CMP_bm;
}*/


void uv_gen_init() {
  // init PWM
  pinMode(PIN_PA4, OUTPUT);
  digitalWrite(PIN_PA4, LOW);

  TCA0.SPLIT.CTRLD = TCA_SPLIT_SPLITM_bm;
  TCA0.SPLIT.HPER = 99;
  TCA0.SPLIT.HCMP1 = 5;
  TCA0.SPLIT.INTCTRL = 0;
  TCA0.SPLIT.CTRLB = 0;
  TCA0.SPLIT.CTRLA = 0;

  uv_gen_stop();

  // init FB
  VREF.CTRLA = VREF_DAC0REFSEL_1V5_gc;
  VREF.CTRLB = 0;
  PORTA.PIN7CTRL = PORT_ISC_INPUT_DISABLE_gc;
  AC0.MUXCTRLA = AC_MUXPOS_PIN0_gc | AC_MUXNEG_VREF_gc;
  AC0.INTCTRL = 0;
}

void uv_gen_start_on(uint8_t timerlimit) {
  uv_gen_stop();

  TCA0.SPLIT.HCMP1 = timerlimit;
  TCA0.SPLIT.INTCTRL = 0; // no interrupt in this mode, just generate  
  TCA0.SPLIT.CTRLB = TCA_SPLIT_HCMP1EN_bm;
  TCA0.SPLIT.CTRLA = TCA_SPLIT_CLKSEL_DIV1_gc | TCA_SPLIT_ENABLE_bm;
}

void uv_gen_start() {
  uv_gen_stop();

  // start FB
  VREF.CTRLB = VREF_DAC0REFEN_bm;
  AC0.STATUS = AC_CMP_bm;
  AC0.INTCTRL = AC_CMP_bm;
  AC0.CTRLA = AC_ENABLE_bm | AC_HYSMODE_10mV_gc;

  // start PWM
  if (uv_gen_status == UV_GEN_STATUS_IDLE || uv_gen_status == UV_GEN_STATUS_IDLE_AFTER_RUN) {
    uv_gen_status = UV_GEN_STATUS_RUNNING;
  }
  uv_gen_cicle_counter = 0;

  TCA0.SPLIT.HCMP1 = 2;
  TCA0.SPLIT.INTCTRL = TCA_SPLIT_HUNF_bm;  
  TCA0.SPLIT.CTRLB = TCA_SPLIT_HCMP1EN_bm;
  TCA0.SPLIT.CTRLA = TCA_SPLIT_CLKSEL_DIV1_gc | TCA_SPLIT_ENABLE_bm;
}

void uv_gen_stop() {
  isrcall__uv_gen_stop();
}

void isrcall__uv_gen_stop() {
  // stop PWM
  TCA0.SPLIT.INTCTRL = 0;  
  TCA0.SPLIT.CTRLB = 0;
  TCA0.SPLIT.CTRLA = 0;

  digitalWrite(PIN_PA4, LOW);

  // stop FB
  VREF.CTRLB = 0;
  AC0.INTCTRL = 0;
  AC0.CTRLA = 0;

  if (uv_gen_status == UV_GEN_STATUS_RUNNING) {
    uv_gen_status = UV_GEN_STATUS_IDLE_AFTER_RUN;
  }
}

void uv_gen_on_main_loop() {
  if (uv_gen_status == UV_GEN_STATUS_IDLE_AFTER_RUN) {
    uv_gen_status = UV_GEN_STATUS_IDLE;
    timerctrl_on_uv_runned();
  }
}

uint8_t uv_get_getstatus() {
  return uv_gen_status;
}

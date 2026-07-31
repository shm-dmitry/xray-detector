#include "uv_gen.h"

#include "timer_ctrl.h"

#include "Arduino.h"
#include <avr/interrupt.h>
#include <avr/io.h>

#define PWM_FREQ_VALUE 210U
#define PWM_FREQ_MIN_DUTY 25
#define PWM_FREQ_MAX_DUTY 30
#define INCREASE_DUTY_PERIODS 200U
#define FB_MAX_COUNTER_BEFORE_OK 5

#define UV_CHARING_MS 150
#define UV_CHARING_PERIOD      (((uint32_t)UV_CHARING_MS * (F_CPU / 1000UL)) / ((uint32_t)PWM_FREQ_VALUE * INCREASE_DUTY_PERIODS))

volatile uint8_t uv_gen_status = UV_GEN_STATUS_IDLE;
volatile uint8_t uv_gen_fb_counter = 0;
volatile uint8_t uv_fb_int_calls = 0;
volatile uint8_t uv_charging_counter = 0;

void isrcall__uv_gen_stop();
void uv_gen_raw_start(uint8_t tostatus, uint8_t initialduty);

ISR(TCB0_INT_vect) {
  TCB0.INTFLAGS = TCB_CAPT_bm;
  
  if (uv_gen_status == UV_GEN_STATUS_CHARGING) {
    if (uv_charging_counter++ > UV_CHARING_PERIOD) {
        isrcall__uv_gen_stop();
        uv_gen_status = UV_GEN_STATUS_IDLE;
    }
  } else if (uv_gen_status == UV_GEN_STATUS_SEARCH_RUN || uv_gen_status == UV_GEN_STATUS_STARTING_CONTINUUS) {
    TCB0.CTRLA = 0;  
  } else {
    uint16_t next_duty = TCA0.SPLIT.HCMP1 + 1;

    if (next_duty > PWM_FREQ_MAX_DUTY) {
        isrcall__uv_gen_stop();
        if (uv_gen_status == UV_GEN_STATUS_SEARCH) {
          uv_gen_status = UV_GEN_STATUS_SEARCH_NOFB;
        } else {
          uv_gen_status = UV_GEN_STATUS_NOFB;
        }
    } else {
        TCA0.SPLIT.HCMP1 = next_duty;
    }
  }
}

ISR(AC0_AC_vect) {
  AC0.STATUS = AC_CMP_bm;

  uv_fb_int_calls++;
  if (uv_fb_int_calls > 100) {
      AC0.INTCTRL = 0; 
  }

  uint8_t v = uv_gen_fb_counter + 1;
  if (v > FB_MAX_COUNTER_BEFORE_OK) {
    if (uv_gen_status == UV_GEN_STATUS_STARTING) {
      AC0.INTCTRL = 0; 
      uv_gen_status = UV_GEN_STATUS_CHARGING;
    } else if (uv_gen_status == UV_GEN_STATUS_STARTING_CONTINUUS) {
      // disable interrupts
      AC0.INTCTRL = 0; 
      TCB0.CTRLA = 0;
      uv_gen_status = UV_GEN_STATUS_RUNNING;
    } else if (uv_gen_status == UV_GEN_STATUS_SEARCH) {
      // disable interrupts
      AC0.INTCTRL = 0; 
      TCB0.CTRLA = 0;
      uv_gen_status = UV_GEN_STATUS_SEARCH_RUN;
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
  TCA0.SPLIT.HCMP1 = PWM_FREQ_MIN_DUTY;
  TCA0.SPLIT.INTCTRL = 0;
  TCA0.SPLIT.CTRLB = 0;
  TCA0.SPLIT.CTRLA = 0;
  
  TCB0.CTRLA = 0;
  TCB0.CNT = 0;  
  TCB0.CCMP = PWM_FREQ_VALUE * INCREASE_DUTY_PERIODS;
  TCB0.CTRLB = 0;
  TCB0.INTCTRL = 0;
  TCB0.CTRLB = TCB_CNTMODE_INT_gc;

  isrcall__uv_gen_stop();

  // init FB
  VREF.CTRLA = VREF_DAC0REFSEL_1V5_gc;
  VREF.CTRLB = 0;
  PORTA.PIN7CTRL = PORT_ISC_INPUT_DISABLE_gc;
  AC0.CTRLA = 0;
  AC0.INTCTRL = 0;
  AC0.MUXCTRLA = AC_MUXPOS_PIN0_gc | AC_MUXNEG_VREF_gc;
}

void uv_gen_start_on(uint8_t timerlimit) {
  uv_gen_raw_start(UV_GEN_STATUS_SEARCH_RUN, timerlimit);
}

bool uv_gen_recharge(bool continuus) {
  uint8_t status = uv_gen_status;
  if (status == UV_GEN_STATUS_IDLE || status == UV_GEN_STATUS_NOFB) {
    uv_gen_raw_start(continuus ? UV_GEN_STATUS_STARTING_CONTINUUS : UV_GEN_STATUS_STARTING, PWM_FREQ_MIN_DUTY);
    return true;
  } else if (status == UV_GEN_STATUS_STARTING || status == UV_GEN_STATUS_STARTING_CONTINUUS || status == UV_GEN_STATUS_RUNNING) {
    if (!continuus && (status == UV_GEN_STATUS_RUNNING || status == UV_GEN_STATUS_STARTING_CONTINUUS)) {
      uv_gen_raw_start(UV_GEN_STATUS_STARTING, PWM_FREQ_MIN_DUTY);
    }

    return true;
  } else {
    return false;
  }
}

void uv_gen_start_search() {
  uv_gen_raw_start(UV_GEN_STATUS_SEARCH, PWM_FREQ_MIN_DUTY);
}

void uv_gen_raw_start(uint8_t tostatus, uint8_t initialduty) {
  isrcall__uv_gen_stop();

  uv_gen_status = tostatus;
  uv_gen_fb_counter = 0;
  uv_fb_int_calls = 0;
  uv_charging_counter = 0;

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

  if (tostatus == UV_GEN_STATUS_STARTING || tostatus == UV_GEN_STATUS_STARTING_CONTINUUS || tostatus == UV_GEN_STATUS_SEARCH) {
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

void uv_get_getstatus(t_uv_status_struct * status) {
  status->status = uv_gen_status;
  status->duty = TCA0.SPLIT.HCMP1;
  status->fb_counter = uv_gen_fb_counter;
  status->fb_int_calls = uv_fb_int_calls;
  status->charge_counter = uv_charging_counter;
}

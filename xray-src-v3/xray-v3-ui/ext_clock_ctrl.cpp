#include "ext_clock_ctrl.h"

#include "Arduino.h"
#include "i2c_ctrl.h"

#define DEFAULT_INIT_TIMEOUT 1000
#define RV_3028_C7_I2C_ADDR 0x52

#define RV_3028_C7_ADDR__STATUS 0x0E
#define RV_3028_C7_ADDR__CONTROL1 0x0F
#define RV_3028_C7_ADDR__CONTROL2 0x10
#define RV_3028_C7_ADDR__TIMERVALUE_LOWER 0x0A
#define RV_3028_C7_ADDR__TIMERVALUE_UPPER 0x0B

volatile uint32_t clock_millis = 0;

bool ext_clock_test_eebusy_cleared();

ISR(TCB0_INT_vect) {
    clock_millis++;
    TCB0.INTFLAGS = TCB_CAPT_bm;
}

void ext_clock_ctrl_init() {
  i2c_ctrl_on_wake_up();

  while (!ext_clock_test_eebusy_cleared()) {
    delay(10);
  }

  // timer value = 1
  i2c_ctrl_write_byte(RV_3028_C7_I2C_ADDR, RV_3028_C7_ADDR__TIMERVALUE_UPPER, 0);
  i2c_ctrl_write_byte(RV_3028_C7_I2C_ADDR, RV_3028_C7_ADDR__TIMERVALUE_LOWER, 1);

  // control2 - enable TIE only
  i2c_ctrl_write_byte(RV_3028_C7_I2C_ADDR, RV_3028_C7_ADDR__CONTROL2, 0b00010000);

  // control1 - enable TRPT + TE + TD=10
  i2c_ctrl_write_byte(RV_3028_C7_I2C_ADDR, RV_3028_C7_ADDR__CONTROL1, 0b10000110);

  i2c_ctrl_on_go_sleep();

  TCB0.CTRLB = TCB_CNTMODE_INT_gc;
  TCB0.CCMP = (uint16_t)((F_CPU / 1000UL) - 1);
  TCB0.INTCTRL = TCB_CAPT_bm;
  TCB0.CTRLA = 0;
}

bool ext_clock_test_eebusy_cleared() {
  uint8_t res = 0;
  if (!i2c_ctrl_read_byte(RV_3028_C7_I2C_ADDR, RV_3028_C7_ADDR__STATUS, &res)) {
    return false;
  }

  // test EEbusy
  if ((res & 0b10000000) == 0) {
    // ok, device ready.
    // reset PORF, set UF and continue
    i2c_ctrl_write_byte(RV_3028_C7_I2C_ADDR, RV_3028_C7_ADDR__STATUS, 0b00010000);
    return true;
  }

  return false;
}

uint32_t ext_clock_millis() {
  noInterrupts();
  uint32_t v = clock_millis;
  interrupts();
  return v;
}

uint32_t isrcall__ext_clock_millis() {
  return clock_millis;
}

void ext_clock_go_sleep() {
  TCB0.CTRLA = 0;
}

void ext_clock_wake_up() {
  TCB0.CTRLA = TCB_CLKSEL_CLKDIV1_gc | TCB_ENABLE_bm;
}


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

bool ext_clock_test_eebusy_cleared();

void ext_clock_ctrl_init() {
  i2c_ctrl_on_wake_up();

  uint32_t timeout = millis() + DEFAULT_INIT_TIMEOUT;
  while (!ext_clock_test_eebusy_cleared()) {
    if (millis() > timeout) {
      return;
    }

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
}

bool ext_clock_test_eebusy_cleared() {
  uint8_t res = 0;
  if (!i2c_ctrl_read_byte(RV_3028_C7_I2C_ADDR, RV_3028_C7_ADDR__STATUS, &res)) {
    return false;
  }

  return (res & 0b10000000) > 0;
}

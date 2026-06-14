#include "ext_clock_ctrl.h"

#include "Arduino.h"
#include <Wire.h>

#define DEFAULT_I2C_TIMEOUT 50
#define RV_3028_C7_I2C_ADDR 0x52

#define RV_3028_C7_ADDR__STATUS 0x0E
#define RV_3028_C7_ADDR__CONTROL1 0x0F
#define RV_3028_C7_ADDR__CONTROL2 0x10
#define RV_3028_C7_ADDR__TIMERVALUE_LOWER 0x0A
#define RV_3028_C7_ADDR__TIMERVALUE_UPPER 0x0B

void ext_clock_configure(uint8_t address, uint8_t value);

bool ext_clock_ctrl_init() {
  Wire.begin();

  Wire.beginTransmission(RV_3028_C7_I2C_ADDR);
  Wire.write(RV_3028_C7_ADDR__CONTROL1);
  Wire.endTransmission(false);
  Wire.requestFrom(RV_3028_C7_I2C_ADDR, 1, false);
  long now = millis();
  while (!Wire.available()) {
    if (millis() - now > DEFAULT_I2C_TIMEOUT) {
      Wire.endTransmission();
      return false;
    }
  } 

  uint8_t value = Wire.read();
  Wire.endTransmission();

  if (value == 0b10000110) {
    // allready initialized
    return true;
  }

  // initialize timer

  // timer value = 1
  ext_clock_configure(RV_3028_C7_ADDR__TIMERVALUE_UPPER, 0);
  ext_clock_configure(RV_3028_C7_ADDR__TIMERVALUE_LOWER, 1);

  // control2 - enable TIE only
  ext_clock_configure(RV_3028_C7_ADDR__CONTROL2, 0b00010000);

  // control1 - enable TRPT + TE + TD=10
  ext_clock_configure(RV_3028_C7_ADDR__CONTROL1, 0b10000110);
  return true;
}

void ext_clock_configure(uint8_t address, uint8_t value) {
  uint8_t config_buffer[2] = {address, value};

  Wire.beginTransmission(RV_3028_C7_I2C_ADDR);
  Wire.write(config_buffer, 2);
  Wire.endTransmission();
}


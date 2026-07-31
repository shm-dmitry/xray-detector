#include "i2c_ctrl.h"

#include <Wire.h>

void i2c_ctrl_on_wake_up() {
  Wire.begin();
}

void i2c_ctrl_on_go_sleep() {
  Wire.end();
}

bool i2c_ctrl_read_byte(uint8_t address, uint8_t reg, uint8_t * res) {
  Wire.beginTransmission(address);
  Wire.write(reg);
  
  if (Wire.endTransmission(false) != 0) {
    Wire.endTransmission(true);
    return false;
  }
  
  uint8_t received = Wire.requestFrom(address, 1, true);
  if (received == 1 && Wire.available()) {
    *res = Wire.read();
    return true;
  } else {
    return false;
  }
}

bool i2c_ctrl_read_buffer(uint8_t address, uint8_t reg, uint8_t * buffer, uint8_t len) {
  Wire.beginTransmission(address);
  Wire.write(reg);
  if (Wire.endTransmission(false) != 0) {
    Wire.endTransmission(true);
    return false;
  }
  
  uint8_t received = Wire.requestFrom(address, len, true);
  if (received == len && Wire.available()) {
    for (uint8_t i = 0; i<len; i++) {
      if (Wire.available()) {
        buffer[i] = Wire.read();
      } else {
        return false;
      }
    }

    return true;
  } else {
    return false;
  }
}


void i2c_ctrl_write_byte(uint8_t address, uint8_t reg, uint8_t value) {
  uint8_t buffer[2] = {reg, value};

  Wire.beginTransmission(address);
  Wire.write(buffer, 2);
  Wire.endTransmission();
}


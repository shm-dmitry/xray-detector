#include "i2c_ctrl.h"

#include "Arduino.h"
#include <Wire.h>
#include "stdint.h"
#include "uv_gen.h"
#include "rad_data.h"

#define I2C_DATA_ONE_PORTION 4

#define I2C_SLAVE_ADDRESS    (0x12)

#define I2C_REGISTER__UV_STATUS          (0x01)
#define I2C_REGISTER__RAD_STATUS         (0x02)
#define I2C_REGISTER__RAD_DECIMALS       (0x03)
#define I2C_REGISTER__RAD_LASTS_VALUE    (0x10)
#define I2C_REGISTER__SBT9_DATA_BEGIN    (0x30)
#define I2C_REGISTER__SBM20_DATA_BEGIN   (0xA0)

volatile bool i2c_ctrl_pin_changed = false;
bool i2c_ctrl_enabled = false;

volatile uint8_t i2c_ctrl_request_register = 0;

void i2c_ctrl_changestate(boolean enable);
void isrcall__i2c_ctrl_onuartchanged();
void isrcall__i2c_onreceive(int bytes);
void isrcall__i2c_onrequest();
void isrcall__i2c_send_uv_status();
void isrcall__i2c_send_rad_status();
void isrcall__i2c_send_decimals();
void isrcall__i2c_send_last_value();
void isrcall__i2c_send_data(bool sbt9, uint8_t addr);

void i2c_ctrl_init() {
  pinMode(PIN_PA5, INPUT);
  i2c_ctrl_changestate(digitalRead(PIN_PA5) == HIGH);
  attachInterrupt(digitalPinToInterrupt(PIN_PA5), isrcall__i2c_ctrl_onuartchanged, CHANGE);
}

void isrcall__i2c_ctrl_onuartchanged() {
  i2c_ctrl_pin_changed = true;
}

void i2c_ctrl_changestate(bool enable) {
  if (enable && !i2c_ctrl_enabled) {
    i2c_ctrl_request_register = 0;

    Wire.begin(I2C_SLAVE_ADDRESS);
    Wire.onReceive(isrcall__i2c_onreceive);
    Wire.onRequest(isrcall__i2c_onrequest);

    i2c_ctrl_enabled = true;
  } else if (!enable && i2c_ctrl_enabled) {
    Wire.end();
    i2c_ctrl_enabled = false;
    pinMode(PIN_PB0, PIN_INPUT_DISABLE);
    pinMode(PIN_PB1, PIN_INPUT_DISABLE);
  }
}

void i2c_ctrl_on_main_loop() {
  if (i2c_ctrl_pin_changed) {
    i2c_ctrl_pin_changed = false;
    i2c_ctrl_changestate(digitalRead(PIN_PA5) == HIGH);
  }
}

bool isrcall__i2c_is_active() {
  return i2c_ctrl_enabled || i2c_ctrl_pin_changed;
}

void isrcall__i2c_onreceive(int length) {
  if (length > 0) {
    i2c_ctrl_request_register = Wire.read();

    if (length > 1 && Wire.available()) {
      if (i2c_ctrl_request_register == I2C_REGISTER__RAD_STATUS) {
        isrcall__rad_data_set_mask(Wire.read());
      }

      while(Wire.available()) {
        Wire.read();
      }
    }
  }
}

void isrcall__i2c_onrequest() {
  if (i2c_ctrl_request_register == I2C_REGISTER__UV_STATUS) {
    isrcall__i2c_send_uv_status();
  } else if (i2c_ctrl_request_register == I2C_REGISTER__RAD_STATUS) {
    isrcall__i2c_send_rad_status();
  } else if (i2c_ctrl_request_register == I2C_REGISTER__RAD_DECIMALS) {
    isrcall__i2c_send_decimals();
  } else if (i2c_ctrl_request_register == I2C_REGISTER__RAD_LASTS_VALUE) {
    isrcall__i2c_send_last_value();
  } else if (i2c_ctrl_request_register >= I2C_REGISTER__SBT9_DATA_BEGIN && i2c_ctrl_request_register < I2C_REGISTER__SBM20_DATA_BEGIN) {
    isrcall__i2c_send_data(true, i2c_ctrl_request_register - I2C_REGISTER__SBT9_DATA_BEGIN);
  } else if (i2c_ctrl_request_register >= I2C_REGISTER__SBM20_DATA_BEGIN) {
    isrcall__i2c_send_data(false, i2c_ctrl_request_register - I2C_REGISTER__SBM20_DATA_BEGIN);
  }
}

void isrcall__i2c_send_decimals() {
  Wire.write(isrcall__rad_data_decimals());
}

void isrcall__i2c_send_uv_status() {
  Wire.write(isrcall__uv_get_getstatus());
}

void isrcall__i2c_send_rad_status() {
  Wire.write(isrcall__rad_data_get_mask());
}

void isrcall__i2c_send_last_value() {
  // sbt9
  uint32_t * buffer = NULL;
  uint8_t size = isrcall__rad_data_buffer_pointer(true, 0, 1, &buffer);
  if (size > 0 && buffer != NULL) {
    const uint8_t* data_ptr = (const uint8_t*)(const void*)buffer;
    Wire.write(data_ptr, size * sizeof(uint32_t));
  }

  // sbm20
  buffer = NULL;
  size = isrcall__rad_data_buffer_pointer(false, 0, 1, &buffer);
  if (size > 0 && buffer != NULL) {
    const uint8_t* data_ptr = (const uint8_t*)(const void*)buffer;
    Wire.write(data_ptr, size * sizeof(uint32_t));
  }
}

void isrcall__i2c_send_data(bool sbt9, uint8_t addr) {
  uint32_t * buffer = NULL;
  uint8_t size = isrcall__rad_data_buffer_pointer(sbt9, addr, I2C_DATA_ONE_PORTION, &buffer);
  if (size > 0 && buffer != NULL) {
    const uint8_t* data_ptr = (const uint8_t*)(const void*)buffer;
    Wire.write(data_ptr, size * sizeof(uint32_t));
  }
}

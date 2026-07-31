#include "rad_api.h"

#include "Arduino.h"
#include "i2c_ctrl.h"

#define I2C_TIMEOUT 1000

#define I2C_SLAVE_ADDRESS    (0x12)

#define I2C_REGISTER__UV_STATUS          (0x01)
#define I2C_REGISTER__RAD_STATUS         (0x02)
#define I2C_REGISTER__RAD_DECIMALS       (0x03)
#define I2C_REGISTER__RAD_LASTS_VALUE    (0x10)
#define I2C_REGISTER__SBT9_DATA_BEGIN    (0x30)
#define I2C_REGISTER__SBM20_DATA_BEGIN   (0xA0)

void isrcall__on_high_rad();

void rad_api_init() {
  pinMode(PIN_PA4, INPUT);
  attachInterrupt(digitalPinToInterrupt(PIN_PA4), isrcall__on_high_rad, FALLING);

  pinMode(PIN_PA7, OUTPUT);
  rad_api_go_sleep();
}

void isrcall__on_high_rad() {
  // just wake up
}

bool rad_api_can_go_sleep() {
  return digitalRead(PIN_PA4) == HIGH;
}

void rad_api_wakeup() {
  digitalWrite(PIN_PA7, HIGH);

  // give time to wakeup
  delay(50);

//  Wire.begin();
}

void rad_api_go_sleep() {
  digitalWrite(PIN_PA7, LOW);
//  Wire.end();
}

uint8_t rad_api_get_uv_status() {
  uint8_t value = 0;
  if (i2c_ctrl_read_byte(I2C_SLAVE_ADDRESS, I2C_REGISTER__UV_STATUS, &value)) {
    return value;
  } else {
      return UV_GEN_STATUS_ERROR;
  }
}

uint8_t rad_api_get_rad_status() {
  uint8_t value = 0;
  if (i2c_ctrl_read_byte(I2C_SLAVE_ADDRESS, I2C_REGISTER__RAD_STATUS, &value)) {
    return value;
  } else {
    return UV_GEN_STATUS_ERROR;
  }
}

uint8_t rad_api_get_decimals() {
  uint8_t value = 0;
  if (i2c_ctrl_read_byte(I2C_SLAVE_ADDRESS, I2C_REGISTER__RAD_DECIMALS, &value)) {
    return value;
  } else {
    return UV_GEN_STATUS_ERROR;
  }
}

bool rad_api_get_current(uint32_t * sbt9, uint32_t * stm20) {
  uint8_t buffer[8];
  memset(buffer, 0xFF, 8);

  if (i2c_ctrl_read_buffer(I2C_SLAVE_ADDRESS, I2C_REGISTER__RAD_LASTS_VALUE, buffer, 8)) {
    uint32_t * temp = (uint32_t *) buffer;
    *sbt9 = temp[0];
    *stm20 = temp[1];

    return true;
  } else {
    return false;
  }
}


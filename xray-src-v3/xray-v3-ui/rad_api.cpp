#include "rad_api.h"

#include "Arduino.h"
#include "i2c_ctrl.h"

#define I2C_SLAVE_ADDRESS    (0x12)

#define I2C_REGISTER__UV_STATUS          (0x01)
#define I2C_REGISTER__RAD_STATUS         (0x02)
#define I2C_REGISTER__RAD_DECIMALS       (0x03)
#define I2C_REGISTER__RAD_BUFFER_SIZE    (0x04)
#define I2C_REGISTER__RAD_ALERT_MASK     (0x05)
#define I2C_REGISTER__RAD_ALERT_SBT9_V   (0x06)
#define I2C_REGISTER__RAD_ALERT_SBM20_V  (0x07)
#define I2C_REGISTER__RAD_LASTS_VALUE    (0x10)
#define I2C_REGISTER__SBT9_DATA_BEGIN    (0x30)
#define I2C_REGISTER__SBM20_DATA_BEGIN   (0x60)

#define RAD_API_READ_BUFFER_PART         (5)

void isrcall__on_high_rad();

void rad_api_init() {
  pinMode(PIN_PA4, INPUT);
  attachInterrupt(digitalPinToInterrupt(PIN_PA4), isrcall__on_high_rad, CHANGE);

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
}

void rad_api_go_sleep() {
  digitalWrite(PIN_PA7, LOW);
}

void rad_api_alert_setmask(uint8_t mask) {
  i2c_ctrl_write_buffer(I2C_SLAVE_ADDRESS, I2C_REGISTER__RAD_ALERT_MASK, &mask, 1);
}

void rad_api_alert_set_high_value(bool sbt9, uint32_t value) {
  const uint8_t * buffer = (const uint8_t *) (&value);
  i2c_ctrl_write_buffer(I2C_SLAVE_ADDRESS, (sbt9 ? I2C_REGISTER__RAD_ALERT_SBT9_V : I2C_REGISTER__RAD_ALERT_SBM20_V), buffer, 4);
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

uint8_t rad_api_get_buffer_size() {
  uint8_t value = 0;
  if (i2c_ctrl_read_byte(I2C_SLAVE_ADDRESS, I2C_REGISTER__RAD_BUFFER_SIZE, &value)) {
    return value;
  } else {
    return UV_GEN_STATUS_ERROR;
  }
}

bool rad_api_read_buffer(bool sbt9, uint32_t * buffer, uint8_t size) {
  uint8_t base_reg  = sbt9 ? I2C_REGISTER__SBT9_DATA_BEGIN : I2C_REGISTER__SBM20_DATA_BEGIN;

  for (uint8_t i = 0; i < size; i += RAD_API_READ_BUFFER_PART) {
    uint8_t reg = base_reg + i;
    uint8_t count = ((i + RAD_API_READ_BUFFER_PART) >= size) ? (size - i) : RAD_API_READ_BUFFER_PART; 

    count *= sizeof(uint32_t);

    uint8_t * temp = ((uint8_t *) buffer) + (i * sizeof(uint32_t));
    if (!i2c_ctrl_read_buffer(I2C_SLAVE_ADDRESS, reg, temp, count)) {
      return false;
    }
  }

  return true;
}

bool rad_api_get_current(uint32_t * sbt9, uint32_t * stm20) {
  uint8_t buffer[8];
  if (i2c_ctrl_read_buffer(I2C_SLAVE_ADDRESS, I2C_REGISTER__RAD_LASTS_VALUE, buffer, 8)) {
    uint32_t * temp = (uint32_t *) buffer;
    *sbt9 = temp[0];
    *stm20 = temp[1];

    return true;
  } else {
    return false;
  }
}


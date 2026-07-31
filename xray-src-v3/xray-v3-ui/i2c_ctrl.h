#pragma once

#include "stdint.h"

void i2c_ctrl_on_wake_up();
void i2c_ctrl_on_go_sleep();

bool i2c_ctrl_read_byte(uint8_t address, uint8_t reg, uint8_t * res);
void i2c_ctrl_write_byte(uint8_t address, uint8_t reg, uint8_t value);
bool i2c_ctrl_read_buffer(uint8_t address, uint8_t reg, uint8_t * buffer, uint8_t len);

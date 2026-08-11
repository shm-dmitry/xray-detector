#include "bat_state.h"

#include "Arduino.h"
#include "i2c_ctrl.h"

#define BATSTATE_BQ27441_I2C_ADDRESS (0x55)

#define BATSTATE_COMMAND_CONTROL      (0x00)
#define BATSTATE_COMMAND_FLAGS        (0x06)
#define BATSTATE_COMMAND_AVG_CURRENT  (0x10)
#define BATSTATE_COMMAND_VOLTAGE      (0x04)
#define BATSTATE_COMMAND_SOC          (0x1C)

#define BATSTATE_SUBCOMMAND_SOFT_RESET     (0x0042)
#define BATSTATE_SUBCOMMAND_CLR_HIBERNATE  (0x0012)
#define BATSTATE_SUBCOMMAND_SET_HIBERNATE  (0x0011)
#define BATSTATE_SUBCOMMAND_CONTROL_STATUS (0x0000)

volatile bool batstate_onbatlow_changed=false;
bool batstate_onbatlow=false;
bool batstate_initialized=false;

uint16_t bat_state_i2c_read_control(uint16_t subcommand);
void bat_state_configure();
void bat_state_i2c_exec_control(uint16_t subcommand);
uint16_t bat_state_i2c_read(uint8_t command);

void isrcall__bat_state_onbatlow() {
  batstate_onbatlow_changed = true;
}

void bat_state_init() {
  pinMode(PIN_PB5, INPUT);
  attachInterrupt(digitalPinToInterrupt(PIN_PB5), isrcall__bat_state_onbatlow, CHANGE);
}

bool bat_state_can_sleep() {
  return !batstate_onbatlow_changed && !batstate_onbatlow;
}

void bat_state_on_main_loop() {
  if (batstate_onbatlow_changed) {
    batstate_onbatlow_changed = false;
    batstate_onbatlow = digitalRead(PIN_PB5) == LOW;
  }

  if (!batstate_initialized) {
    uint16_t flags = bat_state_i2c_read(BATSTATE_COMMAND_FLAGS);
    if (flags == 0xFFFF) {
      return;
    }

    // BAT_DET set
    if (!(flags & 0b00001000)) {
      return;
    }

    // check ITPOR set
    if (flags & 0b00100000) {
      bat_state_configure();
    } else {
      batstate_initialized = true;
    }
  }
}

uint16_t bat_state_get_voltage() {
  if (!batstate_initialized) {
    return 0xFFFF;
  }

  return bat_state_i2c_read(BATSTATE_COMMAND_VOLTAGE);
}

uint16_t bat_state_get_current() {
  if (!batstate_initialized) {
    return 0xFFFF;
  }

  return bat_state_i2c_read(BATSTATE_COMMAND_AVG_CURRENT);
}

uint8_t bat_state_get_percent() {
  if (!batstate_initialized) {
    return 0xFF;
  }

  return (uint8_t)bat_state_i2c_read(BATSTATE_COMMAND_SOC);
}

void bat_state_on_wake_up() {
  bat_state_i2c_exec_control(BATSTATE_SUBCOMMAND_CLR_HIBERNATE);

  batstate_initialized = false;
}

void bat_state_on_go_sleep() {
  bat_state_i2c_exec_control(BATSTATE_SUBCOMMAND_SET_HIBERNATE);

  batstate_initialized = false;
}

void bat_state_configure() {
  // TODO: configuration

  // raise soft-reset
  bat_state_i2c_exec_control(BATSTATE_SUBCOMMAND_SOFT_RESET);

  batstate_initialized = true;
}

void bat_state_i2c_exec_control(uint16_t subcommand) {
  uint8_t buffer[2] = { (uint8_t)(subcommand & 0x00FF), (uint8_t)(subcommand >> 8) };
  i2c_ctrl_write_buffer(BATSTATE_BQ27441_I2C_ADDRESS, BATSTATE_COMMAND_CONTROL, buffer, 2);
}

uint16_t bat_state_i2c_read_control(uint16_t subcommand) {
  uint8_t buffer[2] = { (uint8_t)(subcommand & 0x00FF), (uint8_t)(subcommand >> 8) };
  i2c_ctrl_write_buffer(BATSTATE_BQ27441_I2C_ADDRESS, BATSTATE_COMMAND_CONTROL, buffer, 2);

  buffer[0] = 0;
  buffer[1] = 0;
  if (i2c_ctrl_read_buffer(BATSTATE_BQ27441_I2C_ADDRESS, BATSTATE_COMMAND_CONTROL, buffer, 2)) {
    return buffer[0] + (((uint16_t)buffer[1]) << 8);
  } else {
    return 0xFFFF;
  }
}

uint16_t bat_state_i2c_read(uint8_t command) {
  uint8_t buffer[2] = { 0, 0 };
  if (i2c_ctrl_read_buffer(BATSTATE_BQ27441_I2C_ADDRESS, command, buffer, 2)) {
    return buffer[0] + (((uint16_t)buffer[1]) << 8);
  } else {
    return 0xFFFF;
  }
}


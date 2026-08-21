#include "bat_state.h"

#include "Arduino.h"
#include "i2c_ctrl.h"

// based on https://github.com/sparkfun/SparkFun_BQ27441_Arduino_Library/tree/master

#define BATSTATE_BQ27441_I2C_ADDRESS (0x55)

#define BATSTATE_COMMAND_CONTROL             (0x00)
#define BATSTATE_COMMAND_FLAGS               (0x06)
#define BATSTATE_COMMAND_AVG_CURRENT         (0x10)
#define BATSTATE_COMMAND_VOLTAGE             (0x04)
#define BATSTATE_COMMAND_SOC                 (0x1C)
#define BATSTATE_COMMAND_BLOCK_DATA_CONTROL  (0x61)
#define BATSTATE_COMMAND_BLOCK_DATA_CLASS    (0x3E)
#define BATSTATE_COMMAND_BLOCK_DATA_BLOCK    (0x3F)
#define BATSTATE_COMMAND_BLOCK_DATA          (0x40)
#define BATSTATE_COMMAND_BLOCK_DATA_CHECKSUM (0x60)

#define BATSTATE_CLASSID_REGISTERS    (64)

#define BATSTATE_SUBCOMMAND_SOFT_RESET     (0x0042)
#define BATSTATE_SUBCOMMAND_CLR_HIBERNATE  (0x0012)
#define BATSTATE_SUBCOMMAND_SET_HIBERNATE  (0x0011)
#define BATSTATE_SUBCOMMAND_SET_CFGUPDATE  (0x0013)
#define BATSTATE_SUBCOMMAND_CONTROL_STATUS (0x0000)
#define BATSTATE_SUBCOMMAND_EXIT_CFGUPDATE (0x0043)

volatile bool batstate_onbatlow_changed=false;
bool batstate_onbatlow=false;
bool batstate_initialized=false;

uint16_t bat_state_i2c_read_control(uint16_t subcommand);
void bat_state_configure();
void bat_state_i2c_exec_control(uint16_t subcommand);
uint16_t bat_state_i2c_read(uint8_t command);
bool bat_state_write_ext_data(uint8_t classId, uint8_t offset, uint8_t * data, uint8_t len);
bool bat_state_update_opconfig(uint16_t value);

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
    uint16_t flags = bat_state_i2c_read(BATSTATE_COMMAND_FLAGS);
    if (flags != 0xFFFF) {
      batstate_onbatlow = (flags & ((1 << 2) | (1 << 1)));
      batstate_onbatlow_changed = false;
    }
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

int16_t bat_state_get_current() {
  if (!batstate_initialized) {
    return (int16_t) 0xFFFF;
  }

  return (int16_t) bat_state_i2c_read(BATSTATE_COMMAND_AVG_CURRENT);
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

  // opconfig: all defaults + set BATLOWEN 
  uint16_t opconfig = ((1 << 5) | (1 << 2) | (1 << 0));
  opconfig = opconfig << 8;
  opconfig = opconfig | ((1 << 7) | (1 << 6) | (1 << 5) | (1 << 4) | (1 << 3) | (1 << 2));
  bat_state_update_opconfig(opconfig);

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

bool bat_state_update_opconfig(uint16_t value) {
  uint8_t data[] = { (uint8_t)(value >> 8) , (uint8_t)(value & 0x00FF) };
  return bat_state_write_ext_data(BATSTATE_CLASSID_REGISTERS, 0, data, 2);
}

bool bat_state_enter_config_update() {
  bat_state_i2c_exec_control(BATSTATE_SUBCOMMAND_SET_CFGUPDATE);

  uint8_t timeout = 200;
  while (timeout-- > 0) {
    uint16_t flags = bat_state_i2c_read(BATSTATE_COMMAND_FLAGS);
    if (flags == 0xFFFF) {
      continue;
    }
    
    if (flags & 0b00010000) {
      return true;
    }

    delay(1);
  }

  return false;
}

bool bat_state_write_ext_data(uint8_t classId, uint8_t offset, uint8_t * data, uint8_t len) {
  if (!bat_state_enter_config_update()) {
    return false;
  }

  // block data
  i2c_ctrl_write_byte(BATSTATE_BQ27441_I2C_ADDRESS, BATSTATE_COMMAND_BLOCK_DATA_CONTROL, 0x00);
  i2c_ctrl_write_byte(BATSTATE_BQ27441_I2C_ADDRESS, BATSTATE_COMMAND_BLOCK_DATA_CLASS, classId);
  i2c_ctrl_write_byte(BATSTATE_BQ27441_I2C_ADDRESS, BATSTATE_COMMAND_BLOCK_DATA_BLOCK, offset / 32);

  // write
  for (uint8_t i = 0; i < len; i++) {
    i2c_ctrl_write_byte(BATSTATE_BQ27441_I2C_ADDRESS, BATSTATE_COMMAND_BLOCK_DATA + (offset % 32) + i, data[i]);
  }

  // compute checksum
  uint8_t buffer[32];
  if (!i2c_ctrl_read_buffer(BATSTATE_BQ27441_I2C_ADDRESS, BATSTATE_COMMAND_BLOCK_DATA, buffer, 32)) {
    return false;
  }

  uint8_t csum = 0;
	for (uint8_t i=0; i<32; i++) {
		csum += data[i];
	}
	csum = 255 - csum;

  i2c_ctrl_write_byte(BATSTATE_BQ27441_I2C_ADDRESS, BATSTATE_COMMAND_BLOCK_DATA_CHECKSUM, csum);

  bat_state_i2c_exec_control(BATSTATE_SUBCOMMAND_EXIT_CFGUPDATE);

  return true;
}

bool bat_state_is_low() {
  return batstate_onbatlow;
}


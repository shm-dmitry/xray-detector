#pragma once

#include "stdint.h"

#define RAD_DATA_MASK__SBT9_CURRENT_UPDATED  0b00000001
#define RAD_DATA_MASK__SBT9_HIST_UPDATED     0b00000010
#define RAD_DATA_MASK__SBM20_CURRENT_UPDATED 0b00000100
#define RAD_DATA_MASK__SBM20_HIST_UPDATED    0b00001000
#define RAD_DATA_MASK__ALERT                 0b01000000
#define RAD_DATA_MASK__NODATA                0b11111111

#define UV_GEN_STATUS_IDLE                0b00000000

// bit0 = charging
// bit1 = continuus
#define UV_GEN_STATUS_CHARGING            0b00000001
#define UV_GEN_STATUS_CHARGING_CONTINUUS  0b00000011

// bit2 = manual mode
#define UV_GEN_STATUS_MANUAL              0b00000100
#define UV_GEN_STATUS_MANUAL_CONTINUUS    0b00000111

// bit3 = manual - stopped
#define UV_GEN_STATUS_STOPPED             0b00001000

// bit4 = error bad voltage
#define UV_GEN_STATUS_BAD_VOLTAGE         0b00010000
// bit5 = no feedback
#define UV_GEN_STATUS_NOFB                0b00100000
// bit6 = no timer
#define UV_GEN_STATUS_NOTIMER             0b01000000

#define UV_GEN_STATUS_ERROR               0b11111111

#define RAD_BUFFER_TYPE__MAX              (0)
#define RAD_BUFFER_TYPE__SBT9             (1)
#define RAD_BUFFER_TYPE__SBM20            (2)
#define RAD_BUFFER_MAX_SIZE               (20)

void rad_api_init();
bool rad_api_can_go_sleep();

void rad_api_wakeup();
void rad_api_go_sleep();

uint8_t rad_api_get_uv_status();
uint8_t rad_api_get_rad_status();
uint8_t rad_api_get_decimals();
uint8_t rad_api_get_buffer_size();

bool rad_api_get_current(uint32_t * sbt9, uint32_t * stm20);
bool rad_api_read_buffer(uint8_t buffer_type, uint32_t * buffer, uint8_t size);

#pragma once

#include "stdint.h"

#define RAD_DATA_MASK__SBT9_CURRENT_UPDATED  0b00000001
#define RAD_DATA_MASK__SBT9_HIST_UPDATED     0b00000010
#define RAD_DATA_MASK__SBM20_CURRENT_UPDATED 0b00000100
#define RAD_DATA_MASK__SBM20_HIST_UPDATED    0b00001000
#define RAD_DATA_MASK__ALERT                 0b01000000
#define RAD_DATA_MASK__NODATA                0b11111111

// bit0 = search duty
// bit1 = duty found, running
#define UV_GEN_STATUS_IDLE                0b00000000
#define UV_GEN_STATUS_STARTING            0b00000001
#define UV_GEN_STATUS_STARTING_CONTINUUS  0b00000011
#define UV_GEN_STATUS_RUNNING             0b00000010
#define UV_GEN_STATUS_CHARGING            0b00000111

// bit4 = manual mode
#define UV_GEN_STATUS_SEARCH              0b00010001
#define UV_GEN_STATUS_SEARCH_RUN          0b00010000

// bit5 = stopped
#define UV_GEN_STATUS_STOPPED             0b00110000

// bit7 = error, no feedback
#define UV_GEN_STATUS_SEARCH_NOFB         0b01010000
#define UV_GEN_STATUS_NOFB                0b01000000

#define UV_GEN_STATUS_ERROR               0b11111111

void rad_api_init();
bool rad_api_can_go_sleep();

void rad_api_wakeup();
void rad_api_go_sleep();

uint8_t rad_api_get_uv_status();
uint8_t rad_api_get_rad_status();
uint8_t rad_api_get_decimals();

bool rad_api_get_current(uint32_t * sbt9, uint32_t * stm20);

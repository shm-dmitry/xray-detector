#pragma once

#include "stdint.h"

typedef struct {
  uint8_t status;
  uint8_t duty;
  uint8_t fb_counter;
  uint8_t fb_int_calls;
  uint8_t charge_counter;
} t_uv_status_struct;

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

void uv_gen_init();
bool uv_gen_recharge(bool continuus);
void uv_gen_stop();
void uv_gen_start();

void uv_get_getstatus(t_uv_status_struct * status);
uint8_t isrcall__uv_get_getstatus();

void uv_gen_start_on(uint8_t timerlimit);
void uv_gen_start_search();

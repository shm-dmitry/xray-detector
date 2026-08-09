#pragma once

#include "stdint.h"

typedef struct {
  uint8_t status;
  uint8_t duty;
  uint8_t fb_counter;
  uint16_t charge_counter;
  uint16_t supply;
} t_uv_status_struct;

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

void uv_gen_init();
bool uv_gen_recharge(bool continuus);
void uv_gen_stop();
void uv_gen_start();

void uv_get_getstatus(t_uv_status_struct * status);
uint8_t isrcall__uv_get_getstatus();

void uv_gen_start_on(uint8_t timerlimit);
void uv_gen_start_run();


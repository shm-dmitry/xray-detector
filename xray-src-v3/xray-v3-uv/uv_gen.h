#pragma once

#include "stdint.h"

typedef struct {
  uint8_t status;
  uint8_t duty;
  uint8_t fb_counter;
  uint8_t fb_int_calls;
} t_uv_status_struct;

#define UV_GEN_STATUS_IDLE 0
#define UV_GEN_STATUS_RUNNING 1
#define UV_GEN_STATUS_IDLE_AFTER_RUN 2
#define UV_GEN_STATUS_CONTINUUS_SEARCH 3
#define UV_GEN_STATUS_CONTINUUS_RUN 4
#define UV_GEN_STATUS_NOFB 0xFF

void uv_gen_init();
void uv_gen_start();
void uv_gen_stop();

void uv_gen_on_main_loop();

void uv_get_getstatus(t_uv_status_struct * status);

void uv_gen_start_on(uint8_t timerlimit);
void uv_gen_start_search();

#pragma once

#include "stdint.h"

#define UV_GEN_STATUS_IDLE 0
#define UV_GEN_STATUS_RUNNING 1
#define UV_GEN_STATUS_IDLE_AFTER_RUN 2
#define UV_GEN_STATUS_NOFB 0xFF

void uv_gen_init();
void uv_gen_start();
void uv_gen_stop();

void uv_gen_on_main_loop();

uint8_t uv_get_getstatus();

void uv_gen_start_on(uint8_t timerlimit);

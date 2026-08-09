#pragma once

#define DEBUG__PRINT_IMPL true

#if DEBUG__PRINT_IMPL
#include "stdint.h"
#endif

void uart_ctrl_init();
void uart_ctrl_on_main_loop();
bool isrcall__uart_is_active();

#if DEBUG__PRINT_IMPL
void uart_ctrl_print_impl(uint32_t sbt9, uint32_t sbm20);
#endif

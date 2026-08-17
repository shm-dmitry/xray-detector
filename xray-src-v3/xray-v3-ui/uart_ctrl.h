#pragma once

#include "stdint.h"

void uart_ctrl_init();
void uart_ctrl_on_main_loop();
bool uart_ctrl_can_go_sleep();

void uart_ctrl_print_buffer(uint32_t * buffer, uint8_t size);
void uart_ctrl_prints(const char * text);

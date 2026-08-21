#pragma once

#include "stdint.h"

void bat_state_init();
bool bat_state_can_sleep();
void bat_state_on_main_loop();
bool bat_state_is_low();

void bat_state_on_wake_up();
void bat_state_on_go_sleep();

uint16_t bat_state_get_voltage();
int16_t bat_state_get_current();
uint8_t bat_state_get_percent();


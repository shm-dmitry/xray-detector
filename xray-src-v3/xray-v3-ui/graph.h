#pragma once

#include "stdint.h"
#include "rad_api.h"

#define GRAPH_MAX_HEIGHT          0b00111111
#define GRAPH_Y_MYLT              (2)

void graph_refresh_from_history();
void graph_init(uint8_t buffer_type);
void graph_reset();
void graph_write_delta(uint8_t xbase, uint8_t ybase, uint8_t colwidth, uint16_t color1, uint16_t color2, uint16_t color3, uint16_t bgcolor);
uint32_t graph_get_maxvalue(bool prev);
uint8_t graph_get_type();


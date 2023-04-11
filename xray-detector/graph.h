#ifndef GRAPH_H_
#define GRAPH_H_

#include "stdint.h"

#define GRAPH_MAX_HEIGHT          0b00111111

void graph_refresh_from_history();
void graph_reset();
void graph_write_delta(uint8_t xbase, uint8_t ybase, uint8_t colwidth, uint16_t color1, uint16_t color2, uint16_t color3, uint16_t bgcolor);
uint32_t graph_get_maxvalue(bool prev);

#endif /* GRAPH_H_ */

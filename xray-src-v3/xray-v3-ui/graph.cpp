#include "graph.h"

#include "string.h"
#include "display.h"
#include "rad_api.h"
#include "uart_ctrl.h"
#include "alert_manager.h"

#define GRAPH_HISTORY_COLOR1_MASK 0b01000000
#define GRAPH_HISTORY_COLOR2_MASK 0b10000000
#define GRAPH_HISTORY_COLOR3_MASK 0b11000000

#define GRAPH_HISTORY_COLOR_MASKS 0b11000000

#define GRAPH_MIN_MAXVALUE  50

#define GRAPH_COLOR_FOR(value) \
  ((value & GRAPH_HISTORY_COLOR_MASKS) == GRAPH_HISTORY_COLOR1_MASK ? color1 : ((value & GRAPH_HISTORY_COLOR_MASKS) == GRAPH_HISTORY_COLOR2_MASK ? color2 : color3))
#define GRAPH_VALUE_FOR(value) (value & GRAPH_MAX_HEIGHT)

uint8_t graph_history_buffer[RAD_BUFFER_MAX_SIZE]      = { 0 };
uint8_t graph_history_prev_buffer[RAD_BUFFER_MAX_SIZE] = { 0 };
uint32_t graph_history_maxvalue;
uint32_t graph_history_prev_maxvalue;
uint8_t graph_sbt9;

void graph_draw_one_column(uint8_t xbase, uint8_t ybase, uint8_t colwidth, uint8_t index, int8_t height, uint16_t color);
void graph_correct_maxvalue(uint32_t & maxvalue);
uint8_t graph_dose2level(uint32_t dose);

void graph_init(bool sbt9) {
  graph_sbt9 = sbt9;
  graph_reset();
}

bool graph_is_sbt9() {
  return graph_sbt9;
}

void graph_refresh_from_history() {
  uint32_t buffer[RAD_BUFFER_MAX_SIZE];
  if (!rad_api_read_buffer(graph_sbt9, buffer, RAD_BUFFER_MAX_SIZE)) {
    //uart_ctrl_prints("Bad refresh ");
    return;
  }

  //uart_ctrl_print_buffer(buffer, RAD_BUFFER_MAX_SIZE);

  uint32_t maxvalue = 0;
  for (uint8_t i = 0; i<RAD_BUFFER_MAX_SIZE; i++) {
    if (buffer[i] > maxvalue) {
      maxvalue = buffer[i];
    }
  }

  if (maxvalue == 0) {
    return;
  }

  graph_correct_maxvalue(maxvalue);

  graph_history_prev_maxvalue = graph_history_maxvalue;
  graph_history_maxvalue = maxvalue;

  memcpy(graph_history_prev_buffer, graph_history_buffer, RAD_BUFFER_MAX_SIZE);

  uint32_t div = maxvalue / GRAPH_MAX_HEIGHT;

  for (uint8_t i = 0; i<RAD_BUFFER_MAX_SIZE; i++) {
    uint32_t dose = buffer[i];
    
    if (div <= 1) {
      graph_history_buffer[i] = (uint8_t)((dose * GRAPH_MAX_HEIGHT) / maxvalue);
    } else {
      graph_history_buffer[i] = (uint8_t)(dose / div);
    }

    if (graph_history_buffer[i] > GRAPH_MAX_HEIGHT) {
      graph_history_buffer[i] = GRAPH_MAX_HEIGHT;
    } else if (graph_history_buffer[i] == 0 && dose > 0) {
      graph_history_buffer[i] = 1;
    }

    uint8_t level = alert_manager_getlevel(dose);
    if (level == 0) {
      graph_history_buffer[i] |= GRAPH_HISTORY_COLOR1_MASK;
    } else if (level == 1) {
      graph_history_buffer[i] |= GRAPH_HISTORY_COLOR2_MASK;
    } else if (level == 2) {
      graph_history_buffer[i] |= GRAPH_HISTORY_COLOR3_MASK;
    }
  }
}

void graph_reset() {
  memset(graph_history_buffer,      0, RAD_BUFFER_MAX_SIZE);
  memset(graph_history_prev_buffer, 0, RAD_BUFFER_MAX_SIZE);
  graph_history_maxvalue = 0;
  graph_history_prev_maxvalue = 0;
}

void graph_write_delta(uint8_t xbase, uint8_t ybase, uint8_t colwidth, uint16_t color1, uint16_t color2, uint16_t color3, uint16_t bgcolor) {
  for (uint8_t i = 0; i<RAD_BUFFER_MAX_SIZE; i++) {
    if (graph_history_buffer[i] == graph_history_prev_buffer[i]) {
      continue;
    }

    int8_t value = GRAPH_VALUE_FOR(graph_history_buffer[i]);
    int8_t prevvalue = GRAPH_VALUE_FOR(graph_history_prev_buffer[i]);

    if (prevvalue > 0 && (graph_history_buffer[i] & GRAPH_HISTORY_COLOR_MASKS) != (graph_history_prev_buffer[i] & GRAPH_HISTORY_COLOR_MASKS)) {
      graph_draw_one_column(xbase, ybase, colwidth, i, value, GRAPH_COLOR_FOR(graph_history_buffer[i]));

      if (value < prevvalue) {
        graph_draw_one_column(xbase, ybase, colwidth, i, value - (int8_t)GRAPH_MAX_HEIGHT, bgcolor);
      }

      continue;
    }

    if (value > prevvalue) {
      graph_draw_one_column(xbase, ybase, colwidth, i, value, GRAPH_COLOR_FOR(graph_history_buffer[i]));
    }

    if (value < prevvalue) {
      graph_draw_one_column(xbase, ybase, colwidth, i, value - (int8_t)GRAPH_MAX_HEIGHT, bgcolor);
    }
  }
}

void graph_draw_one_column(uint8_t xbase, uint8_t ybase, uint8_t colwidth, uint8_t index, int8_t height, uint16_t color) {
  if (height == 0) {
    return;
  }

  if (height > 0) {
    display_fill_rect(xbase + (RAD_BUFFER_MAX_SIZE  - index - 1) * colwidth, 
                      ybase + (GRAPH_MAX_HEIGHT - height) * GRAPH_Y_MYLT, 
                      
                      colwidth, 
                      height * GRAPH_Y_MYLT,
                      
                      color);
  } else {
    display_fill_rect(xbase + (RAD_BUFFER_MAX_SIZE  - index - 1) * colwidth, 
                      ybase, 
                      
                      colwidth, 
                      -height * GRAPH_Y_MYLT,
                      
                      color);
  }
}

uint32_t graph_get_maxvalue(bool prev) {
  return prev ? graph_history_prev_maxvalue : graph_history_maxvalue;
}

void graph_correct_maxvalue(uint32_t & maxvalue) {
  if (maxvalue < GRAPH_MIN_MAXVALUE) {
    maxvalue = GRAPH_MIN_MAXVALUE;
  } else if (maxvalue > 1000000) {
    maxvalue = ((maxvalue / 100000) + 1) * 100000;
  } else if (maxvalue > 100000) {
    maxvalue = ((maxvalue / 10000) + 1) * 10000;
  } else if (maxvalue > 10000) {
    maxvalue = ((maxvalue / 1000) + 1) * 1000;
  } else if (maxvalue > 1000) {
    maxvalue = ((maxvalue / 100) + 1) * 100;
  }
}

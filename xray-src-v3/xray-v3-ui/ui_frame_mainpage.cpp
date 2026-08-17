#include "rad_api.h"
#include "ui_frame_mainpage.h"

#include "display.h"
#include "graph.h"

#define GRAPH_BASE_X (2)
#define GRAPH_BASE_Y (100)
#define GRAPH_COL_WIDTH (10)
#define MAIN_RAD_Y (53)
#define GRAPH_W (RAD_BUFFER_MAX_SIZE*GRAPH_COL_WIDTH + 2)
#define GRAPH_H (GRAPH_MAX_HEIGHT*GRAPH_Y_MYLT+2)

#define ADD_VALUE_X (GRAPH_BASE_X + GRAPH_W + 10)
#define ADD_VALUE_Y (GRAPH_BASE_Y + 50)
#define ADD_VALUE_W (319 - ADD_VALUE_X)
#define ADD_VALUE_H (GRAPH_H - 50)

uint32_t ui_frame_mainpage_add_value;
uint32_t ui_frame_mainpage_last_value;
uint8_t ui_frame_mainpage_decimals;
bool ui_frame_mainpage_lasterror;
bool ui_frame_mainpage_initialized;

void ui_frame_mainpage_printrad(uint32_t value, bool hide);
void ui_frame_mainpage_printadd(uint32_t value, bool hide);
void ui_frame_mainpage_printgraphmax(uint32_t value, bool hide);

void ui_frame_mainpage_init(bool sbt9) {
  ui_frame_mainpage_last_value = 0;
  ui_frame_mainpage_add_value = 0;
  ui_frame_mainpage_decimals = 0;
  ui_frame_mainpage_lasterror = false;
  ui_frame_mainpage_initialized = false;
  graph_init(sbt9);
}

void ui_frame_mainpage_refresh() {
  if (ui_frame_mainpage_decimals == 0 || ui_frame_mainpage_decimals == RAD_DATA_MASK__NODATA) {
    ui_frame_mainpage_decimals = rad_api_get_decimals();

    if (ui_frame_mainpage_decimals == 0 || ui_frame_mainpage_decimals == RAD_DATA_MASK__NODATA) {
      return;
    }
  }

  if (!ui_frame_mainpage_initialized) {
    display_set_textsize(3);
    display_set_textcolor(DISPLAY_WHITE);

    display_draw_rect(GRAPH_BASE_X - 1, GRAPH_BASE_Y - 1, GRAPH_W, GRAPH_H, DISPLAY_WHITE);
    display_set_cursor(GRAPH_BASE_X, MAIN_RAD_Y);
    if (graph_is_sbt9()) {
      display_prints("Betta: ");
    } else {
      display_prints("Gamma: ");
    }
 
    display_draw_rect(ADD_VALUE_X, ADD_VALUE_Y, ADD_VALUE_W, ADD_VALUE_H, DISPLAY_WHITE);
    display_set_cursor(ADD_VALUE_X + 20, ADD_VALUE_Y + 5);
    display_set_textsize(2);
    if (graph_is_sbt9()) {
      display_prints("Gamma: ");
    } else {
      display_prints("Betta: ");
    }
  }

  graph_refresh_from_history();
  graph_write_delta(GRAPH_BASE_X, GRAPH_BASE_Y, GRAPH_COL_WIDTH, DISPLAY_GREEN, DISPLAY_YELLOW, DISPLAY_RED, DISPLAY_BLACK);

  if (!ui_frame_mainpage_initialized || graph_get_maxvalue(true) != graph_get_maxvalue(false)) {
    ui_frame_mainpage_printgraphmax(graph_get_maxvalue(true), true);
    ui_frame_mainpage_printgraphmax(graph_get_maxvalue(false), false);
  }

  uint32_t sbt9 = 0;
  uint32_t sbm20 = 0;
  if (rad_api_get_current(&sbt9, &sbm20)) {
    if (graph_is_sbt9()) {
      if (!ui_frame_mainpage_initialized || sbt9 != ui_frame_mainpage_last_value) {
        ui_frame_mainpage_printrad(ui_frame_mainpage_last_value, true);
        ui_frame_mainpage_printrad(sbt9, false);

        ui_frame_mainpage_last_value = sbt9;
      }

      if (!ui_frame_mainpage_initialized || sbm20 != ui_frame_mainpage_add_value) {
        ui_frame_mainpage_printadd(ui_frame_mainpage_add_value, true);
        ui_frame_mainpage_printadd(sbm20, false);

        ui_frame_mainpage_add_value = sbm20;
      }
    } else {
      if (!ui_frame_mainpage_initialized || sbm20 != ui_frame_mainpage_last_value) {
        ui_frame_mainpage_printrad(ui_frame_mainpage_last_value, true);
        ui_frame_mainpage_printrad(sbm20, false);

        ui_frame_mainpage_last_value = sbm20;
      }

      if (!ui_frame_mainpage_initialized || sbt9 != ui_frame_mainpage_add_value) {
        ui_frame_mainpage_printadd(ui_frame_mainpage_add_value, true);
        ui_frame_mainpage_printadd(sbt9, false);

        ui_frame_mainpage_add_value = sbt9;
      }
    }
  }

  if (!ui_frame_mainpage_initialized) {
    ui_frame_mainpage_initialized = true;
  }
}

void ui_frame_mainpage_printrad(uint32_t value, bool hide) {
  if (ui_frame_mainpage_decimals == 0 || ui_frame_mainpage_decimals == RAD_DATA_MASK__NODATA) {
    return;
  }

  display_set_cursor(180, MAIN_RAD_Y);
  display_set_textsize(3);
  display_set_textcolor(DISPLAY_WHITE);

  if (hide) {
    display_set_textcolor(DISPLAY_BLACK);
  } else {
    display_set_textcolor(DISPLAY_WHITE);
  }

  display_printn(value / ui_frame_mainpage_decimals);
  display_prints(".");
  display_printn((uint8_t)(value % ui_frame_mainpage_decimals));
  display_prints(" uR");
}

void ui_frame_mainpage_printadd(uint32_t value, bool hide) {
  if (ui_frame_mainpage_decimals == 0 || ui_frame_mainpage_decimals == RAD_DATA_MASK__NODATA) {
    return;
  }

  display_set_cursor(ADD_VALUE_X + 25, ADD_VALUE_Y + 30);
  display_set_textsize(2);
  display_set_textcolor(DISPLAY_WHITE);

  if (hide) {
    display_set_textcolor(DISPLAY_BLACK);
  } else {
    display_set_textcolor(DISPLAY_WHITE);
  }

  display_printn(value / ui_frame_mainpage_decimals);
  display_prints(".");
  display_printn((uint8_t)(value % ui_frame_mainpage_decimals));

  display_set_cursor(ADD_VALUE_X + 30, ADD_VALUE_Y + 55);
  display_prints("uR");
}

void ui_frame_mainpage_printgraphmax(uint32_t value, bool hide) {
  display_set_cursor(GRAPH_BASE_X + GRAPH_W + 10, GRAPH_BASE_Y);
  display_set_textsize(2);
  display_set_textcolor(DISPLAY_WHITE);
  
  if (hide) {
    display_set_textcolor(DISPLAY_BLACK);
  } else {
    display_set_textcolor(DISPLAY_WHITE);
  }

  display_printn(value / ui_frame_mainpage_decimals);
  display_prints(".");
  display_printn((uint8_t)(value % ui_frame_mainpage_decimals));
  display_prints(" uR");
}

void ui_frame_mainpage_init_gamma() {
  ui_frame_mainpage_init(false);
}

void ui_frame_mainpage_init_betta() {
  ui_frame_mainpage_init(true);
}




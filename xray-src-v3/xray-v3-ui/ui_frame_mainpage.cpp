#include "rad_api.h"
#include "ui_frame_mainpage.h"

#include "display.h"
#include "graph.h"

uint32_t ui_frame_mainpage_last_value;
uint8_t ui_frame_mainpage_decimals;
bool ui_frame_mainpage_lasterror;
bool ui_frame_mainpage_initialized;

void ui_frame_mainpage_printrad(uint32_t value, bool hide);
void ui_frame_mainpage_printerr(bool sbt9, bool hide);

void ui_frame_mainpage_init(uint8_t graphtype) {
  ui_frame_mainpage_last_value = 0;
  ui_frame_mainpage_decimals = 0;
  ui_frame_mainpage_lasterror = false;
  ui_frame_mainpage_initialized = false;
  graph_init(graphtype);
  graph_reset();
}

void ui_frame_mainpage_refresh() {
  if (ui_frame_mainpage_decimals == 0 || ui_frame_mainpage_decimals == RAD_DATA_MASK__NODATA) {
    ui_frame_mainpage_decimals = rad_api_get_decimals();

    if (ui_frame_mainpage_decimals == 0 || ui_frame_mainpage_decimals == RAD_DATA_MASK__NODATA) {
      return;
    }
  }

  display_set_textsize(3);
  display_set_textcolor(DISPLAY_WHITE);

  if (!ui_frame_mainpage_initialized) {
    display_draw_rect(10-1, 50-1, RAD_BUFFER_MAX_SIZE*10+2, GRAPH_MAX_HEIGHT*GRAPH_Y_MYLT+2, DISPLAY_WHITE);
    display_set_cursor(10, 200);
    if (graph_get_type() == RAD_BUFFER_TYPE__MAX) {
      display_prints("Current: ");
    } else if (graph_get_type() == RAD_BUFFER_TYPE__SBT9) {
      display_prints("SBT9: ");
    } else {
      display_prints("SBM20: ");
    }
  }

  graph_refresh_from_history();
  graph_write_delta(10, 50, 10, DISPLAY_GREEN, DISPLAY_YELLOW, DISPLAY_RED, DISPLAY_BLACK);

  uint32_t sbt9 = 0;
  uint32_t sbm20 = 0;
  if (rad_api_get_current(&sbt9, &sbm20)) {
    if (graph_get_type() == RAD_BUFFER_TYPE__MAX) {
      if (sbt9 < sbm20) {
        sbt9 = sbm20;
      }

      if (!ui_frame_mainpage_initialized || sbt9 != ui_frame_mainpage_last_value) {
        ui_frame_mainpage_printrad(ui_frame_mainpage_last_value, true);
        ui_frame_mainpage_printrad(sbt9, false);

        ui_frame_mainpage_last_value = sbt9;
      }
    } else if (graph_get_type() == RAD_BUFFER_TYPE__SBT9) {
      if (!ui_frame_mainpage_initialized || sbt9 != ui_frame_mainpage_last_value) {
        ui_frame_mainpage_printrad(ui_frame_mainpage_last_value, true);
        ui_frame_mainpage_printrad(sbt9, false);

        ui_frame_mainpage_last_value = sbt9;
      }
    } else {
      if (!ui_frame_mainpage_initialized || sbm20 != ui_frame_mainpage_last_value) {
        ui_frame_mainpage_printrad(ui_frame_mainpage_last_value, true);
        ui_frame_mainpage_printrad(sbm20, false);

        ui_frame_mainpage_last_value = sbm20;
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

  display_set_cursor(180, 200);

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

void ui_frame_mainpage_init_maxrad() {
  ui_frame_mainpage_init(RAD_BUFFER_TYPE__MAX);
}

void ui_frame_mainpage_init_sbt9() {
  ui_frame_mainpage_init(RAD_BUFFER_TYPE__SBT9);
}

void ui_frame_mainpage_init_sbtm20() {
  ui_frame_mainpage_init(RAD_BUFFER_TYPE__SBM20);
}




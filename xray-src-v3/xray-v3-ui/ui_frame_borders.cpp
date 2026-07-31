#include "ui_frame_borders.h"

#include "display.h"

#include "rad_api.h"

#define BORDER_X (0)
#define BORDER_Y (0)
#define BORDER_W (DISPLAY_WIDTH)
#define BORDER_H (30)

bool ui_borders_initialized = false;
uint8_t ui_borders_uvstatus = UV_GEN_STATUS_ERROR;

void ui_botders_display_uv(uint8_t value, bool hide);

void ui_borders_init() {
  ui_borders_initialized = false;
}

void ui_borders_refresh() {
  if (!ui_borders_initialized) {
    display_draw_line(BORDER_X, BORDER_H, BORDER_W, BORDER_H, DISPLAY_GREEN);
    display_fill_rect(BORDER_X, BORDER_Y, BORDER_W, BORDER_H - 1, DISPLAY_BLACK);
  }

  uint8_t status = rad_api_get_uv_status();
  if (!ui_borders_initialized || status != ui_borders_uvstatus) {
    ui_botders_display_uv(ui_borders_uvstatus, true);
    ui_botders_display_uv(status, false);

    ui_borders_uvstatus = status;
  }

  ui_borders_initialized = true;
}

void ui_botders_display_uv(uint8_t value, bool hide) {
  if (hide) {
    display_set_textcolor(DISPLAY_BLACK);
  } else {
    if (value == UV_GEN_STATUS_IDLE) {
      display_set_textcolor(DISPLAY_GRAY);
    } else if (value & UV_GEN_STATUS_NOFB) {
      display_set_textcolor(DISPLAY_RED);
    } else if (value == UV_GEN_STATUS_STOPPED) {
      display_set_textcolor(DISPLAY_RED);
    } else if (value & UV_GEN_STATUS_SEARCH_RUN) {
      display_set_textcolor(DISPLAY_YELLOW);
    } else if (value > 0 && value <= UV_GEN_STATUS_CHARGING) {
      display_set_textcolor(DISPLAY_GREEN);
    } else {
      display_set_textcolor(DISPLAY_WHITE);
    }
  }

  display_set_cursor(10, 2);
  display_set_textsize(2);

  if (value == UV_GEN_STATUS_IDLE) {
    display_prints("I");
  } else if (value & UV_GEN_STATUS_NOFB) {
    display_prints("NOFB");
  } else if (value == UV_GEN_STATUS_STOPPED) {
    display_prints("STOP");
  } else if (value & UV_GEN_STATUS_SEARCH_RUN) {
    display_prints("MANUAL");
  } else if (value > 0 && value <= UV_GEN_STATUS_CHARGING) {
    display_prints("CH");
  } else {
    display_print8(value);
    display_prints(" ??");
  }
}


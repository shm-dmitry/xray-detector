#include "ui_frame_borders.h"

#include "display.h"

#include "rad_api.h"
#include "bat_state.h"

#define BORDER_X (0)
#define BORDER_Y (0)
#define BORDER_W (DISPLAY_WIDTH)
#define BORDER_H (30)

bool ui_borders_initialized = false;
uint8_t ui_borders_uvstatus = UV_GEN_STATUS_ERROR;
uint8_t ui_borders_radstatus = UV_GEN_STATUS_ERROR;
uint8_t ui_borders_batpc = 0;

void ui_botders_display_uv(uint8_t value, bool hide);
void ui_botders_display_rad(uint8_t value, bool hide);
void ui_botders_display_bat(uint8_t value, bool hide);

void ui_borders_init() {
  ui_borders_initialized = false;
}

void ui_borders_cleanup_working_area() {
  display_fill_rect(BORDER_X, BORDER_H + 1, BORDER_W, DISPLAY_HEIGHT - 1, DISPLAY_BLACK);
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

  status = rad_api_get_rad_status();
  if (!ui_borders_initialized || status != ui_borders_radstatus) {
    ui_botders_display_rad(ui_borders_radstatus, true);
    ui_botders_display_rad(status, false);

    ui_borders_radstatus = status;
  }

  status = bat_state_get_percent();
  if (!ui_borders_initialized || status != ui_borders_batpc) {
    ui_botders_display_bat(ui_borders_batpc, true);
    ui_botders_display_bat(status, false);

    ui_borders_batpc = status;
  }

  if (!ui_borders_initialized) {
    ui_borders_initialized = true;
  }
}

void ui_botders_display_rad(uint8_t value, bool hide) {
  if (hide) {
    display_set_textcolor(DISPLAY_BLACK);
  } else {
    if (value == RAD_DATA_MASK__NODATA) {
      display_set_textcolor(DISPLAY_RED);
    }  else if (value & RAD_DATA_MASK__ALERT) {
      display_set_textcolor(DISPLAY_RED);
    } else {
      display_set_textcolor(DISPLAY_WHITE);
    }
  }

  display_set_cursor(120, 5);
  display_set_textsize(2);

  if (value == RAD_DATA_MASK__NODATA) {
    display_prints("ERR");
  }  else if (value & RAD_DATA_MASK__ALERT) {
    display_prints("ALERT");
  } else {
    display_prints("OK");
  }
}

void ui_botders_display_uv(uint8_t value, bool hide) {
  if (hide) {
    display_set_textcolor(DISPLAY_BLACK);
  } else {
    if (value == UV_GEN_STATUS_IDLE) {
      display_set_textcolor(DISPLAY_GRAY);
    } else if (value & UV_GEN_STATUS_NOFB || value & UV_GEN_STATUS_BAD_VOLTAGE || value & UV_GEN_STATUS_NOTIMER) {
      display_set_textcolor(DISPLAY_RED);
    } else if (value == UV_GEN_STATUS_STOPPED) {
      display_set_textcolor(DISPLAY_RED);
    } else if (value & UV_GEN_STATUS_MANUAL) {
      display_set_textcolor(DISPLAY_YELLOW);
    } else if (value & UV_GEN_STATUS_CHARGING) {
      display_set_textcolor(DISPLAY_GREEN);
    } else {
      display_set_textcolor(DISPLAY_WHITE);
    }
  }

  display_set_cursor(10, 5);
  display_set_textsize(2);

  if (value == UV_GEN_STATUS_IDLE) {
    display_prints("IDLE");
  } else if (value & UV_GEN_STATUS_NOTIMER) {
    display_prints("NOTIMER");
  } else if (value & UV_GEN_STATUS_NOFB) {
    display_prints("NOFB");
  } else if (value & UV_GEN_STATUS_BAD_VOLTAGE) {
    display_prints("BAD V");
  } else if (value == UV_GEN_STATUS_STOPPED) {
    display_prints("STOP");
  } else if (value & UV_GEN_STATUS_MANUAL) {
    display_prints("MANUAL");
  } else if (value & UV_GEN_STATUS_CHARGING) {
    display_prints("CH");
  } else {
    display_print8(value);
    display_prints(" ??");
  }
}

void ui_botders_display_bat(uint8_t value, bool hide) {
  uint16_t color;
  if (hide) {
    color = DISPLAY_BLACK;
  } else if (value < 20) {
    color = DISPLAY_RED;
  } else if (value < 50) {
    color = DISPLAY_YELLOW;
  } else {
    color = DISPLAY_GREEN;
  }

  if (value > 100) {
    value = 100;
  }

  uint16_t filled = value / 4;
  uint16_t max = 100 / 4;
  display_fill_rect(220, 5, filled, 15, color);
  if (filled < max) {
    display_draw_rect(220+filled, 5, max-filled, 15, color);
  }
  display_fill_rect(220+max, 8, 5, 9, color);

  display_set_cursor(220+max+5+20, 5);
  display_set_textsize(2);
  display_set_textcolor(color);
  display_print8(value);
  display_prints("%");
}



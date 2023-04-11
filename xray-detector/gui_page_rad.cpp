#include "gui_page_rad.h"
#include "rad_control.h"
#include "display.h"
#include "alarm_manager.h"
#include "clock.h"
#include "graph.h"
#include "rad_history.h"

#include "Arduino.h"

uint32_t rad_page_dose = 0xFFFFFFFF;

void gui_rad_page_draw_dose(uint32_t dose);
void gui_rad_page_draw_dose_text(uint32_t dose, bool mode);

bool gui_rad_page_refresh(uint8_t data) {
  if (!display_is_on()) {
    return;
  }

  if (data) {
    rad_page_dose = 0xFFFFFFFF;
    display_draw_rect(2, 50, 2+100, GRAPH_MAX_HEIGHT + 2, DISPLAY_WHITE);
    graph_reset();
  }

  uint32_t dose = rad_control_dose();
  if (rad_page_dose != dose) {
    display_set_cursor(10, 20);
    gui_rad_page_draw_dose(dose);
    rad_page_dose = dose;
  }

  if (rad_history_was_minute_buffer_changed()) {
    graph_refresh_from_history();
    graph_write_delta(3, 51, 100 / RAD_HISTORY_STORE_MINUTE_POINTS, DISPLAY_GREEN, DISPLAY_YELLOW, DISPLAY_RED, DISPLAY_BLACK);
    uint32_t maxvalue = graph_get_maxvalue(false);
    if (maxvalue > 0 && (maxvalue != graph_get_maxvalue(true) || data)) {
      display_set_cursor(3 + 100 + 2, 52);
      display_set_textcolor(DISPLAY_BLACK);
      gui_rad_page_draw_dose_text(graph_get_maxvalue(true), false);

      uint8_t level = alarm_manager_dose2level(maxvalue);
      display_set_cursor(3 + 100 + 2, 52);
      display_set_textcolor(level == 0 ? DISPLAY_GREEN : (level == 1 ? DISPLAY_YELLOW : DISPLAY_RED));
      gui_rad_page_draw_dose_text(maxvalue, false);

      display_draw_line(3+100, 50, 3+100+2 + 2, 50, DISPLAY_WHITE);
      display_draw_line(3+100, 50 + GRAPH_MAX_HEIGHT/2, 3+100+2 + 2, 50 + GRAPH_MAX_HEIGHT/2, DISPLAY_WHITE);
    }
  }

  return true;
}

void gui_rad_page_draw_dose_text(uint32_t dose, bool mode) {
  uint8_t x = display_get_cursor_x();
  if (dose < 1000) {
    if (mode) {
      display_set_textsize(3);
    } else {
      display_set_textsize(1);
    }
    display_print16(dose);
    if (mode) {
      display_set_textsize(2);
      display_set_cursor(display_get_cursor_x() + 16, display_get_cursor_y() + 8);
    } else {
      display_set_textsize(1);
      display_set_cursor(x, display_get_cursor_y() + 8);
    }
    display_prints("uR/h");
  } else if (dose < 1000000) {
    if (mode) {
      display_set_textsize(3);
    } else {
      display_set_textsize(1);
    }
    display_print16(dose / 1000);
    display_prints(".");
    display_print16((dose % 1000) / 100);
    if (mode) {
      display_set_cursor(display_get_cursor_x() + 16, display_get_cursor_y() + 8);
      display_set_textsize(2);
    } else {
      display_set_textsize(1);
      display_set_cursor(x, display_get_cursor_y() + 8);
    }
    display_prints("mR/h");
  } else {
    if (mode) {
      display_set_textsize(3);
    } else {
      display_set_textsize(1);
    }
    display_print16(dose / 1000000);
    display_prints(".");
    display_print16((dose % 1000000) / 100000);
    if (mode) {
      display_set_cursor(display_get_cursor_x() + 16, display_get_cursor_y() + 8);
      display_set_textsize(2);
    } else {
      display_set_textsize(1);
      display_set_cursor(x, display_get_cursor_y() + 8);
    }
    display_prints("R/h");
  }
}

void gui_rad_page_draw_dose(uint32_t dose) {
  uint8_t x = display_get_cursor_x();
  uint8_t y = display_get_cursor_y();

  display_set_textcolor(DISPLAY_BLACK);
  gui_rad_page_draw_dose_text(rad_page_dose, true);

  uint8_t level = alarm_manager_dose2level(dose);
  display_set_cursor(x, y);
  display_set_textcolor(level == 0 ? DISPLAY_GREEN : (level == 1 ? DISPLAY_YELLOW : DISPLAY_RED));
  gui_rad_page_draw_dose_text(dose, true);
}

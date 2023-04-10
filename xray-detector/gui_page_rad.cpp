#include "gui_page_rad.h"
#include "rad_control.h"
#include "display.h"
#include "alarm_manager.h"
#include "clock.h"

uint32_t rad_page_dose = 0xFFFFFFFF;

void gui_rad_page_draw_dose(uint32_t dose);

bool gui_rad_page_refresh(uint8_t data) {
  if (!display_is_on()) {
    return;
  }

  if (data) {
    rad_page_dose = 0xFFFFFFFF;
  }

  uint32_t dose = rad_control_dose();
  if (rad_page_dose != dose) {
    gui_rad_page_draw_dose(dose);
    rad_page_dose = dose;
  }

  return true;
}

void gui_rad_page_draw_dose_text(uint32_t dose) {
  if (dose < 1000) {
    display_set_textsize(3);
    display_print16(dose);
    display_set_textsize(2);
    display_set_cursor(display_get_cursor_x(), display_get_cursor_y() + 8);
    display_prints(" uR/h");
  } else if (dose < 1000000) {
    display_set_textsize(3);
    display_print16(dose / 1000);
    display_prints(".");
    display_print16((dose % 1000) / 100);
    display_set_cursor(display_get_cursor_x(), display_get_cursor_y() + 8);
    display_set_textsize(2);
    display_prints(" mR/h");
  } else {
    display_set_textsize(3);
    display_print16(dose / 1000000);
    display_prints(".");
    display_print16((dose % 1000000) / 100000);
    display_set_cursor(display_get_cursor_x(), display_get_cursor_y() + 8);
    display_set_textsize(2);
    display_prints(" R/h");
  }
}

void gui_rad_page_draw_dose(uint32_t dose) {
  display_set_cursor(10, 20);
  display_set_textcolor(DISPLAY_BLACK);
  gui_rad_page_draw_dose_text(rad_page_dose);

  uint8_t level = alarm_manager_dose2level(dose);
  display_set_cursor(10, 20);
  display_set_textcolor(level == 0 ? DISPLAY_GREEN : (level == 1 ? DISPLAY_YELLOW : DISPLAY_RED));
  gui_rad_page_draw_dose_text(dose);
}

#include "gui_page_rad.h"
#include "rad_control.h"
#include "display.h"
#include "alarm_manager.h"
#include "clock.h"
#include "graph.h"
#include "rad_history.h"
#include "rad_accum_history.h"
#include "datepacker.h"

#include "Arduino.h"

#define RAD_PAGE_MODE_STARTUP             0
#define RAD_PAGE_MODE_HISTORY             1
#define RAD_PAGE_MODE_STARTUP_FULLREFRESH 2
#define RAD_PAGE_MODE_HISTORY_FULLREFRESH 3

uint32_t rad_page_dose = 0;
uint32_t rad_page_accum = 0;
uint8_t rad_page_mode = RAD_PAGE_MODE_STARTUP;

void gui_rad_page_draw_dose(uint32_t dose);
void gui_rad_page_draw_dose_text(uint32_t dose, bool mode);
void gui_rad_page_draw_accum(uint32_t dose_usv);
void gui_rad_page_draw_accum_text(uint32_t dose_usv);
void gui_rad_page_draw_accum_list_item(uint32_t dose_usv);

bool gui_rad_page_refresh(uint8_t data) {
  if (!display_is_on()) {
    return;
  }

  if (data == 0xFF) {
    rad_page_mode = RAD_PAGE_MODE_STARTUP;
  }

  if (rad_page_mode == RAD_PAGE_MODE_STARTUP_FULLREFRESH || rad_page_mode == RAD_PAGE_MODE_HISTORY_FULLREFRESH) {
    data = 0x01;
    rad_page_mode = ((rad_page_mode == RAD_PAGE_MODE_STARTUP_FULLREFRESH) ? RAD_PAGE_MODE_STARTUP : RAD_PAGE_MODE_HISTORY);
    display_fill_rect(0, 8, 160, 128-8*2, DISPLAY_BLACK);
  }

  if (data) {
    rad_page_dose = 0xFFFFFFFF;
    rad_page_accum = 0xFFFFFFFF;
    graph_reset();

    if (rad_page_mode == RAD_PAGE_MODE_STARTUP) {
      display_draw_rect(2, 50, 2+100, GRAPH_MAX_HEIGHT + 2, DISPLAY_WHITE);
    }
  }

  if (rad_page_mode == RAD_PAGE_MODE_STARTUP) {
    uint32_t dose = rad_control_dose();
    if (rad_page_dose != dose) {
      display_set_cursor(8, 20);
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
    
    uint32_t accum = rad_accum_get_daily_dose_usv();
    if (accum == 0xFFFFFFFF) {
      accum = 0xFFFFFFFE;
    }

    if (rad_page_accum == 0xFFFFFFFF || accum != rad_page_accum) {
      gui_rad_page_draw_accum(accum);
      rad_page_accum = accum;
    }
  } else if (data) {
    uint32_t total = 0;
    display_set_cursor(0, 10);
    display_set_textcolor(DISPLAY_WHITE);
    display_set_textsize(1);
    for (uint8_t i = 0; i<rad_accum_history_points_count(); i++) {
      if (i+1 <= 9) {
        display_prints(" ");
      }

      display_print8(i + 1);
      display_prints(". ");

      uint32_t datepacked = 0;
      uint32_t value = 0;
      if (rad_accum_history_read(i, datepacked, value)) {
        display_print16(DATE_PACKER__YY(datepacked));
        display_prints("-");        
        uint8_t month = DATE_PACKER__MM(datepacked); 
        if (month <= 9) {
          display_prints("0");
        }
        display_print8(month);
        display_prints(": ");
        gui_rad_page_draw_accum_list_item(value);

        if (value != 0 && total != 0xFFFFFFFF && total + value > total) {
          total += value;
        } else {
          total = 0xFFFFFFFF;
        }
      } else {
        display_println("<empty cell>");
      }
    }
    display_prints("      Total: ");
    gui_rad_page_draw_accum_list_item(total);
  }

  return true;
}

void gui_rad_page_draw_accum_list_item(uint32_t value) {
  if (value == 0xFFFFFFFF) {
    display_prints("TOO HIGH!");
  } else if (value > 1000000) {
    display_print16(value / 1000000);
    display_prints(".");
    display_print16((value % 1000000) / 100000);
    display_println(" Sv");
  } else if (value > 1000) {
    display_print16(value / 1000);
    display_prints(".");
    display_print16((value % 1000) / 100);
    display_println(" mSv");
  } else {
    display_print16(value);
    display_println(" uSv");
  }
}

void gui_rad_page_draw_accum(uint32_t dose_usv) {
  display_set_cursor(3 + 100 + 7, 85);
  display_set_textcolor(DISPLAY_BLACK);
  gui_rad_page_draw_accum_text(rad_page_accum);

  display_set_cursor(3 + 100 + 7, 85);
  display_set_textcolor(DISPLAY_WHITE);
  gui_rad_page_draw_accum_text(dose_usv);
}

void gui_rad_page_draw_accum_text(uint32_t dose_usv) {
  uint8_t x = display_get_cursor_x();

  display_set_textsize(1);

  display_prints("Accum");
  display_set_cursor(x, display_get_cursor_y() + 10);

  if (dose_usv >= 0xFFFFFFFE) {
    display_prints("TOO HIGH");
  } else if (dose_usv > 1000000) {
    display_print16(dose_usv / 1000000);
    display_prints(".");
    display_print16((dose_usv % 1000000) / 100000);
    display_set_cursor(x, display_get_cursor_y() + 10);
    display_prints("Sv");
  } else if (dose_usv > 1000) {
    display_print16(dose_usv / 1000);
    display_prints(".");
    display_print16((dose_usv % 1000) / 100);
    display_set_cursor(x, display_get_cursor_y() + 10);
    display_prints("mSv");
  } else {
    display_print16(dose_usv);
    display_set_cursor(x, display_get_cursor_y() + 10);
    display_prints("uSv");
  }
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

bool gui_rad_page_onclick(uint8_t data) {
  if (rad_page_mode == RAD_PAGE_MODE_STARTUP || rad_page_mode == RAD_PAGE_MODE_STARTUP_FULLREFRESH) {
    rad_page_mode = RAD_PAGE_MODE_HISTORY_FULLREFRESH;
  } else {
    rad_page_mode = RAD_PAGE_MODE_STARTUP_FULLREFRESH;
  }
}


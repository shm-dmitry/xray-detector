#include "gui_page_bat.h"

#include "charger_control.h"
#include "display.h"

#define GUI_PAGE_BAT_HIGH   70
#define GUI_PAGE_BAT_MEDIUM 30


t_charger_data gui_bat_prev_data = { 0 };

bool gui_bat_page_refresh(uint8_t data) {
  if (!display_is_on()) {
    return false;
  }

  t_charger_data charger_data = { 0 };
  if (!charger_control_get_data(charger_data)) {
    if (!data) {
      return true;
    }
  }

  display_set_textsize(1);

  if (data) {
    display_set_textcolor(DISPLAY_WHITE);

    display_set_cursor(20, 30);
    display_prints("Vbat     = ");

    display_set_cursor(20, 30 + 8);
    display_prints("Icharger = ");

    display_set_cursor(20, 30 + 8*2);
    display_prints("Source   = ");
  }

  if (data || charger_data.bat_voltage_x100 != gui_bat_prev_data.bat_voltage_x100) {
    display_set_textcolor(DISPLAY_BLACK);
    display_set_cursor(100, 30);
    display_print8(gui_bat_prev_data.bat_voltage_x100 / 100);
    if (gui_bat_prev_data.bat_voltage_x100 % 100 < 10) {
      display_prints(".0");
    } else {
      display_prints(".");
    }
    display_print8(gui_bat_prev_data.bat_voltage_x100 % 100);
    display_prints("V");

    uint8_t pc = charger_control_get_voltage_pc();

    display_set_textcolor(pc > GUI_PAGE_BAT_HIGH ? DISPLAY_GREEN : (pc > GUI_PAGE_BAT_MEDIUM ? DISPLAY_WHITE : DISPLAY_RED));
    display_set_cursor(100, 30);
    display_print8(charger_data.bat_voltage_x100 / 100);
    if (charger_data.bat_voltage_x100 % 100 < 10) {
      display_prints(".0");
    } else {
      display_prints(".");
    }
    display_print8(charger_data.bat_voltage_x100 % 100);
    display_prints("V");

    gui_bat_prev_data.bat_voltage_x100 = charger_data.bat_voltage_x100;
  }

  if (data || charger_data.bat_current_x10 != gui_bat_prev_data.bat_current_x10) {
    display_set_textcolor(DISPLAY_BLACK);
    display_set_cursor(100, 30+8);
    display_print8(gui_bat_prev_data.bat_current_x10 / 10);
    display_prints(".");
    display_print8(gui_bat_prev_data.bat_current_x10 % 10);
    display_prints("A");

    display_set_textcolor(DISPLAY_WHITE);
    display_set_cursor(100, 30+8);
    display_print8(charger_data.bat_current_x10 / 10);
    display_prints(".");
    display_print8(charger_data.bat_current_x10 % 10);
    display_prints("A");

    gui_bat_prev_data.bat_current_x10 = charger_data.bat_current_x10;
  }

  if (data || charger_data.i2c_in_use != gui_bat_prev_data.i2c_in_use) {
    display_set_textcolor(DISPLAY_BLACK);
    display_set_cursor(100, 30+8*2);
    if (gui_bat_prev_data.i2c_in_use) {
      display_prints("Charger");
    } else {
      display_prints("ADC");
    }

    display_set_textcolor(charger_data.i2c_in_use ? DISPLAY_GREEN : DISPLAY_YELLOW);
    display_set_cursor(100, 30+8*2);
    if (charger_data.i2c_in_use) {
      display_prints("Charger");
    } else {
      display_prints("ADC");
    }

    gui_bat_prev_data.i2c_in_use = charger_data.i2c_in_use;
  }

  return true;
}

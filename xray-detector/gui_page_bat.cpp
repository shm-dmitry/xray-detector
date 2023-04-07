#include "gui_page_bat.h"

#include "charger_control.h"
#include "display.h"

#define GUI_PAGE_BAT_HIGH   70
#define GUI_PAGE_BAT_MEDIUM 30


t_charger_data gui_bat_prev_data = { 0 };

bool gui_bat_page_refresh(uint8_t data) {
  Adafruit_SPITFT * tft = display_get_object();
  if (tft == NULL) {
    return false;
  }

  t_charger_data charger_data = { 0 };
  if (!charger_control_get_data(charger_data)) {
    if (!data) {
      return true;
    }
  }

  tft->setTextSize(1);

  if (data) {
    tft->setTextColor(DISPLAY_WHITE);

    tft->setCursor(20, 30);
    tft->print("Vbat     = ");

    tft->setCursor(20, 30 + 8);
    tft->print("Icharger = ");

    tft->setCursor(20, 30 + 8*2);
    tft->print("Source   = ");
  }

  if (data || charger_data.bat_voltage_x100 != gui_bat_prev_data.bat_voltage_x100) {
    tft->setTextColor(DISPLAY_BLACK);
    tft->setCursor(100, 30);
    tft->print(gui_bat_prev_data.bat_voltage_x100 / 100);
    if (gui_bat_prev_data.bat_voltage_x100 % 100 < 10) {
      tft->print(".0");
    } else {
      tft->print(".");
    }
    tft->print(gui_bat_prev_data.bat_voltage_x100 % 100);
    tft->print("V");

    uint8_t pc = charger_control_get_voltage_pc();

    tft->setTextColor(pc > GUI_PAGE_BAT_HIGH ? DISPLAY_GREEN : (pc > GUI_PAGE_BAT_MEDIUM ? DISPLAY_WHITE : DISPLAY_RED));
    tft->setCursor(100, 30);
    tft->print(charger_data.bat_voltage_x100 / 100);
    if (charger_data.bat_voltage_x100 % 100 < 10) {
      tft->print(".0");
    } else {
      tft->print(".");
    }
    tft->print(charger_data.bat_voltage_x100 % 100);
    tft->print("V");

    gui_bat_prev_data.bat_voltage_x100 = charger_data.bat_voltage_x100;
  }

  if (data || charger_data.bat_current_x10 != gui_bat_prev_data.bat_current_x10) {
    tft->setTextColor(DISPLAY_BLACK);
    tft->setCursor(100, 30+8);
    tft->print(gui_bat_prev_data.bat_current_x10 / 10);
    tft->print(".");
    tft->print(gui_bat_prev_data.bat_current_x10 % 10);
    tft->print("A");

    tft->setTextColor(DISPLAY_WHITE);
    tft->setCursor(100, 30+8);
    tft->print(charger_data.bat_current_x10 / 10);
    tft->print(".");
    tft->print(charger_data.bat_current_x10 % 10);
    tft->print("A");

    gui_bat_prev_data.bat_current_x10 = charger_data.bat_current_x10;
  }

  if (data || charger_data.i2c_in_use != gui_bat_prev_data.i2c_in_use) {
    tft->setTextColor(DISPLAY_BLACK);
    tft->setCursor(100, 30+8*2);
    if (gui_bat_prev_data.i2c_in_use) {
      tft->print("Charger");
    } else {
      tft->print("ADC");
    }

    tft->setTextColor(charger_data.i2c_in_use ? DISPLAY_GREEN : DISPLAY_YELLOW);
    tft->setCursor(100, 30+8*2);
    if (charger_data.i2c_in_use) {
      tft->print("Charger");
    } else {
      tft->print("ADC");
    }

    gui_bat_prev_data.i2c_in_use = charger_data.i2c_in_use;
  }

  return true;
}
